#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wiringPi.h>
#include <string.h>
#define PATH "/usr/lib/cgi-bin/tmp/fifo"

const int FndSelectPin[6] = {4, 17, 18, 27, 22, 23};
const int FndPin[8] = {6, 12, 13, 16, 19, 20, 26, 21};
const int FndFont[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
static int state;
pthread_t fndTh, fifoTh;

void FndSelect(int position) 
{
    int i;

    for (i = 0; i < 6; i++) 
    {
        if (i == position) 
            digitalWrite(FndSelectPin[i], LOW);
        else 
            digitalWrite(FndSelectPin[i], HIGH);
    }
}

void FndDisplay(int position, int num) 
{
    int i;
    int flag = 0;
    int shift = 0x01;

    for (i = 0; i < 8; i++) 
    {
        flag = (FndFont[num] & shift);
        if (i == 7 && (position == 2 || position == 4)) 
            flag |= 0x80;
        digitalWrite(FndPin[i], flag);
        shift <<= 1;
    }
}

void* fndThread(void* arg) 
{
    int pos;
    int data[6] = {0, 0, 0, 0, 0, 0};
    unsigned long time = millis();
    int i = 0;
    int stopped = 0;
    unsigned long prevStopped = 0;

    while (1) {
        if (state == -1) 
            break;
        else if (state == 0) 
        {
            if(stopped == 0)
            {
                stopped = 1;
                prevStopped = millis();
            }
        } 
        else if(state == 1) 
        {
            if(stopped == 1)
            {
                stopped = 0;
                time += millis() - prevStopped;
            }
            unsigned int diff = millis() - time;
            unsigned int min = diff / 60000; //
            unsigned int sec = (diff / 1000) % 60;
            unsigned int millisec = (diff % 1000) / 10;
            data[5] = min / 10;
            data[4] = min % 10;
            data[3] = sec / 10;
            data[2] = sec % 10;
            data[1] = millisec / 10;
            data[0] = millisec % 10;
            if(min == 10)
                break;
        }
        for (pos = 0; pos < 6; pos++) 
        {
            FndSelect(pos);
            FndDisplay(pos, data[pos]);
            delay(1);
        }
    }

    int result = remove(PATH);
    if (result != 0)
    {
        perror("Fail Delete FIFO");
        pthread_exit(NULL);
    }

    pthread_exit(NULL);
}

void* fifoThread(void* arg) 
{
    int fd;
    char message[256];

    while(1)
    {
        if ((fd = open(PATH, O_RDONLY)) < 0) 
        {
            perror("Fail Open FIFO");
            pthread_exit(NULL);
        }
        if (read(fd, message, sizeof(message)) < 0) 
        {
            perror("Fail Read FIFO");
            close(fd);
            pthread_exit(NULL);
        }
        if (strcmp(message, "stop") == 0)
            state = 0;
        else if (strcmp(message, "clear") == 0)
        {
            state = -1;
            break;
        }
        else if (strcmp(message, "start") == 0)
            state = 1;
        close(fd);
    }
    close(fd);
    pthread_exit(NULL);
}

//Do not edit
void Init() 
{
    int i;
    if (wiringPiSetupGpio() == -1) 
    {
        printf("Init Fail");
        exit(1);
    }
    for (i = 0; i < 6; i++) 
    {
        pinMode(FndSelectPin[i], OUTPUT);
        digitalWrite(FndSelectPin[i], HIGH);
    }

    for (i = 0; i < 8; i++) 
    {
        pinMode(FndPin[i], OUTPUT);
        digitalWrite(FndPin[i], LOW);
    }
}

int main() {
    pid_t getPid;
    int fd;
    umask(0);
    if (access(PATH, F_OK) == -1) 
    {
        if (mkfifo(PATH, 0666) == -1) 
        {
            perror("Fail create FIFO");
            return EXIT_FAILURE;
        }
        getPid = fork();
    } 
    else 
        getPid = -1;

    printf("Content-type: text/html\n\n");
    printf("<script>window.location.href = '/stopWatch.html';</script>\n");

    if (getPid == -1) 
    {
        fd = open(PATH, O_WRONLY | O_TRUNC);
        if (write(fd, "start", strlen("start")) < 0) 
        {
            perror("Failed to write to FIFO");
            close(fd);
            return EXIT_FAILURE;
        }
        close(fd);
        return EXIT_SUCCESS;
    } 
    else if (getPid == 0) 
    {

        Init();
        state = 1;
        pthread_create(&fndTh, NULL, fndThread, NULL);
        pthread_create(&fifoTh, NULL, fifoThread, NULL);
        pthread_join(fifoTh, NULL);

        fd = open(PATH, O_WRONLY | O_TRUNC);
        if (write(fd, "start", strlen("start")) < 0) 
        {
            perror("Failed to write to FIFO");
            close(fd);
            return EXIT_FAILURE;
        }
        close(fd);
    }
    return EXIT_SUCCESS;
}