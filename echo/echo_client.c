#include "csapp.h"


int main(int argc, char **argv){
    int clientfd;
    char *host , *port , buf[MAXLINE]; 
    rio_t rio;

    if(argc !=3){
        fprintf(stderr, "usage : %s <host> <port>\n",argv[0]);
        exit(0);
    }

    host = argv[1];
    port = argv[2];

    // host와 port로 소켓을 만들고 클라이언트 소켓으로 만들기
    clientfd = Open_clientfd(host,port);
    Rio_readinitb(&rio,clientfd);


    // 표준 입력으로부터 최대 MAXLINE 까지 입력을 받아 buf에 저장
    while (Fgets(buf,MAXLINE,stdin) != NULL){
        // client 소켓을 통해 buf에 저장된 데이터를 서버로 저장
        Rio_writen(clientfd, buf, strlen(buf));
        // 서버로 부터 응답을 받아 buf에 저장
        Rio_readlineb(&rio, buf, MAXLINE);
        // buf의 내용을 표준 출력으로 출력
        Fputs(buf,stdout);
    }
    Close(clientfd);
    exit(0);
}