/*
    -------------------------------------
    | Inter-process comunication - pipe |
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
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
    ----------------
   -- DECLARATIONS --
    ----------------
*/
// Definitions
#define READ 0
#define WRITE 1
#define BUFFERSIZE 80

// Funcs
void PP1_do();
void PP2_do();
void PM_do();


// PIDs
pid_t pp1, pp2;

// PIPEs
int pipe1_read[2];   // pipe PP1  >  PM
int pipe2_read[2];   // pipe  PM  >  PP2
int pipe3_read[2];   // pipe PP2  >  PM
int pipe4_read[2];   // pipe  PM  >  PP1


/*
    --------------
   -- MAIN DRIVE --
    --------------
*/
int main()
{
    if((pp1 = fork()) == 0)
    {
        // PP1 func
        PP1_do(); 
    }
    else
    {
        if((pp2 = fork()) == 0)
        {
            // PP2 func
            PP2_do();
        }
        else
        {
            // PM func
            PM_do();
        }
    }

    exit(0);
}

void PM_do()
{
    char buffer[BUFFERSIZE];

    close(pipe1_read[WRITE]);
    close(pipe2_read[READ]);
    close(pipe3_read[WRITE]);
    close(pipe4_read[READ]);

    while(true)
    {
        ;;
    }    
}

void PP1_do()
{
    char buffer[BUFFERSIZE];

    close(pipe1_read[READ]);
    close(pipe2_read[READ]);
    close(pipe2_read[WRITE]);
    close(pipe3_read[READ]);
    close(pipe3_read[WRITE]);
    close(pipe4_read[WRITE]);

    
    while(true)
    {
        printf("> ");
        fgets(buffer, BUFFERSIZE, stdin);

        printf("\n\nbuffer:\t%s\n----------\n\n",buffer);
    }    
}

void PP2_do()
{
    char buffer[BUFFERSIZE];

    close(pipe1_read[READ]);
    close(pipe1_read[WRITE]);
    close(pipe2_read[WRITE]);
    close(pipe3_read[READ]);
    close(pipe4_read[READ]);
    close(pipe4_read[WRITE]);

    while(true)
    {
        
    }    
}

