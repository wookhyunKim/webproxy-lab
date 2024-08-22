#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

int main(int argc, char **argv) {
    // printf("%s", user_agent_hdr);
  int listenfd_c, connfd_c, clientfd_s;  //   _c : client와 연결  ,  _s : server와 연결
  char hostname[MAXLINE], cur_port[MAXLINE], server_port[MAXLINE], buf[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  rio_t rio;


  /* 인자 2개(server입장) or 3개(client입장) 받을 것이다. 
  // 인자 3개를 받을 것이다. ex) ./proxy 8000(current num) 8001(server port) */
  if (argc != 3)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  /*
  argv[0] : hostname
  argv[1] : current_post
  argv[2] : server_post
  */


// ========================================================================================== 여기부터 
    // 클라이언트의 요청을 처리하는 것이 아닌 웹 서버로 넘겨줘야함.
    // host와 port로 소켓을 만들고 클라이언트 소켓으로 만들기
    clientfd_s = Open_clientfd(hostname,argv[2]);
    Rio_readinitb(&rio,clientfd_s);

    // 표준 입력으로부터 최대 MAXLINE 까지 입력을 받아 buf에 저장
    while (Fgets(buf,MAXLINE,stdin) != NULL){
        // client 소켓을 통해 buf에 저장된 데이터를 서버로 저장
        Rio_writen(clientfd_s, buf, strlen(buf));
        // 서버로 부터 응답을 받아 buf에 저장
        Rio_readlineb(&rio, buf, MAXLINE);
        // buf의 내용을 표준 출력으로 출력
        Fputs(buf,stdout);
    }
    Close(clientfd_s);
// ========================================================================================== 여기까지 추가함
  // 서버 소켓을 생성하고 지정된 포트(argv[1])에 바인딩하고 클라이언트의 연결 요청을 수신하는 소켓 만들기
  listenfd_c = Open_listenfd(argv[1]);
  while (1)
  {// 서버가 계속해서 클라이언트의 요청을 처리할 수 있도록 무한 루프

    // 클라이언트 주소의 길이
    clientlen = sizeof(clientaddr);
    
    // 클라이언트의 연결 요청을 수락하고 클라이언트와 통신할 수 있는 소켓 반환 
    connfd_c = Accept(listenfd_c, (SA *)&clientaddr,
                    &clientlen); 

    // 클라이언트의 주소를 통해 호스트 이름과 포트번호를 받아서 저장한다. 
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, cur_port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, cur_port);

    // 클라이언트와 연결 종료
    Close(connfd_c); 
  }
}


// ======================================================================================================================
