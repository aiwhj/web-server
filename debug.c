#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "debug.h"

#if DEBUG == 0
void serverDebug(char *first, ...)
{
    
}
#elif DEBUG > 0
void serverDebug(char *first, ...)
{
    char *msg;
    msg = first;
    char buf[BUF_SIZE];
    va_list valist;
    int i = 0;
    va_start(valist, first);

    char c;
    int d;
    int len = 0;
    char str[10];
    char *s;
    while (*msg != '\0' && i <= BUF_SIZE)
    {
        if (*msg == '%')
        {
            msg++;
            switch (*msg)
            {
                case 'c':
                    c = va_arg(valist, int);
                    buf[i] = c;
                    i++;
                    break;
                case 'd':
                    d = va_arg(valist, int);
                    sprintf(str, "%d", d);
                    len = strlen(str);
                    strcat(buf, str);
                    i += len;
                    break;
                case 's':
                    s = va_arg(valist, char *);
                    if(s)
                    {
                        len = strlen(s);
                        strcat(buf, s);
                        i += len;
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            buf[i] = *msg;
            i++;
        }
        msg++;
        buf[i + 1] = '\0';
    }
    buf[i] = '\0';
#if DEBUG == 1
    printf("log：%s\n", buf);
#else
    fileLog(buf);
#endif
    va_end(valist);
}

void fileLog(char *buf)
{
    FILE *fp = NULL;
    if((fp = fopen(LOG_PATH, "a+")) == NULL)
    {
        printf("打开日志文件失败：%s\n", LOG_PATH);
        return;
    }
    fputs(buf, fp);
    fclose(fp);
}
#endif
