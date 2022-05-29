/*
    -------------------------------------
    | Message Queue  - multithreading C |
    -------------------------------------
*/

/*
     -------------
    -- LIBRARIES --
     -------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

/*
     ----------------
    -- DECLARATIONS --
     ----------------
*/
int queueOpen();
int queueRemove(int qID);
void genericProcessDo(int mesAmount);
void readingProcessDo(int permType, int mesAmount);

/*
     ---------------
    -- GLOBAL VARS --
     ---------------
*/
int qID;    // queue ID

// Message structure
struct msgbuff
{
    int type;
    int data;
};



// -----------------------------------------------------------------------
/*
     --------------
    -- MAIN DRIVE --
     --------------
*/
int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        printf("Amount of params not correct!\n");
        printf("Usage: %s <permitted type...> <messages count...>\n", argv[0]);
        exit(1);
    }

    int permType = atoi(argv[1]);
    int mesAmount = atoi(argv[2]);

    if(permType < 1 || permType > 10)
    {
        printf("Parameters not correct!\n");
        printf("Usage: %s <permitted type...> <messages count...>\n", argv[0]);
        printf("Permitted type - number from range <1,10>\n");
        exit(1);
    }
    if(mesAmount < 1 || mesAmount > 1000)
    {
        printf("Parameters not correct!\n");
        printf("Usage: %s <permitted type...> <messages count...>\n", argv[0]);
        printf("Messages count - number from range <1,1000>\n");
        exit(1);
    }

    if ((qID = queueOpen()) == -1)
    {
        perror("Error opening the queue\n");
        exit(1);
    }

    pid_t PG, PR;

    PG = fork(); 

    if(PG == 0)
    {
        genericProcessDo(mesAmount);
    }

    PR = fork();
    wait(NULL);
    if(PR == 0)
    {
        readingProcessDo(permType, mesAmount);
    }
    
    wait(NULL);
    queueRemove(qID);
    exit(0);
}

// -----------------------------------------------------------------------
/*
     -----------------
    -- PROCESS FUNCS --
     -----------------
*/
int queueOpen()
{
    int qID;

    if((qID = msgget((key_t)1234, IPC_CREAT | 0660)) == -1) return -1;
    else return qID;
}
int queueRemove(int qID)
{
    if(msgctl(qID, IPC_RMID, 0) == -1) return -1;
    else return qID;
}

void genericProcessDo(int mesAmount)
{
    struct msgbuff msg;

    srand(time(NULL));
    int tp;
    int dt;

    for(int i=1; i<=mesAmount; i++)
    {
        tp = (rand() % 10) + 1;
        dt = (rand() % 1000) + 1;

        msg.type = tp;
        msg.data = dt;

        printf("[%d] GENERATOR:\tmessage %d send: \t{type: %d , data: %d}\n", getpid(), i, msg.type, msg.data);

        msgsnd(qID, &msg, sizeof(int), 0);
    }

    exit(0);
}

void readingProcessDo(int permType, int mesAmount)
{
    struct msgbuff rcv;

    int rcv_mes_count = 0;

    for(int i=1; i<=mesAmount; i++)
    {
        msgrcv(qID, &rcv, sizeof(int), 0, 0);

        if(rcv.type != permType)
        {
            rcv_mes_count++;
            printf("[%d] RECEIVER:\tmessage %d received:\t{type: %d , data: %d}\n", getpid(), rcv_mes_count, rcv.type, rcv.data);
        }
    }

    exit(0);
}