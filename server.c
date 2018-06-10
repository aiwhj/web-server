#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUF_SIZE 1024
#define EXT "php"
#define WWWROOT "./wwwroot"
#define INDEX "/index.html"

#include "server.h"
#include "sds.h"

typedef struct 
{
    sds_hdr *method;
    sds_hdr *url;
    sds_hdr *version;
} t_httpUrl;

typedef struct
{
    sds_hdr *key;
    sds_hdr *value;
} t_httpParams;

typedef struct
{
    int len;
    t_httpParams *httpParams;
} t_httpParams_p;

unsigned sockBufferSize(int serv_sock)
{
    unsigned optVal;
    int optLen = sizeof(int);
    getsockopt(serv_sock, SOL_SOCKET, SO_SNDBUF, (char *)&optVal, (socklen_t*)&optLen);
    return optVal;
}
void header(int clnt_sock, int httpCode, char *type)
{
    char *ContentType = '\0';
    char buf[1024];
    if (strcmp(type, "json"))
    {
        ContentType = "Content-type: text/html\r\n";
    }
    else if (strcmp(type, "html"))
    {
        ContentType = "Content-type: application/json\r\n";
    }
    switch (httpCode)
    {
        case 200:
            sprintf(buf, "HTTP/1.0 200 OK\r\n");
            send(clnt_sock, buf, strlen(buf), 0);
            send(clnt_sock, ContentType, strlen(ContentType), 0);
            sprintf(buf, "\r\n");
            send(clnt_sock, buf, strlen(buf), 0);
            break;
        case 500:
            sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
            send(clnt_sock, buf, strlen(buf), 0);
            send(clnt_sock, ContentType, strlen(ContentType), 0);
            sprintf(buf, "\r\n");
            send(clnt_sock, buf, strlen(buf), 0);
            sprintf(buf, "500 Internal Server Error\r\n");
            send(clnt_sock, buf, strlen(buf), 0);
            break;
        case 404:
            sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
            send(clnt_sock, buf, strlen(buf), 0);
            send(clnt_sock, ContentType, strlen(ContentType), 0);
            sprintf(buf, "\r\n");
            send(clnt_sock, buf, strlen(buf), 0);
            sprintf(buf, "404 Not Found\r\n");
            send(clnt_sock, buf, strlen(buf), 0);
            break;
        default:
            break;
    }
}
int fcgiProcess(int clnt_sock, t_httpParams_p *httpParams_p, char *file, char *ext, char *query, char **response)
{
    printf("fcgiProcess\n");
    return 200;
}
int fileProcess(int clnt_sock, t_httpParams_p *httpParams_p, char *file, char *ext, char *query, char **response)
{
    printf("fileProcess\n");
    struct stat s_buf;
    if (file[strlen(file)] == '/')
    {
        file[strlen(file)] == '\0';
    }

    int strLen = strlen(file) + strlen(WWWROOT);

    char *path = malloc(strLen + strlen(INDEX) + 1);

    memcpy(path, WWWROOT, strlen(WWWROOT));

    strcat(path, file);

    stat(path, &s_buf);

    if (S_ISDIR(s_buf.st_mode))
    {
        printf("路径是文件夹\n");
        strLen += strlen(INDEX);
    }
    strcat(path, INDEX);
    path[strLen]='\0';
    printf("path: %s\n", path);
    char ch;
    FILE *fp;
    int i;
    char buf[1024];
    if ((fp = fopen(path, "r")) == NULL)
    {
        header(clnt_sock, 404, "html");
        return 404;
    }
    header(clnt_sock, 200, "html");
    fgets(buf, sizeof(buf), fp);
    send(clnt_sock, buf, strlen(buf), 0);
    while (!feof(fp))
    {
        send(clnt_sock, buf, strlen(buf), 0);
        fgets(buf, sizeof(buf), fp);
    }
    fclose(fp);
    return 200;
}

int parseRequest(char *request, t_httpUrl **httpUrl, t_httpParams **httpParams)
{
    t_httpUrl *httpUrl_tmp;
    t_httpParams *httpParams_tmp;
    printf("parseRequest start\n");
    char *delim = "\n";
    char *p;
    char buf1[BUF_SIZE];
    char buf2[BUF_SIZE];
    char buf3[BUF_SIZE];
    p = strtok(request, delim);
    printf("第一行：%s\n", p);
    sscanf(p, "%s %s %s", buf1, buf2, buf3);
    httpUrl_tmp = (t_httpUrl *)malloc(sizeof(t_httpUrl));
    httpUrl_tmp->method = sdsinit(buf1);
    httpUrl_tmp->url = sdsinit(buf2);
    httpUrl_tmp->version = sdsinit(buf3);
    printf("解析过的： %s %s %s\n", httpUrl_tmp->method->str, httpUrl_tmp->url->str, httpUrl_tmp->version->str);
    *httpUrl = httpUrl_tmp;
    int initLen = 10;
    httpParams_tmp = (t_httpParams *)malloc(sizeof(t_httpParams) * initLen);
    int i = 0;
    while ((p = strtok(NULL, delim)))
    {
        if(i >= initLen)
        {
            initLen *= 2;
            httpParams_tmp = realloc(httpParams_tmp, sizeof(t_httpParams) * initLen);
        }
        printf("我是一行%s %d\n", p, (int)strlen(p));
        if(strlen(p) > 1)
        {
            sscanf(p, "%s %s", buf1, buf2);
            buf1[strlen(buf1) - 1] = '\0';
            httpParams_tmp[i].key = sdsinit(buf1);
            httpParams_tmp[i].value = sdsinit(buf2);
            printf("我是解析过的一行 key: %s  value: %s\n", httpParams_tmp[i].key->str, httpParams_tmp[i].value->str);
            i++;
        }
    }
    httpParams_tmp = realloc(httpParams_tmp, sizeof(t_httpParams) * i);
    *httpParams = httpParams_tmp;
    printf("\n");
    printf("parseRequest end\n");
    return i;
}

char *run(int clnt_sock, char *request)
{
    char *response = "result";
    t_httpUrl *httpUrl;
    t_httpParams_p httpParams_p;
    httpParams_p.len = parseRequest(request, &httpUrl, &(httpParams_p.httpParams));
    printf("解析过的： %s %s %s\n", httpUrl->method->str, httpUrl->url->str, httpUrl->version->str);
    for (int i = httpParams_p.len - 1; i >= 0; i--)
    {
        printf("run 里面的解析过的： %s %s\n", httpParams_p.httpParams[i].key->str, httpParams_p.httpParams[i].value->str);
    }
    char *url = malloc(httpUrl->url->len + 1);
    memcpy(url, httpUrl->url->str, httpUrl->url->len);
    url[httpUrl->url->len] = '\0';
    char *file = '\0';
    char *ext = '\0';
    char *query = '\0';
    file = url;
    while (*url != '\0')
    {
        printf("当前字符： %c\n", *url);
        if (*url == '.')
        {
            printf("ext\n");
            ext = ++url;
        }
        else if (*url == '?')
        {
            printf("query\n");
            *url = '\0';
            query = ++url;
            break;
        }
        else
        {
            url++;
        }
    }
    printf("url: %s file: %s ext: %s query: %s\n", url, file, ext, query);
    if (strcmp(file, "/") == 0)
    {
        printf("设置默认\n");
        file = "index.html";
        ext = "html";
    }
    if (!query)
    {
        printf("query 是空的\n");
    }
    else
    {
        printf("query 不是空的\n");
    }
    
    printf("url: %s file: %s ext: %s query: %s\n", url, file, ext, query);
    printf("你好呀\n");
    int httpCode = 200;
    if (!ext || strcmp(ext, EXT) != 0)
    {
        httpCode = fileProcess(clnt_sock, &httpParams_p, file, ext, query, &response);
    }
    else
    {
        httpCode = fcgiProcess(clnt_sock, &httpParams_p, file, ext, query, &response);
    }
    printf("httpCode: %d\n", httpCode);
    return response;
}

int sockInit()
{
    //创建套接字
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // I/O缓冲区在每个TCP套接字中单独存在；
    // I/O缓冲区在创建套接字时自动生成；
    // 即使关闭套接字也会继续传送输出缓冲区中遗留的数据；
    // 关闭套接字将丢失输入缓冲区中的数据。

    //将套接字和IP、端口绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));         //每个字节都用0填充
    serv_addr.sin_family = AF_INET;                   //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr("192.168.33.10"); //具体的IP地址
    serv_addr.sin_port = htons(1234);                 //端口
    bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    //进入监听状态，等待用户发起请求
    listen(serv_sock, 20);
    //接收客户端请求
    return serv_sock;
}

int acceptSock(int serv_sock, struct sockaddr_in clnt_addr)
{
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    int client_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    return client_sock;
}

int acceptData(int serv_sock, struct sockaddr_in clnt_addr, char **data)
{
    printf("start accept\n");
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    int clnt_sock = acceptSock(serv_sock, clnt_addr);
    //向客户端发送数据
    char buffer[BUF_SIZE + 1];
    // char *tmp;
    char *src = "";
    printf("开始接受数据啦\n");
    int strLen = 0;
    int recvLen = 0;
    int isBuffer = 1;
    while (isBuffer)
    {
        printf("start recv\n");
        recvLen = recv(clnt_sock, buffer, BUF_SIZE, 0);
        strLen += recvLen;
        buffer[recvLen] = '\0';
        printf("recvLen: %d strLen: %d\n", recvLen, strLen);
        printf("-----buffer------\n %s \n-----buffer------\n", buffer);
        *data = (char *)malloc(strLen + 1);
        // memcpy(tmp, src, strLen);
        strncpy(*data, src, strLen);
        strcat(*data, buffer);
        src = *data;
        printf("接受到数据了哦\n");
        printf("------data-----\n %s \n------data-----\n", *data);
        if (recvLen < BUF_SIZE)
        {
            printf("isBuffer = 0\n");
            isBuffer = 0;
        }
    }
    printf("结束While啦\n");
    printf("clnt_sock %d\n", clnt_sock);
    printf("------data-----\n %s \n------data-----\n", *data);
    // data = data;
    return clnt_sock;
}

int main(){

    int serv_sock = sockInit();
    int *clnt_sock;
    char *request;
    struct sockaddr_in clnt_addr;
    while(1){
        int clnt_sock = acceptData(serv_sock, clnt_addr, &request);
        printf("结束acceptData啦\n");
        printf("clnt_sock %d\n", clnt_sock);
        printf("-------request------\n %s\n-------request------\n", request);
        printf("结束acceptData啦\n");
        char *response = run(clnt_sock, request);
        // 1) 首先会检查缓冲区，如果缓冲区中有数据，那么就读取，否则函数会被阻塞，直到网络上有数据到来。

        // 2) 如果要读取的数据长度小于缓冲区中的数据长度，那么就不能一次性将缓冲区中的所有数据读出，剩余数据将不断积压，直到有 read()/recv() 函数再次读取。

        // 3) 直到读取到数据后 read()/recv() 函数才会返回，否则就一直被阻塞。
        printf("write start\n");
        // write(clnt_sock, response, strlen(response));
        printf("write end\n");
        // 2) 如果TCP协议正在向网络发送数据，那么输出缓冲区会被锁定，不允许写入，write()/send() 也会被阻塞，直到数据发送完毕缓冲区解锁，write()/send() 才会被唤醒。

        // 3) 如果要写入的数据大于缓冲区的最大长度，那么将分批写入。

        // 4) 直到所有数据被写入缓冲区 write()/send() 才能返回。

        // 数据的“粘包”问题 (服务端发送的数据被缓存区分成的数据和客户端缓存区获取的到次数不同)
            //关闭套接字
        close(clnt_sock);
        /* code */
    }

    close(serv_sock);
    return 0;
}
