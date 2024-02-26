/* echoclient.c - An echo client */
/* $begin echoclientmain */
#include "csapp.h"

int main(int argc, char **argv) // 프로그램 실행 시 전달된 인자의 개수 argc
{
    int clientfd; // 서버와 연결된 파일 디스크립터
    char *host, *port, buf[MAXLINE]; // 연결할 서버의 호스트 이름, 포트 번호, 서버에서 받은 문자열 buf
    rio_t rio; // 데이터 타입 변수

    if(argc != 3) // 프로그램 실행 시 전달된 인자 개수가 3개가 아니면, 즉, 제대로 전달이 안됐다면
    {
        fprintf(stderr, "usage: %s <host> <post>\n", argv[0]); // 작성 사용 방법을 출력하고 프로그램 종료
        exit(0);
    }

    // 호스트 이름과 포트 번호를 argv에서 가져온다.
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port); // 서버에 연결을 시도하고, 연결된 소켓의 파일 디스크립터를 저장
    Rio_readinitb(&rio, clientfd); // rio변수를 초기화하고 clientfd에 대한 RIO를 설정

    while(Fgets(buf, MAXLINE, stdin) != NULL) // 서버에서 받은 응답 출력을 입력이 끝날때 까지 반복
    {
        Rio_writen(clientfd, buf, strlen(buf)); // buf에 저장된 문자열을 서버에 전송
        Rio_readlineb(&rio, buf, MAXLINE); // 서버에서 받은 응답을 buf에 저장
        Fputs(buf, stdout); // buf에 저장된 응답을 출력
    }
    Close(clientfd); //line:netp:echoclient:close
    exit(0);
}
/* $end echoclientmain */