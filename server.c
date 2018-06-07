#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_SIZE 1024

#include "server.h"

typedef struct 
{
    char Method[BUF_SIZE];
    char Url[BUF_SIZE];
    char Version[BUF_SIZE];
} t_httpUrl;

typedef struct
{
    char *key;
    int key_len;
    char *value;
    int value_len;
} t_httpParams;

unsigned sockBufferSize(int serv_sock)
{
    unsigned optVal;
    int optLen = sizeof(int);
    getsockopt(serv_sock, SOL_SOCKET, SO_SNDBUF, (char *)&optVal, (socklen_t*)&optLen);
    return optVal;
}

void process()
{

}
int parseRequest(char *request, t_httpUrl **httpUrl, t_httpParams *(*httpParams)[BUF_SIZE])
{
    t_httpUrl httpUrl_tmp;
    printf("parseRequest start\n");
    char *delim = "\n";
    char *p;
    char key[BUF_SIZE];
    char value[BUF_SIZE];
    p = strtok(request, delim);
    printf("第一行：%s\n", p);
    sscanf(p, "%s %s %s", httpUrl_tmp.Method, httpUrl_tmp.Url, httpUrl_tmp.Version);
    printf("解析过的： %s %s %s\n", httpUrl_tmp.Method, httpUrl_tmp.Url, httpUrl_tmp.Version);
    *httpUrl = &httpUrl_tmp;
    int i = 0;
    while ((p = strtok(NULL, delim)))
    {
        printf("我是一行%s\n", p);
        sscanf(p, "%s %s", key, value);
        key[strlen(key)-1] = '\0';
        printf("我是解析过的一行 key: %s  value: %s\n strlen: %d\n", key, value, (int)strlen(value));
        char * whj = (char *)malloc(strlen(key) + 1);
        printf("开始赋值。。22\n");
        (*httpParams)[i]->key = (char *)malloc(strlen(key) + 1);
        (*httpParams)[i]->value = (char *)malloc(strlen(value) + 1);
        printf("开始赋值。。\n");
        memcpy((*(*httpParams)[i]).key, key, strlen(key));
        memcpy((*(*httpParams)[i]).value, value, strlen(value));
        i++;
    }
    printf("\n");
    printf("parseRequest end\n");
    return i;
}

char *run(char *request)
{
    char *response = "result";
    t_httpUrl *httpUrl;
    t_httpParams *httpParams[BUF_SIZE];
    int params = parseRequest(request, &httpUrl, &httpParams);

    for (int i = params; i >= 0; i--)
    {
        printf("run 里面的解析过的： %s %s\n", httpParams[i]->key, httpParams[i]->value);
    }
    printf("你好呀\n");
    printf("解析过的： %s %s %s\n", httpUrl->Method, httpUrl->Url, httpUrl->Version);
    process();
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
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //具体的IP地址
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
        char *response = run(request);
        // 1) 首先会检查缓冲区，如果缓冲区中有数据，那么就读取，否则函数会被阻塞，直到网络上有数据到来。

        // 2) 如果要读取的数据长度小于缓冲区中的数据长度，那么就不能一次性将缓冲区中的所有数据读出，剩余数据将不断积压，直到有 read()/recv() 函数再次读取。

        // 3) 直到读取到数据后 read()/recv() 函数才会返回，否则就一直被阻塞。
        printf("write start\n");
        write(clnt_sock, response, strlen(response));
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