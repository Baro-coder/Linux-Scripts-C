/*
    -------------------------------------
    | Inter-process comunication - FIFO |
    |          -- CLIENT --             |
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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
    ----------------
   -- DECLARATIONS --
    ----------------
*/
// Funcs
void client_do(int index);

// client data
struct client_data
{
    int index;
    int pid;
    char * fifo;
};

/*
    ---------------
   -- GLOBAL VARS --
    ---------------
*/
char * fifo_server = "fifo_server";     // FIFO Server Name


/*
    --------------
   -- MAIN DRIVE --
    --------------
*/
int main(int argc, char * argv[])
{
    // Parameter validation
    if(argc != 2){
        fprintf(stderr, "Not correct runat parameter!\n");
        fprintf(stdout, "Usage: %s <client amount...>\n", argv[0]);
        exit(-1);
    }

    int clients_amount = atoi(argv[1]);
    
    for(int i = 0; i < clients_amount; i++)
    {
        if(fork() == 0)
        {
            client_do(i+1);
        }
        else
        {
            wait(NULL);
        }
    }
    
    printf("\n");

    exit(0);
}

/*
     -------------
    -- FUNCTIONS --
     -------------
*/
void client_do(int index)
{
    struct client_data *cl_dat = (struct client_data *) malloc(sizeof(cl_dat));

    cl_dat->index = index;
    cl_dat->pid = getpid();
    cl_dat->fifo = (char *) malloc(80 * sizeof(char));
    sprintf(cl_dat->fifo, "fifo_%d", getpid());

    mkfifo(cl_dat->fifo, 0666);

    int fd;
    fd = open(fifo_server, O_WRONLY);
    write(fd, cl_dat->fifo, sizeof(cl_dat->fifo)+10);
    close(fd);

    char res[2048];

    fd = open(cl_dat->fifo, O_RDONLY);
    read(fd, res, 1024);
    printf("Client %d [%d]: %s\n\n", cl_dat->index, cl_dat->pid, res);
    close(fd);

    exit(0);
}