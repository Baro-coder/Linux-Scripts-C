#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include <time.h>

void writer_do();
void sig_handler(int signum);
void read_reader_pid();
void write_own_pid();

char * filename_in = "/etc/profile";
char * filename_out = "tmp.txt";

int reader_pid = 0;

int ready = 0;
int line_count = 0;

void writer_do()
{
    FILE *f_in_wrt = fopen(filename_in, "r"); // opening the source file

    char line[100]; // read line

    if(f_in_wrt == NULL)
    {
        // error opening file
        fprintf(stderr, "Error opening file:\t%s\n\n", getpid(), filename_in);
        kill(reader_pid, SIGUSR2);
    }
    else
    {
        int i = 0;

        // selecting the specific line
        while(i != line_count)
        {
            if (fgets(line, sizeof(line), f_in_wrt) != NULL){
                i++;
            }
            else{
                fprintf(stderr, "Error read\n", getpid());
                kill(reader_pid, SIGUSR2);
            }
        }
        if (fgets(line, sizeof(line), f_in_wrt) != NULL){
            line_count++; // increase read line counter
        }
        else
        {
            kill(reader_pid, SIGUSR2);
            kill(getpid(), SIGUSR2);
        }
        fclose(f_in_wrt); // closing the source file
    }

    int f_out_wrt = open(filename_out, O_WRONLY | O_CREAT | O_TRUNC, 0644); // opening the output file

    if (f_out_wrt == -1)
    {
        fprintf(stderr, "Error opening file:\t%s\n\n", getpid(), filename_out);
        kill(reader_pid, SIGUSR2);
    }
    else
    {
        int i = 0;
        char c = line[i];

        // writing line to the output file
        while(c != '\0'){
            write(f_out_wrt, &c, sizeof(c));
            i++;
            c = line[i];
        }
    }

    // closing the output file
    if(close(f_out_wrt) < 0)
    {
        fprintf(stderr, "Error closing file:\t%s\n\n", getpid(), filename_out);
        kill(reader_pid, SIGUSR2);
    }
}

void sig_handler(int signum)
{
    if(signum == SIGUSR1)
    {
        if(ready == 0)
        {
            ready = 1;
            read_reader_pid();
            kill(reader_pid, SIGUSR1);
        }
        else
        {
            writer_do();
            kill(reader_pid, SIGUSR1);
        }
    }
    else if(signum == SIGUSR2)
    {
        if(remove(filename_out) != 0)   // removing auxillary file
        {
            fprintf(stderr, "Error removing file: %s\n", filename_out);
            exit(-1);
        }
        kill(getpid(), SIGINT);
    }
}

void read_reader_pid()
{
    FILE * tmp;
    tmp = fopen(filename_out, "r");
    if(tmp != NULL)
    {
        fscanf(tmp, "%d", &reader_pid);
        fclose(tmp);
    } else {
        fprintf(stderr, "Error opening file: %s\n", filename_out);
        exit(-1);
    }
}

void write_own_pid()
{
    FILE * tmp;
    tmp = fopen(filename_out, "w");
    if(tmp != NULL)
    {
        fprintf(tmp, "%d", getpid());
        fclose(tmp);
    } else {
        fprintf(stderr, "Error opening file: %s\n", filename_out);
        exit(-1);
    }
}

int main()
{
    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);

    write_own_pid();

    while(1) { ;; }

    return 0;
}