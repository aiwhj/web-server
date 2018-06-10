#include <sys/socket.h>
#include <netinet/in.h>

#include "sds.h"

#define BUF_SIZE 1024
#define EXT "php"
#define WWWROOT "./wwwroot"
#define INDEX "/index.html"

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

void header(int clnt_sock, int httpCode, char *type);
int fcgiProcess(int clnt_sock, t_httpParams_p *httpParams_p, char *file, char *ext, char *query, char **response);
int fileProcess(int clnt_sock, t_httpParams_p *httpParams_p, char *file, char *ext, char *query, char **response);
int parseRequest(char *request, t_httpUrl **httpUrl, t_httpParams **httpParams);
char *run(int clnt_sock, char *request);
int sockInit();
int acceptSock(int serv_sock, struct sockaddr_in clnt_addr);
int acceptData(int serv_sock, struct sockaddr_in clnt_addr, char **data);
