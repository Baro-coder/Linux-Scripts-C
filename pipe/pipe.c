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
int pipe1[2];   // pipe PP1  >  PM
int pipe2[2];   // pipe  PM  >  PP2
int pipe3[2];   // pipe PP2  >  PM
int pipe4[2];   // pipe  PM  >  PP1


/*
    --------------
   -- MAIN DRIVE --
    --------------
*/
int main()
{
    pipe(pipe1);
    pipe(pipe2);
    pipe(pipe3);
    pipe(pipe4);

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

/*
     -------------
    -- FUNCTIONS --
     -------------
*/
void PM_do()
{
    char buffer[BUFFERSIZE];

    close(pipe1[WRITE]);
    close(pipe2[READ]);
    close(pipe3[WRITE]);
    close(pipe4[READ]);

    while(true)
    {
        if (read(pipe1[READ], buffer, sizeof(buffer)) > 0)
        {
            printf("PM [%d, %d]:\tOdczytano od PP1:\t\t%s\n", getpid(), pipe1[READ], buffer);
        }

        if (write(pipe2[WRITE], buffer, strlen(buffer)) > 0)
        {
            printf("PM [%d, %d]:\tWysłano do PP2:\t\t\t%s\n", getpid(), pipe2[WRITE], buffer);
        }

        if (read(pipe3[READ], buffer, sizeof(buffer)) > 0)
        {
            printf("PM [%d, %d]:\tOdczytano od PP2:\t\t%s\n", getpid(), pipe3[READ], buffer);
        }
        
        if (write(pipe4[WRITE], buffer, strlen(buffer)) > 0)
        {
            printf("PM [%d, %d]:\tWysłano do PP1:\t\t\t%s\n", getpid(), pipe4[WRITE], buffer);
        }
    }    
}

void PP1_do()
{
    char buffer[BUFFERSIZE];

    close(pipe1[READ]);
    close(pipe2[READ]);
    close(pipe2[WRITE]);
    close(pipe3[READ]);
    close(pipe3[WRITE]);
    close(pipe4[WRITE]);

    
    while(true)
    {
        printf("> PP1[%d]: Podaj tekst:\t\t\t", getpid());
        scanf("%s", &buffer);

        if (write(pipe1[WRITE], buffer, strlen(buffer)) > 0)
        {
            printf("PP1[%d, %d]:\tWysłano do PM:\t\t\t%s\n", getpid(), pipe1[WRITE], buffer);
        }

        if((read(pipe4[READ], buffer, sizeof(buffer))) > 0)
        {
            printf("PP1[%d, %d]:\tOdczytano od PM:\t\t%s\n\n", getpid(), pipe4[READ], buffer);
        }

        
    }    
}

void PP2_do()
{
    char buffer[BUFFERSIZE];

    close(pipe1[READ]);
    close(pipe1[WRITE]);
    close(pipe2[WRITE]);
    close(pipe3[READ]);
    close(pipe4[READ]);
    close(pipe4[WRITE]);

    while(true)
    {
        if (read(pipe2[READ], buffer, sizeof(buffer)) > 0)
        {
            printf("PP2[%d, %d]:\tOdczytano od PM:\t\t%s\n", getpid(), pipe2[READ], buffer);
        }

        // ------------------------------------------------
        // Convert the message
        // -
        char c = 0;
        int i = 0;
        c = buffer[i];
        while(c != 0)
        {
            if(c >= 65 && c <= 90)
            {
                // letters big to small
                c += 32;
                buffer[i] = c;
            }
            else if(c >= 97 && c <= 122)
            {
                // letters small to big
                c -= 32;
                buffer[i] = c;
            }
            i++;
            c = buffer[i];
        }
        // -
        // ------------------------------------------------

        if (write(pipe3[WRITE], buffer, strlen(buffer)) > 0)
        {
            printf("PP2[%d, %d]:\tWysłano do PM:\t\t\t%s\n", getpid(), pipe3[WRITE], buffer);
        }
    }    
}

