#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sds.h"

sds_hdr * sdsinit(char *str)
{
    int len = strlen(str);
    sds_hdr *sds;
    sds = malloc(sizeof(struct sds) + (len * 2) + 1);
    sds->len = len;
    sds->free = len;
    memcpy(sds->str, str, len);
    sds->str[len] = '\0';
    return sds;
}

void sdsfree(sds_hdr *sds)
{
    if(sds == NULL) {
        return;
    }
    free(sds);
}
