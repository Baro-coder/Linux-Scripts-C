/*
    -------------------------------------
    | Shared memory  - multithreading C |
    -------------------------------------
*/

/*
     -------------
    -- LIBRARIES --
     -------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "string.h"
#include <signal.h>
#include <sys/wait.h>

/*
     ----------------
    -- DECLARATIONS --
     ----------------
*/
void doConsumer(int cID);

/*
     ---------------
    -- GLOBAL VARS --
     ---------------
*/
struct sem_t
{
    sem_t s_prod;
    sem_t s_cons;
    char buffer[256];
};

// -----------------------------------------------------------------------
/*
     --------------
    -- MAIN DRIVE --
     --------------
*/
int main()
{
    int serverMemory = shm_open("auction", O_CREAT | O_RDWR, 0666);
    if(serverMemory == -1)
    {
        fprintf(stderr, "Error opening shared memory!\n");
        exit(1);
    }
    ftruncate(serverMemory, sizeof(struct sem_t));

    struct sem_t *te;
    te = mmap(NULL, sizeof(struct sem_t), PROT_WRITE | PROT_READ, MAP_SHARED, serverMemory, 0);

    sem_init(&te->s_prod, 1, 0);
    sem_init(&te->s_cons, 1, 0);

    pid_t PP1, PP2, PP3;

    ((PP1=fork()) && (PP2=fork()) && (PP3=fork()));
    if(PP1 == 0)
    {
        doConsumer(1);
    }
    if(PP2 == 0)
    {
        doConsumer(2);
    }
    if(PP3 == 0)
    {
        doConsumer(3);
    }
    else if(PP1 != 0 && PP2 != 0 && PP3 != 0)
    {
        char data[256];

        while(1)
        {
            printf("PRODUCER[%d]: Type message ('exit' to quit): ", getpid());
            fgets(data, 128, stdin);

            if(strncmp(data, "exit", 4) == 0)
            {
                break;
            }

            memcpy(te->buffer, data, sizeof(data));
            memset(data, 0, 128);

            sem_post(&te->s_cons);
            sem_wait(&te->s_prod);

            memset(te->buffer,0,128);
        }

        kill(PP1, SIGINT);
        kill(PP2, SIGINT);
        kill(PP3, SIGINT);
    }

    wait(NULL);

    sem_close(&te->s_prod);
    sem_close(&te->s_cons);

    if(shm_unlink("auction") == -1){
        printf("Blad zwalniania pamieci wspoldzielonej!\n");
        exit(1);
    }

    return 0;
}

// -----------------------------------------------------------------------
/*
     -----------------
    -- PROCESS FUNCS --
     -----------------
*/
void doConsumer(int cID)
{
    int mem = shm_open("auction", O_RDWR, 0666);
    if(mem == -1)
    {
        printf("Pamiec wspoldzielona nie istnieje...\n");
        exit(1);
    }
    struct sem_t *te = mmap(NULL, sizeof(struct sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, mem, 0);

    char buff[256];
    while(1)
    {
        sem_wait(&te->s_cons);

        strcpy(buff, te->buffer);
        printf("CONSUMER[%d] [%d]: Received: %s\n", cID, getpid(), buff);

        sem_post(&te->s_prod);
    }
}
