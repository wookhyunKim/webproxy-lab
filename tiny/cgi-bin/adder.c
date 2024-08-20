/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {

  /*
  buf : QUERY_STRING 을 가리키는 포인터 변수
  p : buf 를 탐색하는 포인터
  arg1, arg2 : QUERY_STRING 에서 추출된 두 개의 숫자를 저장
  content : response body
  n1, n2 : arg1, arg2 를 int 타입으로 변환
  */
  char *buf, *p ; 
  char arg1[MAXLINE], arg2[MAXLINE],content[MAXLINE];
  int n1 = 0, n2 = 0;

  // URL 로 부터 인자를 받아오는 작업 -> CGI QUERY_STRING 추출
  if ((buf = getenv("QUERY_STRING")) != NULL){
    p = strchr(buf, '&');
    *p = '\0';
    strcpy(arg1,buf);
    strcpy(arg2,p+1);
    n1 = atoi(arg1);
    n2 = atoi(arg2);
  }

  // response body 구성
  sprintf(content, "QUERY_STRING=%s",buf);
  sprintf(content,"Welcome to add.com : ");
  sprintf(content,"%sTHE Internet addition portal. \r\n<p>",content);
  sprintf(content,"%sThe answer is : %d + %d = %d\r\n<p>",content,n1,n2,n1+n2);
  sprintf(content,"%sThanks for visiting!\r\n",content);

  // HTTP response 생성
  printf("Connection : close \r\n");
  printf("Content-length : %d\r\n",(int)strlen(content));
  printf("Content-type : text/html\r\n\r\n");
  printf("%s",content);
  fflush(stdout);



  exit(0);
}
/* $end adder */
