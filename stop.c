#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define PATH "/usr/lib/cgi-bin/tmp/fifo"

int main() 
{
    int fd;
    char message[] = "stop";
    if ((fd = open(PATH, O_WRONLY | O_TRUNC)) < 0) 
    {
        perror("Fail Open FIFO");
        printf("Content-type: text/html\n\n");
        printf("<script>window.location.href = '/stopWatch.html';</script>\n");
        return EXIT_FAILURE;
    }
    if (write(fd, message, strlen(message)) < 0) 
    {
        perror("Fail Write FIFO");
        close(fd);
        printf("Content-type: text/html\n\n");
        printf("<script>window.location.href = '/stopWatch.html';</script>\n");
        return EXIT_FAILURE;
    }
    close(fd);
    printf("Content-type: text/html\n\n");
    printf("<script>window.location.href = '/stopWatch.html';</script>\n");
    return EXIT_SUCCESS;
}