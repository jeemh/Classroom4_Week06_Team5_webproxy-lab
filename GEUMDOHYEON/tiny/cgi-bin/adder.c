/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
  char *buf, *A, *B, *method;
  char arg1[MAXLINE],arg2[MAXLINE],content[MAXLINE];
  int n1 = 0, n2 = 0;

  if((buf = getenv("QUERY_STRING")) != NULL){
    A = strchr(buf, 'A');
    B = strchr(buf, 'B');
    *A = '\0';
    *B = '\0';
    strcpy(arg1, A+2);
    strcpy(arg2, B+2);
    n1 = atoi(arg1);
    n2 = atoi(arg2);
  }

  method = getenv("REQUEST_METHOD");

  sprintf(content,"Wecolme to add.com: ");
  sprintf(content, "%sTHE Internet addittion portal.\r\n<p>",content);
  sprintf(content,"%sThe answer is: %d + %d = %d\r\n<p>", content, n1 ,n2 ,n1+n2);
  sprintf(content,"%sThanks for visiting!\r\n",content);

  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n",(int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  if (strcasecmp(method, "GET") == 0) {
    printf("%s", content);
    printf("n1: %d, n2: %d",n1,n2);
  }

  // printf("%s",content);
  // printf("n1: %d, n2: %d",n1,n2);
  fflush(stdout);

  exit(0);
}
/* $end adder */
