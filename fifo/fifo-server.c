/*
    -------------------------------------
    | Inter-process comunication - FIFO |
    |          -- SERVER --             |
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
void sig_handler(int signo);
char * cmd_ls();

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
int main()
{
    int fd;
    char * mes;
    char res[80];

    signal(SIGINT, sig_handler);

    mkfifo(fifo_server, 0666);

    mes = (char *) malloc(512 * sizeof(char));

    while(1)
    {
        fd = open(fifo_server, O_RDONLY);

        read(fd, res, sizeof(res));
        printf(" > Write to %s\n", res);

        close(fd);

        char * out = cmd_ls();

        strcpy(mes, out);

        //printf("   # %s\n\n   # %s\n\n", mes, out);

        fd = open(res, O_WRONLY);
        write(fd, out, 100 * sizeof(out));    // Here is PROBLEM !!!
                                            // out is not written 
                                            //  in client's fifo
        close(fd);
    }

    exit(0);
}

char * cmd_ls()
{
    FILE *fpipe;
    char *cmd = "ls";
    char * out = (char *) malloc(512 * sizeof(char));

    if(0 == (fpipe = (FILE*)popen(cmd, "r")))
    {
        fprintf(stderr, "'ls' command failed!\n");
        exit(-1);
    }
    fread(out, 2, 256, fpipe);
    pclose(fpipe);

    int i = 0;
    char c = out[i];
    while(c != 0){
        
        if(c == '\n')
        {
            out[i] = ' ';
        }

        i++;
        c = out[i];
    }

    return out;
}

void sig_handler(int signo)
{
    if(signo == SIGINT)
    {
        system("rm fifo_*");
        exit(0);
    }
}