/*
    -----------------------------------
    |       Signal handling - C       |
    -----------------------------------
*/

/*
     -------------
    -- LIBRARIES --
     -------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

/*
     ----------------
    -- DECLARATIONS --
     ----------------
*/
#define X 2
#define Y 8

void main_sig_handler(int signo);

void childProcessFunc();
int getRandom(int start, int end);

/*
     ---------------
    -- GLOBAL VARS --
     ---------------
*/
bool running = true;


// -----------------------------------------------------------------------
/*
     --------------
    -- MAIN DRIVE --
     --------------
*/
int main()
{
    printf("PID: %d\n\n", getpid());

    for(int i=1; i<31; i++)
    {
        signal(i, main_sig_handler);
    }

    int seconds = 0;

    while(true)
    {
        if(running)
        {
            if(seconds % X == 0)
            {
                fprintf(stdout, "%d + ", seconds);
            }
            seconds++;
            sleep(1);
        }
    }

    return 0;
}

// -----------------------------------------------------------------------
/*
     ----------------
    -- SIG HANDLERS --
     ----------------
*/
void main_sig_handler(int signo)
{
    if(signo == SIGCONT)
    {  
        fprintf(stderr, "\nMainProcess[%d]: Received signal: SIGCONT: %d\n", getpid(), signo);
        
        if(running)
        {
            pid_t child_pid = -1;
            child_pid = fork();

            if(child_pid == 0)
            {
                childProcessFunc();
                exit(0);
            }
            else
            {
                wait(NULL);
            }
        }
    }
    else if(signo == SIGUSR1)
    {
        running = false;
        fprintf(stderr, "\nMainProcess[%d]: Received signal: SIGUSR1: %d\n", getpid(), signo);
    }
    else if(signo == SIGUSR2)
    {
        running = true;
        fprintf(stderr, "\nMainProcess[%d]: Received signal: SIGUSR2: %d\n", getpid(), signo);
    }
    else
    {
        fprintf(stderr, "\nMainProcess[%d]: Received signal: %d\n", getpid(), signo);
    }
}

// -----------------------------------------------------------------------
/*
     ---------
    -- FUNCS --
     ---------
*/
void childProcessFunc()
{
    srand((unsigned)time(0));

    int sum = 0;
    int cmp = 0;
    for(int i=0; i<Y; i++)
    {
        cmp = getRandom(1,49);
        sum += cmp;
    }

    printf("\n\tChildProcess[%d]: Sum = %d\n", getpid(), sum);

    return;
}

int getRandom(int start, int end)
{
    int result;
    result = (rand()%(end-start)) + start;
    return result;
}