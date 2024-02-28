// /* $begin tinymain */
// /*
//  * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
//  *     GET method to serve static and dynamic content.
//  *
//  * Updated 11/2019 droh
//  *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
//  */
#include "csapp.h"

void doit(int fd); // do_it = 한 개의 HTTP 트랜잭션 처리하는 함수
void read_requesthdrs(rio_t *rp); // request header을 읽는 함수
int parse_uri(char *uri, char *filename, char *cgiargs); // 클라이언트가 요청한 uri를 파싱하는 함수
void serve_static(int fd, char *filename, int filesize, char* method); // 정적 컨텐츠 제공하는 함수
void get_filetype(char *filename, char *filetype); // 파일 타입 명시하는 함수
void serve_dynamic(int fd, char *filename, char *cgiargs, char* method); // 동적 컨텐츠 제공하는 함수
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg); // 에러 처리 함수

// 입력 ./tiny 8000 | argc = 2, argv[0] = tiny, argv[1] = 8000
int main(int argc, char **argv) { // argc는 갯수, argv는 입력한 값의 문자들
  int listenfd, connfd; 
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr; // 클라이언트에서 연결 요청 후 클라이언트 연결 소켓 주소

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]); // 해당 포트 번호에 해당하는 듣기 소켓 식별자 열어줌. (서버쪽에서 열어줌.)
  // 무한 서버 루프
  while (1) {
    clientlen = sizeof(clientaddr); // accpet 함수 인자에 넣기 위한 주소 길이 계산
    connfd = Accept(listenfd, (SA *)&clientaddr,&clientlen);  // 반복적으로 연결 요청(Accept 함수-서버 소켓이 클라이언트의 연결 요청 수락하는 함수)
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,0); // 소켓 주소 구조체에서 스트링으로 변환 -> hostname, port 반환
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // 트랜잭션 수행
    Close(connfd);  // 트랜잭션 수행 후 연결 해제
  }
}

void doit(int fd){ // 트랜잭션 수행하는 함수
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio,fd); // &rio 주소를 가지는 읽기 버퍼와 식별자 connfd를 연결한다.
  if(!Rio_readlineb(&rio, buf, MAXLINE)){
    return;
  }
  

  printf("Request headers:\n");
  printf("%s",buf); // "GET / HTTP/1.1"
  sscanf(buf, "%s %s %s",method,uri,version); // method = GET, uri = HTTP, version = 1.1   

  if(strcasecmp(method, "GET") && strcmp(method,"HEAD")){
    clienterror(fd,method,"501","Not Implemented","Tiny does not implement this method");
    return;
  }
  read_requesthdrs(&rio); //GET 메서드면 request header 구하는 함수

  is_static = parse_uri(uri,filename,cgiargs); // 
  printf("uri : %s, filename : %s, cgiargs : %s \n", uri, filename, cgiargs);

  if (stat(filename, &sbuf) < 0) {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

  if(is_static){
    // S_ISREG : 일반 파일인지? , S_IRUSR: 읽기 권한이 있는지? S_IXUSR 실행권한이 있는지?
    // 일반 파일, 읽기 권한 판단
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
      clienterror(fd,filename,"403","Forbidden","Tiny couldn't read the file");
      return;
    }
    serve_static(fd,filename,sbuf.st_size,method); // 정적 컨텐츠를 위한 함수
  }else{
    // 일반 파일, 실행 권한 판단
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
      clienterror(fd,filename,"403","Forbidden", "Tiny couldn't run the CGI program");
      return;
    }
    serve_dynamic(fd,filename,cgiargs,method); // 동적 컨텐츠를 위한 함수
  }
}

void read_requesthdrs(rio_t *rp){ // 헤더 구하는 함수
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  /* strcmp 두 문자열을 비교하는 함수 */
  /* 헤더의 마지막 줄은 비어있기에 \r\n 만 buf에 담겨있다면 while문을 탈출한다.  */
  while (strcmp(buf, "\r\n")){
    //rio 설명에 나와있다 싶이 rio_readlineb는 \n를 만날때 멈춘다.
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

int parse_uri(char *uri, char *filename, char *cgiargs){ //클라이언트가 요청한 URI를 파싱하는 함수
  char *ptr;

  if(!strstr(uri, "cgi-bin")){ // 정적 컨텐츠일 때
    strcpy(cgiargs, ""); // CGI 인자 스트링 제거
    strcpy(filename, "."); // URI를 ./index.html같은 상대 리눅스 경로 이름으로 바꾼다.
    strcat(filename, uri); //결과 cgiargs = "" 공백 문자열, filename = "./~~ or ./home.html

    if (uri[strlen(uri) - 1] == '/') { //URI가 '/'로 끝나면
      strcat(filename, "home.html"); // 기존에 /home.html 추가
    }
    return 1;
  }else{ // 동적 컨텐츠일 때
    ptr = index(uri, '?');
    if(ptr){
      strcpy(cgiargs,ptr+1);
      *ptr = '\0';
    }else{
      strcpy(cgiargs,""); 
    }
    strcpy(filename,"."); // 나머지 부분 .으로 변환
    strcat(filename,uri); // 결과 - ./uri 
    return 0;
  }
}

void get_filetype(char *filename,char *filetype){
  if(strstr(filename,".html")){
    strcpy(filetype,"text/html");
  }else if(strstr(filename,".gif")){
    strcpy(filetype,"image/gif");
  }else if(strstr(filename,".png")){
    strcpy(filetype,"image/png");
  }else if(strstr(filename, ".jpg")){
    strcpy(filetype,"image/jpeg");
  }else if(strstr(filename,".mp4")){ // 11.7
    strcpy(filetype,"video/mp4");
  }else{
    strcpy(filetype,"text/plain");
  }
}

void serve_static(int fd, char *filename, int filesize, char *method){
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  get_filetype(filename,filetype); // 접미어를 통해 파일 타입 결정

  sprintf(buf, "HTTP/1.1 200 OK\r\n");
  sprintf(buf, "%sServer : Tiny Web Server \r\n", buf);
  sprintf(buf, "%sConnection : close \r\n", buf);
  sprintf(buf, "%sConnect-length : %d \r\n", buf, filesize);
  sprintf(buf, "%sContent-type : %s \r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers: \n");
  printf("%s", buf);

  if(!strcmp(method,"HEAD")){
    return;
  }

  srcfd = Open(filename, O_RDONLY, 0);
  // 11.9
  srcp = (char *) Malloc(filesize);
  Rio_readn(srcfd, srcp, filesize);
  Close(srcfd);
  Rio_writen(fd, srcp, filesize);
  free(srcp);
}

void serve_dynamic(int fd, char *filename, char *cgiargs, char* method){
  char buf[MAXLINE], *emptylist[] = {NULL};

  sprintf(buf,"HTTP/1.1 200 OK\r\n");
  Rio_writen(fd,buf,strlen(buf));
  sprintf(buf,"Server: Tiny Web Server\r\n");
  Rio_writen(fd,buf,strlen(buf));

  // fork 함수를 호출하는 프로세스는 부모 프로세스가 되고, 새롭게 생성되는 프로세스는 자식 프로세스가 됨
  // fork 함수에 의해 생성된 자식 프로세스는 부모 프로세스의 메모리를 그대로 복사하여 갖게 됨
  // fork 함수 호출 이후 코드부터 각자의 메모리를 사용해 실행 
  // fork()의 반환 값 = 부모는 자식프로세스의 PID(프로세스 아이디)값, 자식프로세스는 0
  // fork()의 값을 어디 변수에 저장해 놓는다면 조건문으로 부모와 자식 프로세스에서 원하는 것 따로 실행할 수 있음 
  // pid_t pid = fork() / if (pid > 0) (=부모){~~} else if (pid == 0) (=자식){~~}
  if(Fork() == 0){
    setenv("QUERY_STRING", cgiargs, 1);
    setenv("REQUEST_METHOD",method,1);
    Dup2(fd,STDOUT_FILENO);
    Execve(filename,emptylist,environ);
    // 그 후에 cgi프로그램을 로드하고 실행한다.
    // 자식은 본인 실행 파일을 호출하기 전 존재하던 파일과, 환경변수들에도 접근할 수 있다.

  }
  Wait(NULL);
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
  char buf[MAXLINE];

  sprintf(buf, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "content-type: text/html\r\n\r\n");
  Rio_writen(fd, buf, strlen(buf));
  
  sprintf(buf, "<html><title>Tiny Error</title>");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "%s: %s\r\n",errnum,shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf,"<p>%s: %s\r\n",longmsg,cause);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf,"<hr><em>The Tiny Web server</em>\r\n");
  Rio_writen(fd, buf, strlen(buf));

}
