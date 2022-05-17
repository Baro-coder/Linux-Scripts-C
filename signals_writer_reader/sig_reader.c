#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include <time.h>

void reader_do();
void sig_handler(int signum);
void read_writer_pid();
void write_own_pid();

char * filename_in = "tmp.txt";
int writer_pid = 0;


void reader_do()
{
    int f_out_disp = open(filename_in, O_RDONLY, 0); // opening the output file to read

    if(f_out_disp == -1)
    {
        fprintf(stderr, "Error opening file: %s\n", getpid(), filename_in);
        kill(writer_pid, SIGUSR2);
    }
    else
    {
        char *line = (char *) calloc(100, sizeof(char));
        
        read(f_out_disp, line, 100); // reading the line

        printf("%s", line); // display the line

        // closing the file
        if(close(f_out_disp) < 0)
        {
            fprintf(stderr, "Error closing file:\t%s\n\n", getpid(), filename_in);
            kill(writer_pid, SIGUSR2);
        }
    }
}

void sig_handler(int signum)
{
    if(signum == SIGUSR1)
    {
        reader_do();
        kill(writer_pid, SIGUSR1);
    }
    else if(signum == SIGUSR2)
    {
        kill(getpid(), SIGINT);
    }
}

void read_writer_pid()
{
    FILE * tmp;
    tmp = fopen(filename_in, "r");
    if(tmp != NULL)
    {
        fscanf(tmp, "%d", &writer_pid);
        fclose(tmp);
    } else {
        fprintf(stderr, "Error opening file: %s\n", filename_in);
        exit(-1);
    }
}

void write_own_pid()
{
    FILE * tmp;
    tmp = fopen(filename_in, "w");
    if(tmp != NULL)
    {
        fprintf(tmp, "%d", getpid());
        fclose(tmp);
    } else {
        fprintf(stderr, "Error opening file: %s\n", filename_in);
        exit(-1);
    }
}

int main()
{
    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);

    read_writer_pid();
    write_own_pid();
    kill(writer_pid, SIGUSR1);

    while(1) { ;; }

    return 0;
}