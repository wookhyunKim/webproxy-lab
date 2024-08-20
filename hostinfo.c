#include "csapp.h"
// #include <csapp.c>

int main(int argc,char **argv){
    /*
    addrinfo 구조체 : 주소 정보를 저장하는데 사용
    p는 리스트를 순회하는데 사용하는 임시 포인터
    listp는 getaddrinfo 가 반환한 주소 정보의 연결리스트의 시작점
    hints는 getaddrinfo에 넣어줄 인자값을 저장하는 구조체
    */
    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc, flags;

    if(argc != 2){
        fprintf(stderr,"usage : %s <domain name>\n",argv[0]);
        exit(0);
    }

    // getaddrinfo 함수의 파라미터로 넣어줄 hints 구조체 초기화
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // getaddrinfo함수로 listp 도메인 이름 argv[1] 주소 정보를 가져와서 listp에 연결리스트형태로 반환
    if ((rc = getaddrinfo(argv[1],NULL,&hints,&listp))!= 0){
        fprintf(stderr,"getaddrinfo error : %s\n",gai_strerror(rc));
        exit(1);
    }

    flags = NI_NUMERICHOST; // 숫자 형식의 IP 주소로 반환하는 flag
    for (p = listp ; p ; p = p->ai_next){
        Getnameinfo(p->ai_addr,p->ai_addrlen,buf,MAXLINE,NULL,0,flags);
        printf("%s\n",buf); // 버퍼에 ip주소 저장하여 출력
    }


    Freeaddrinfo(listp);
    exit(0);
}