#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>  // definitions of a number of data types used in socket.h and netinet/in.h
#include <netinet/in.h> // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <sys/socket.h> // definitions of structures needed for sockets, e.g. sockaddr
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <string.h>

#define BUFF_SIZE 25000

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    int portno;
    FILE *hfile;
    struct sockaddr_in serv_addr, cli_addr; //소켓통신을 위한 구조체를 선언
    socklen_t clien;                        //socklen_t는 소켓 관련 매개 변수에 사용되는 것으로 길이 및 크기 값에 대한 정의

    char buff_rcv[BUFF_SIZE];
    char buff_snd[BUFF_SIZE];
    char header[2000];
    char data[500000];

    int n = 0;
    long len;
    int fd;
    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //IPv4프로토콜에 연결지향형 TCP연결 소켓 생성
    if (sockfd == -1)
        perror("ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr)); //서버의 주소를 0으로 초기화
    portno = atoi(argv[1]);                       //포트번호를 변환해줌 ( aragv[1]을 정수형으로 형변환 )

    serv_addr.sin_family = AF_INET;         //IPv4프로토콜
    serv_addr.sin_port = htons(portno);     //convert from host to network byte order (s = short:16bit)
    serv_addr.sin_addr.s_addr = INADDR_ANY; //서버의 ip주소는 항상 서버가 실행중인 주소임 (INACCR_ANY는 내 현재 pc의 ip주소임)

    bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)); //socket에 ip와 port번호 할당
    listen(sockfd, 5);                                              //최대 5명의 client의 요청을 받게끔

    while (1)
    {
        clien = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clien); //client로 오는 request를 받기 위해 accept
        if (newsockfd < 0)
            perror("ERROR on accept");

        bzero(buff_rcv, BUFF_SIZE);               //client로부터 올 메세지를 버퍼에 담기 위하여 초기화
        n = read(newsockfd, buff_rcv, BUFF_SIZE); //socket으로부터 읽어 버퍼에 채워줌
        printf("receive: %s\n", buff_rcv);        //확인print

        strcpy(buff_snd, buff_rcv);
        char *tok = strtok(buff_rcv, " ");
        tok = strtok(NULL, " ");

        memset(data, 0x00, sizeof(data));
        memset(header, 0x00, sizeof(header)); //while문 돌때마다 data, header 새로 초기화

        if (strcmp(tok, "/hazzun.html") == 0)
        {
            snprintf(header, sizeof(header),
                     "HTTP/1.0 200 OK\r\n"
                     "Content-Length: 2555\r\n"
                     "Content-Type: text/html\r\n"
                     "\r\n");
            //헤더 작성

            snprintf(data, sizeof(data),
                     "<!DOCTYPE html>\n"
                     "<html>\n"
                     "<head>\n"
                     "</head>\n"
                     "<body>\n"
                     "<h1>amazing http socket programming</h1> \n"
                     "<a href=\"./img\"><p>Image</p></a>\n"
                     "<a href=\"./mp3\"><p>Music</p></a>\n"
                     "<a href=\"./gif\"><p>gif</p></a>\n"
                     "<a href=\"./pdf\"><p>pdf</p></a>\n"
                     "</body>\n"
                     "</html>\n"
                     "\r\n");
            //index일때의 html data
        }
        else if (strcmp(tok, "/img") == 0)
        {
            int fd = open("./sexy.png", O_RDWR);
            len = lseek(fd, 0, SEEK_END); //파일 크기를 len에 넣어준다.
            printf("len = %ld\n", len);
            lseek(fd, 0, SEEK_SET);           //파일 디스크립터 포인터를 다시 원래자리로 돌려놓는다.
            read(fd, data, sizeof(data) - 1); //파일로부터 읽은 데이터를 data 버퍼에 채워준다.

            snprintf(header, sizeof(header),
                     "HTTP/1.0 200 OK\r\n"
                     "Accept-Ranges: bytes\r\n"
                     "Content-Length: %ld\r\n"
                     "Connection: Keep-Alive\r\n"
                     "Content-Type: image/png\r\n"
                     "\r\n",
                     len); //헤더작성 : Content-Length는 보내줄 파일의 크기만큼으로 설정하였다.
        }
        else if (strcmp(tok, "/mp3") == 0)
        {
            int fd = open("./test.mp3", O_RDWR);
            len = lseek(fd, 0, SEEK_END);
            printf("len = %ld\n", len);
            lseek(fd, 0, SEEK_SET);
            read(fd, data, sizeof(data) - 1);

            snprintf(header, sizeof(header),
                     "HTTP/1.0 200 OK\r\n"
                     // "HTTP/1.0 206 Partial Content\r\n"
                     "Content-Length: %ld\r\n"
                     "Content-Type: audio/mpeg\r\n"
                     "Connection: keep-alive\r\n"
                     "\r\n",
                     len);
        }

        else if (strcmp(tok, "/gif") == 0)
        {
            int fd = open("./k_league.gif", O_RDWR);
            len = lseek(fd, 0, SEEK_END);
            printf("len = %ld\n", len);
            lseek(fd, 0, SEEK_SET);
            read(fd, data, sizeof(data) - 1);

            snprintf(header, sizeof(header),
                     "HTTP/1.0 200 OK\r\n"
                     "Content-Length: %ld\r\n"
                     "Content-Type: image/gif\r\n"
                     "\r\n",
                     len);
        }
        else if (strcmp(tok, "/pdf") == 0)
        {
            int fd = open("./study.pdf", O_RDWR);
            len = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);
            read(fd, data, sizeof(data) - 1);
            printf("len = %ld\n", len);

            snprintf(header, sizeof(header),
                     "HTTP/1.0 200 OK\r\n"
                     "Content-Length: %ld\r\n"
                     "Content-Type: application/pdf\r\n"
                     "\r\n",
                     len);
        }

        else if (strcmp(tok, "/favicon.ico") == 0)
        {
            int fd = open("./ico.ico", O_RDWR);
            len = lseek(fd, 0, SEEK_END);
            printf("len = %ld\n", len);
            lseek(fd, 0, SEEK_SET);
            read(fd, data, sizeof(data) - 1);

            snprintf(header, sizeof(header),
                     "HTTP/1.0 200 OK\r\n"
                     "Content-Length: %ld\r\n"
                     "Content-Type: image/x-icon\r\n" //favicon
                     "\r\n",
                     len);
        }

        if (write(newsockfd, header, strlen(header)) > 0)
        {
            printf("header send end\n");
        }
        if (write(newsockfd, data, sizeof(data)) > 0)
        {
            printf("data send end\n\n");
        }
        //헤더를 보내고 바로 데이터를 보내주었다. 확인하기 위해 출력문을 썼다.
    }
    close(newsockfd);
    close(sockfd);
    //통신 종료시 소켓들을 닫는다.
    return 0;
}