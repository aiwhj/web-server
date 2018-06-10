typedef struct sds
{
    int len;
    int free;
    char str[];
} sds_hdr;

typedef char * string;

sds_hdr *sdsinit(char *str);
void sdsfree(sds_hdr *sds);
