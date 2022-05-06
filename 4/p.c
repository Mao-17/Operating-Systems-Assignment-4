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

#define PRO_NUM 30

void random_string(char *string, unsigned length)
{
    int i;
    for (i = 0; i < length; ++i)
    {
        string[i] = rand() % 10 + 65;
    }

    string[i] = '\0';
}

int main(int argc, const char *argv[])
{
    srand(time(NULL));

    int fd = open(argv[1], O_RDONLY);

    if (fd < 0)
    {
        perror("Failed to open the device.");
        return errno;
    }
    int consumer = PRO_NUM;
    if (argc == 2)
    {
        consumer = atoi(argv[1]);
    }
    int i = 0;
    for (i = 0; i < consumer; i++)
    {
        char s[31];
        random_string(s, 8);
        sleep(1);
        int ret = write(fd, s, 10);
        if (ret < 0)
        {
            fprintf(stderr, "error writing ret=%ld errno=%d perror: ", ret, errno);
            perror("");
        }
        else
        {
            printf("Bytes written: %ld\n", ret);
        }
        printf("[%d] Produced--> %s \n", getpid(), s);
    }
    close(fd);
    return 0;
}