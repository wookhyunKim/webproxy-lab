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
  char *buf, *p ,*a,*b;  // a,b는 search parameter  받아서 쓰는 값
  char arg1[MAXLINE], arg2[MAXLINE],content[MAXLINE];
  int n1 = 0, n2 = 0;

  // URL 로 부터 인자를 받아오는 작업 -> CGI QUERY_STRING 추출
  if ((buf = getenv("QUERY_STRING")) != NULL){
    p = strchr(buf, '&');
    *p = '\0';

    char *num1,*num2;
    num1 = buf;
    num2 = p+1;
    // url 형식이 num1=1&num2=3 이런형식으로 오기 때문에 형식을 맞춰줘야함.
    a = strchr(num1, '=');
    *a = '\0';
    strcpy(arg1,a+1);
    b = strchr(num2, '=');
    *b = '\0';
    strcpy(arg2,b+1);

    n1 = atoi(arg1);
    n2 = atoi(arg2);
    // printf("서버로 부터 받은 인자 값  : n1 : %d   n2 : %d   \n",n1,n2);   확인할 수 없다 왜냐면 dup함수로 서버에서 클라이언트로 직접 전달하기 때문
   }
  // 기본 set
  // if ((buf = getenv("QUERY_STRING")) != NULL){
  //   p = strchr(buf, '&');
  //   *p = '\0';
  //   strcpy(arg1,buf);
  //   strcpy(arg2,p+1);
  //   n1 = atoi(arg1);
  //   n2 = atoi(arg2);
  //   printf("서버로 부터 받은 인자 값  : n1 : %d   n2 : %d   \n",n1,n2);
  // }

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
