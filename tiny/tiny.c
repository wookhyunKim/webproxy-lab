/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv)
{
  /*
  listenfd : 서버가 클라이언트의 연결을 수신 대기 하는 소켓 (파일 디스크립터)
  connfd : 클라이언트와 서버가 연결된 서버쪽 소켓
  clientlen : 클라이언트 주소의 길이
  clientaddr : 클라이언트 주소를 저장
  */


  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* 인자 2개만 받을 것이다. ex) ./tiny 8000(port num) */
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }


  // 서버 소켓을 생성하고 지정된 포트(argv[1])에 바인딩하고 클라이언트의 연결 요청을 수신하는 소켓 만들기
  listenfd = Open_listenfd(argv[1]);
  while (1)
  {// 서버가 계속해서 클라이언트의 요청을 처리할 수 있도록 무한 루프

    // 클라이언트 주소의 길이
    clientlen = sizeof(clientaddr);
    
    // 클라이언트의 연결 요청을 수락하고 클라이언트와 통신할 수 있는 소켓 반환 
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen); 

    // 클라이언트의 주소를 통해 호스트 이름과 포트번호를 받아서 저장한다. 
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);

    // 클라이언트의 요청을 처리하는 함수
    doit(connfd);  

    // 클라이언트와 연결 종료
    Close(connfd); 
  }
}



// ====================================================== functions =================================================================
void doit(int fd)
{
  /*
  is_static : 정적 컨텐츠인지 동적 컨텐츠인지 확인하는 flag
  sbuf : 파일의 메타데이터(크기 , 수정시간 등)를 저장하는 구조체
  buf, method, uri, version, filename, cgiargs: 요청 라인과 URI, 파일 이름, CGI 인자를 저장하기 위한 문자열 버퍼들
  rio : robust I/O 패키지로, 안전하고 효율적인 입력/출력을 돕는 구조체
  */
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  // rio 구조체 초기화
  Rio_readinitb(&rio, fd);

  // 클라이언트의 요청 라인을 읽어 buf에 저장 (print하려면 buffer 에 저장해야 한다.)
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);

  // buf 로부터 HTTP 메서드 URI , version을 추출한다.
  sscanf(buf, "%s %s %s", method, uri, version);

  // 대소문자 구분없이 method 가 GET인지 비교
  if (strcasecmp(method, "GET"))
  {
    clienterror(fd, method, "501", "Not implemented",
                "Tiny does not implement this method");
    return;
  }
  // request 헤더를 읽어들임. 클라이언트가 보낸 추가적인 헤더들을 처리하거나 무시시
  read_requesthdrs(&rio);

  // URI를 분석하여 요청이 정적 컨텐츠인지 동적 컨텐츠인지 판단.
  is_static = parse_uri(uri, filename, cgiargs);
  if (stat(filename, &sbuf) < 0)
  {
    clienterror(fd, filename, "404", "Not found",
                "Tiny couldn’t find this file");
    return;
  }
  

  /*
  S_ISREG(sbuf.st_mode): 요청된 파일이 일반 파일(정규 파일)인지 확인합니다.
  S_IRUSR & sbuf.st_mode: 파일이 읽기 권한을 가지고 있는지 확인합니다.

  정적 콘텐츠일 경우, 파일을 클라이언트에게 전송합니다.
  동적 콘텐츠일 경우, CGI 프로그램을 실행하고 그 결과를 클라이언트에게 전송합니다.  
  */
  if (is_static)
  { /* Serve static content */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn’t read the file");
      return;
    }
    // 파일을 클라이언트에게 전송하여 정적 컨텐츠를 제공. 파일의 내용을 읽어서 클라이언트로 전송하는 함수
    serve_static(fd, filename, sbuf.st_size);
  }
  else
  { /* Serve dynamic content */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn’t run the CGI program");
      return;
    }
    // 동적 컨텐츠를 제공하기 위해 CGI 프로그램을 실행한다. CGI 스크립트를 실행하고 결과를 클라이언트에게 전송
    serve_dynamic(fd, filename, cgiargs);
  }
}


// ====================================================== ERROR 함수 =================================================================
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor="
                "ffffff"
                ">\r\n",
          body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);


  // 클라이언트에 보낼 데이터를 버퍼에 저장한 후에 전송을 한다. 
  // 저장하는 함수는 sprintf , 전송하는 함수는 Rio_writen

  /* Print the HTTP response */
  // 버퍼에 response line 저장
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  // 클라이언트에게 response line 을 전송
  Rio_writen(fd, buf, strlen(buf));
  // 버퍼에 content type 저장
  sprintf(buf, "Content-type: text/html\r\n");
  // 클라이언트에게 content type 전송
  Rio_writen(fd, buf, strlen(buf));
  // 버퍼에 content length 저장
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  // 클라이언트에게 content length 전송
  Rio_writen(fd, buf, strlen(buf));
  // 클라이언트에게 body 전송
  Rio_writen(fd, body, strlen(body));
}


// ====================================================== request 헤더 함수 =================================================================
void read_requesthdrs(rio_t *rp)
{

  char buf[MAXLINE];


  // rp로부터 한 줄을 읽어 buf에 저장
  Rio_readlineb(rp, buf, MAXLINE);
  while (strcmp(buf, "\r\n")) // buf가 빈 줄이 아닐 때 반복
  {
    // 한 줄 읽어서 버퍼에 저장하고 출력
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}



// ====================================================== URI 분석 함수 =================================================================
// uri 로 부터 정적 컨텐츠인지 동적 컨텐츠인지 파악하고 , filename 과 cgiargs를 알아내는 함수
int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr;


  if (!strstr(uri, "cgi-bin"))
  {
    strcpy(cgiargs, "");              // CGI 인자들을 비워둠
    strcpy(filename, ".");            // 파일 경로의 시작을 현재 디렉토리로 설정
    strcat(filename, uri);            // URI를 파일 경로에 추가
    if (uri[strlen(uri) - 1] == '/')  // URI가 '/'로 끝나면
    {
      strcat(filename, "home.html");  // 기본 파일명 'home.html' 추가
    }
    return 1;  // 정적 콘텐츠임을 표시
  }
  else
  {
    ptr = index(uri, '?');   // '?' 문자를 찾아 CGI 인자들을 추출
    if (ptr)
    {
      strcpy(cgiargs, ptr + 1);  // '?' 이후의 문자열을 cgiargs에 저장
      *ptr = '\0';               // '?'을 '\0'으로 대체하여 URI에서 CGI 인자 제거
    }
    else
    {
      strcpy(cgiargs, "");  // '?'가 없으면 CGI 인자를 빈 문자열로 설정
    }
    strcpy(filename, ".");  // 파일 경로의 시작을 현재 디렉토리로 설정
    strcat(filename, uri);  // URI를 파일 경로에 추가
    return 0;               // 동적 콘텐츠임을 표시
  }
}



// ====================================================== 정적 컨텐츠 =================================================================
void serve_static(int fd, char *filename, int filesize)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  // buf에 헤더를 모두 저장하고 
  // client에 buf를 보낸다. 
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  // 파일을 읽기 전용으로 열고 파일 디스크립터로 저장
  srcfd = Open(filename, O_RDONLY, 0);
  // 파일을 메모리에 매핑 파일의 내용을 가상 메모리 주소공간에 매핑하고 시작주소를 srcp로 반환 파일내용을 메모리처럼 읽을 수 있다.
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  Close(srcfd);
  // 매핑된 파일 내용을 클라이언트에 전송
  Rio_writen(fd, srcp, filesize);
  // 파일의 매핑을 해제하여 메모리를 반환
  Munmap(srcp, filesize);
}

// ====================================================== MIME type 알아내는 함수 =================================================================
void get_filetype(char *filename, char *filetype)
{
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else
    strcpy(filetype, "text/plain");
}


// ====================================================== 동적 컨텐츠 함수 =================================================================
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
  /*
  buf : HTTP 응답 메시지를 저장할 버퍼
  emptylist : execve 함수에서 사용할 인자 목록을 저장하는 배열 -> execve함수는 fork한 자식 프로세스를 실행하는 함수
  */
  char buf[MAXLINE], *emptylist[] = {NULL};

  // 버퍼에 상태 줄 저장
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  // 클라이언트에 버퍼 전송
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));


  // fork로 새로운 프로세스 생성(자식 프로세스) 자식 프로세스는 return 값이 0이다.
  if (Fork() == 0)
  { /* Child */
    // 환경 변수 설정 : CGI 프로그램이 쿼리 문자열을 통해 전달된 인자들을 인식하도록 함
    setenv("QUERY_STRING", cgiargs, 1);
    // 표준출력을 CGI 에서 클라이언트로 직접 전달 ( 리다이렉트 )
    Dup2(fd, STDOUT_FILENO);              /* Redirect stdout to client */
    // filename 으로 지정된 CGI 프로그램을 실행
    Execve(filename, emptylist, environ); /* Run CGI program */
  }
  Wait(NULL); /*부모 프로세스는 자식이 종료되어 정리되는 것을 기다린다 wait 함수 */
}