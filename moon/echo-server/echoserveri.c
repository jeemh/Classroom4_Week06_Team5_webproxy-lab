/* echoserveri.c - An iterative echo server */
/* $begin.echoserverimain */
#include "csapp.h"

void echo(int connfd); // echo 함수를 선언, 클라이언트와 연결된 후 데이터를 수신하고 다시 돌려준다.

int main(int argc, char **argv) // 전달된 인자 개수 argv
{
    int listenfd, connfd; // 서버의 리스닝 소켓, 클라이언트의 커넥트 소켓 디스크립터
    socklen_t clientlen; // 클라이언트의 주소 길이
    /* Enough space for any address */
    // line:netp:echoserveri:sockaddrstorage
    struct sockaddr_storage clientaddr; // 클라이언트의 주소 정보저장 구조체
    char client_hostname[MAXLINE], client_port[MAXLINE]; // 클라이언트의 호스트 이름, 포트 번호

    if(argc != 2) // 전달된 인자가 2개가 아니라면
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]); // 사용 방법 출력 후 종료
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]); // 클라이언트의 요청을 수신하기 위해 리스닝 소켓을 열기, 포트번호를 받는다.
    while(1) // 클라이언트의 연결 요청을 계속 수락하고 처리하는 무한루프
    {
        clientlen = sizeof(struct sockaddr_storage); // accept함수를 호출할 때 인자로 보낼 클라이언트 주소 길이 저장
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // 클라이언트의 연결 요청 수락 후 파일 디스크립터를 connfd에 저장
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, // 클라이언트 주소 정보를 이용해 호스트 이름과 포트번호를 추출
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port); // 클라이언트의 호스트 이름과 포트 번호 출력
        echo(connfd); // 클라이언트에서 받은 데이터를 그대로 돌려준다.
        Close(connfd); // 클라이언트와의 연결 종료
    }
    exit(0);
}
/* $end echoserverimain */