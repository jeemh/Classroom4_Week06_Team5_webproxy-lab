/* echo - read and echo text lines until client closed connection */
/* $begin echo */
#include "csapp.h"

void echo(int connfd)
{
    size_t n; // 전송받은 문자열의 크기
    char buf[MAXLINE]; // 클라이언트로부터 전송받은 문자열을 저장할 버퍼
    rio_t rio; // Robust I/O 패키지에서 정의한 데이터 타입

    Rio_readinitb(&rio, connfd); // 초기화하고 클라이언트와 연결된 소켓에 대한 RIO 설정
    
    // 클라이언트로부터 문자열을 읽어 buf에 저장하는 동작을 클라이언트가 연결을 종료할 때까지 반복
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {
        //line:netp:echo:eof
        printf("server received %d bytes\n", (int)n); // 서버가 받은 데이터의 길이를 출력
        Rio_writen(connfd, buf, n); // buf에 저장된 문자열을 다시 클라이언트에 전송한다.
    }
}
/* $end echo */