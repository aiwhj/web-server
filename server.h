int sockInit();
char *run(int serv_sock, char *data);
unsigned sockBufferSize();
int acceptData(int serv_sock, struct sockaddr_in clnt_addr, char **data);
int acceptSock(int serv_sock, struct sockaddr_in clnt_addr);
// void parseRequest(char *request, t_httpUrl **httpUrl, t_httpParams **httpParams);
