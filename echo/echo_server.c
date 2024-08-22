#include "csapp.h"

void echo(int);

int main(int argc, char **argv){
    int listenfd, connfd;  // listen 소켓과 연결 소켓
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if(argc != 2){
        fprintf(stderr,"usage %s <port>\n",argv[0]);
        exit(0);
    }

    // 수신대기 소켓 열기
    listenfd = Open_listenfd(argv[1]);
    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        // 클라이언트의 연결 요청을 받아들이고 새로운 연결 소켓을 생성
        connfd = Accept(listenfd,(SA *)&clientaddr,&clientlen);
        // clientaddr로 부터 hostname과 port번호를 가져와서 저장해두기
        Getnameinfo((SA *)&clientaddr, clientlen,client_hostname,MAXLINE,client_port,MAXLINE,0);
        printf("Connected to (%s, %s)\n",client_hostname,client_port);
        echo(connfd);
        Close(connfd);
    }
    exit(0);
}

void echo(int connfd){
    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0 ){
        // 클라이언트로부터 한 줄의 데이터를 읽음, 데이터는 buf에 저장하고 데이터크기는 n에 저장
        printf("server received %d bytes\n", (int)n);
        // connfd를 통해 buf를 클라이언트로 돌려보냄
        Rio_writen(connfd,buf,n);
    }
}