#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CON_NUM 30

int main(int argc, const char *argv[])
{
    int fd = open(argv[1], O_RDONLY);

    if (fd < 0)
    {
        perror("Failed to open the device.");
        return errno;
    }
    int consumer = CON_NUM;
    if (argc == 2)
    {
        consumer = atoi(argv[1]);
    }
    int i = 0;
    for (i = 0; i < consumer; i++)
    {
        char line[10];
        int ret = read(fd, line, 10);
        if (ret < 0)
        {
            fprintf(stderr, "error writing ret=%ld errno=%d perror: ", ret, errno);
            perror("");
        }
        else
        {
            printf("Bytes written: %ld\n", ret);
        }
        printf("[%d] Consumed--> %s\n", getpid(), line);
        sleep(1);
    }
    close(fd);
    return 0;
}