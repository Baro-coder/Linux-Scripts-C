/*
    --------------------------------------
    | Caesar Cipher  -  multithreading C |
    --------------------------------------
*/

/*
    -------------
   -- LIBRARIES --
    -------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
    ----------------
   -- DECLARATIONS --
    ----------------
*/
#define READ 0
#define WRITE 1
#define BUFFERSIZE 8

void P1_run(char * arg);
void P2_run();
void P3_run();

int buildSyncStructures();
int closeSyncStructures();
void callTheChildProcesses(char * arg);
void sig_handler(int signo);

/*
    ---------------
   -- GLOBAL VARS --
    ---------------
*/
pid_t p1, p2, p3;   // PIDs

int pipe_fd[2]; // pipe: P1 -> P2

char * fifo_server = "FIFO_server"; // fifo: P2 -> P3

char * file_input = "./test.txt";   // file to read by P1

/*
    --------------
   -- MAIN DRIVE --
    --------------
*/
int main(int argc, char **argv)
{
    if(argc != 2){
        fprintf(stderr, "Parameters not correct!\n");
        fprintf(stderr, "Usage: %s <k>\n", argv[0]);
        fprintf(stderr, "k - integer number\n");
        exit(EXIT_FAILURE);    
    }

    if(buildSyncStructures() == -1) {
        exit(EXIT_FAILURE);
    }

    callTheChildProcesses(argv[1]);

    if(closeSyncStructures() == 0){
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }
}

/*
     -------------
    -- FUNCTIONS --
     -------------
*/
int buildSyncStructures()
{
    fprintf(stdout, "MAIN[%d]: Building the sync structures...\n", getpid());

    fprintf(stdout, "MAIN[%d]: \tCreating the fifo...\n", getpid());
    if(mkfifo(fifo_server, 0666) == -1){
        fprintf(stderr, "MAIN[%d]: \tFailed to create fifo!\n", getpid());
        return -1;
    } else {
        fprintf(stdout, "MAIN[%d]: \tFifo created.\n", getpid());
    }

    fprintf(stdout, "MAIN[%d]: \tCreating the pipes...\n", getpid());
    if((pipe(pipe_fd) == -1)){
        fprintf(stderr, "MAIN[%d]: \tFailed to create pipes!\n", getpid());
        return -1;
    } else {
        fprintf(stdout, "MAIN[%d]: \tPipes created.\n", getpid());
    }

    fprintf(stdout, "MAIN[%d]: The sync structures build.\n\n", getpid());
    return 0;
}

int closeSyncStructures()
{
    fprintf(stdout, "MAIN[%d]: Removing the sync structures...\n", getpid());

    fprintf(stdout, "MAIN[%d]: \tClosing the pipes...\n", getpid());
    if(close(pipe_fd[READ]) == 0 && close(pipe_fd[WRITE]) == 0){
        fprintf(stdout, "MAIN[%d]: \tPipes closed.\n", getpid());
    } else {
        fprintf(stderr, "MAIN[%d]: \tError closing the pipes!\n", getpid());
        return -1;
    }

    fprintf(stdout, "MAIN[%d]: \tRemoving the fifo...\n", getpid());
    if (unlink(fifo_server) == 0){
        fprintf(stdout, "MAIN[%d]: \tFifo removed.\n", getpid());
    } else {
        fprintf(stderr, "MAIN[%d]: \tError removing the fifo!\n", getpid());
        return -1;
    }

    fprintf(stdout, "MAIN[%d]: The sync structures removed\n", getpid());
    return 0;
}

void callTheChildProcesses(char * arg)
{
    fprintf(stdout, "MAIN[%d]: Calling the child processes...\n", getpid());

    ((p1 = fork()) && (p2 = fork()) && (p3 = fork()));
    
    if(p1 == -1 || p2 == -1 || p3 == -1){
        // fork() failure
        fprintf(stderr, "MAIN[%d]: Fork the processes failed!\n", getpid());
        return;
    }
    else if(p1 == 0){
        // Process 1
        P1_run(arg);
        fprintf(stdout, "P1[%d]: End of process.\n", getpid());
        exit(EXIT_SUCCESS);
    }
    else if(p2 == 0){
        // Process 2
        P2_run();
        fprintf(stdout, "P2[%d]: End of process.\n", getpid());
        exit(EXIT_SUCCESS);
    }
    else if(p3 == 0){
        // Process 3
        P3_run();
        fprintf(stdout, "P3[%d]: End of process.\n", getpid());
        exit(EXIT_SUCCESS);
    }
    else if(p1 > 0 && p2 > 0 && p3 > 0){
        // MAIN Process
        signal(SIGINT, sig_handler);

        fprintf(stdout, "MAIN[%d]: Main process is waiting...\n", getpid());
        
        wait(NULL);

        sleep(2);

        kill(p2, SIGINT);
        kill(p3, SIGINT);
        
        fprintf(stdout, "\n\nMAIN[%d]: Processes are done.\n\n", getpid());

        return;
    }
}

void sig_handler(int signo)
{
    if(signo == SIGINT)
    {
        kill(p1, SIGINT);
        kill(p2, SIGINT);
        kill(p3, SIGINT);
    }
}

/*
     -------------------
    -- PROCESSES FUNCS --
     -------------------
*/
void P1_run(char * arg)
{
    printf("P1[%d]: Ready.\n", getpid());
    
    close(pipe_fd[READ]);

    int fp = open(file_input, O_RDONLY, 0);
    if(fp == -1){
        fprintf(stderr, "P1[%d]: Cannot to read the file: %s\n", getpid(), file_input);
        kill(getppid(), SIGINT);
        while(1) { ;; }
    }

    char * buffer;
    
    if(write(pipe_fd[WRITE], arg, strlen(arg)) > 0){
        fprintf(stdout, "P1[%d]: Send: %s\n", getpid(), arg);
        usleep(400);
    } else {
        fprintf(stderr, "P1[%d]: K arg passing failure!\n", getpid());
        kill(getppid(), SIGINT);
        while(1) { ;; }
    }


    while(1)
    {
        buffer = (char *) calloc(BUFFERSIZE, sizeof(char));

        if(read(fp, buffer, BUFFERSIZE) > 0)
        {
            if(write(pipe_fd[WRITE], buffer, strlen(buffer)) > 0){
                //fprintf(stdout, "P1[%d]: Send: %s\n", getpid(), buffer);
            }
        } else {
            break;
        }
    }

    close(fp);
}

void P2_run()
{
    int fd;
    int k = 0;
    bool K_passed = false;
    char * buffer;
    char cipher[BUFFERSIZE];
    
    close(pipe_fd[WRITE]);

    printf("P2[%d]: Ready.\n", getpid());
    
    while(1)
    {
        buffer = (char *) calloc(BUFFERSIZE, sizeof(char));

        if(!K_passed){
            if(read(pipe_fd[READ], buffer, sizeof(buffer)) > 0){
                k = atoi(buffer);
                fprintf(stdout, "P2[%d]: Received K: [%d]\n", getpid(), k);
                K_passed = true;
            }
        } else {
            if(read(pipe_fd[READ], buffer, sizeof(buffer)) > 0){
                //fprintf(stdout, "P2[%d]: Received: {%s}\n", getpid(), buffer);

                int i = 0;
                char c = buffer[i];
                char e = 0;

                while(c != '\0')
                {
                    e = ((int)c + k);
                    cipher[i] = e;
                    i++;
                    c = buffer[i];
                }

                usleep(500);

                fd = open(fifo_server, O_WRONLY);
                write(fd, &cipher, sizeof(cipher));
                close(fd);
            }
        }
    }
}

void P3_run()
{
    int fd;
    char buffer[BUFFERSIZE];

    printf("P3[%d]: Ready.\n", getpid());

    while(1)
    {
        fd = open(fifo_server, O_RDONLY);

        read(fd, buffer, sizeof(buffer));

        printf("[%d]:[%d]:[%d]:<%d>:%s\n", p1, p2, getpid(), strlen(buffer), buffer);

        close(fd);

        for(int i = 0; i < BUFFERSIZE; i++){
            buffer[i] = 0;
        }
    }
}