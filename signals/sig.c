/*
    -----------------------------------
    | Signal sync  - multithreading C |
    -----------------------------------
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
#include <sys/wait.h>
#include <fcntl.h> 
#include <time.h>

/*
     ----------------
    -- DECLARATIONS --
     ----------------
*/
void sig_handler_write(int signum);     // sig handler for process write
void sig_handler_display(int signum);   // sig handler for process display
void sig_handler_main(int signum);      // sig handler for process main
void p_write_run();             // main function for process write
void p_display_run();           // main function for process display
void write_src_line();          // read line from source file and write to output file by process write
void display_line();            // read and display line from output file by proces display

/*
     ---------------
    -- GLOBAL VARS --
     ---------------
*/
// PIDs
pid_t p_write;
pid_t p_display;

int turn = 0;   // 0 - p_write's turn | 1 - p_display's turn
int ready = 0;  // 0 - no process ready | 1 - one process ready | 2 - two processes ready

// Files
const char *filename_in  = "./sig.c";   // File to read
const char *filename_out = "./tmp.txt"; // File to write and read

int line_count = 0; // Read lines counter

// -----------------------------------------------------------------------
/*
     --------------
    -- MAIN DRIVE --
     --------------
*/
int main(int argc, char *argv[])
{
    p_display = fork(); // create process to display read lines

    if(p_display == 0){
        // process display function
        p_display_run();
    }
    else if(p_display == -1){
        // process display failed
        fprintf(stderr, "Process_Display Failed...\n");
        exit(-1);
    }
    else{
        p_write = fork();  // create process to read source file

        if(p_write == 0){
            // process write function
            p_write_run();
        }
        else if(p_write == -1){
            // process write failed
            fprintf(stderr, "Process_Write Failed...\n");
            exit(-1);
        }
        else{
            
            signal(SIGUSR1, sig_handler_main); // main signal SIGUSR1 interpreter 
            signal(SIGUSR2, sig_handler_main); // main signal SIGUSR2 interpreter
            wait(NULL); // waiting for processes to finish
        }
    }

    if(remove(filename_out) != 0)   // removing auxillary file
    {
        fprintf(stderr, "MAIN: Error removing file: %s\n", filename_out);
        exit(-1);
    }

    exit(0);
}

// -----------------------------------------------------------------------
/*
     ----------------
    -- SIG HANDLERS --
     ----------------
*/
void sig_handler_write(int signum)
{
    if(signum == SIGUSR1)
    {  
        write_src_line(); // do an iteration
        kill(getppid(), SIGUSR1); // confirm finish the iteration
    }
    else if(signum == SIGUSR2)
    {
        kill(getpid(), SIGINT); // end of process
    }
    else
    {
        // Unknown signal received
        fprintf(stderr, "Process_Write[%d]: Received: Unknown signal, Code: %d.\n", getpid(), signum);
    }
}

void sig_handler_display(int signum)
{
    if(signum == SIGUSR1)
    {  
        display_line(); // do an iteration
        kill(getppid(), SIGUSR1); // confirm finish the iteration
    }
    else if(signum == SIGUSR2)
    {
        kill(getpid(), SIGINT); // end of process
    }
    else
    {
        // Unknown signal received
        fprintf(stderr, "Process_Display[%d]: Received: Unknown signal, Code: %d\n", getpid(), signum);
    }
}

void sig_handler_main(int signum)
{
    if(signum == SIGUSR1)
    {
        if(ready < 2)
        {
            // waiting for processes' readiness
            ready++;
            if(ready == 2)
            {
                kill(getpid(), SIGUSR1); // procedure start
            }
        }
        else
        {
            // Procedure starting
            if(turn == 0){
                // Process Write's turn
                turn = 1;
                kill(p_write, SIGUSR1);
            } else {
                // Process Display's turn
                turn = 0;
                kill(p_display, SIGUSR1);
            }
        }
    }
    else if(signum == SIGUSR2)
    {
        // End of procedure
        kill(p_write, SIGUSR2);
        kill(p_display, SIGUSR2);
    }
    else
    {
        // Unknown signal received
        fprintf(stderr, "MAIN: Received: Unknown Signal, Code: %d\n", signum);
    }
}

// -----------------------------------------------------------------------
/*
     -----------------
    -- PROCESS FUNCS --
     -----------------
*/
void p_write_run()
{
    signal(SIGUSR1, sig_handler_write); // process signal SIGUSR1 interpreter 
    signal(SIGUSR2, sig_handler_write); // process signal SIGUSR2 interpreter 

    kill(getppid(), SIGUSR1); // readiness call

    while(1){ ;; } // infinite loop

    return;
}

void p_display_run()
{
    signal(SIGUSR1, sig_handler_display); // process signal SIGUSR1 interpreter 
    signal(SIGUSR2, sig_handler_display); // process signal SIGUSR2 interpreter 

    kill(getppid(), SIGUSR1); // readiness call

    while(1){ ;; } // infinite loop

    return;
}


void write_src_line()
{
    FILE *f_in_wrt = fopen(filename_in, "r"); // opening the source file

    char line[100]; // read line

    if(f_in_wrt == NULL)
    {
        // error opening file
        fprintf(stderr, "Process_Write[%d]: error opening file:\t%s\n\n", getpid(), filename_in);
        kill(getppid(), SIGUSR2);
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
                fprintf(stderr, "Process_Write[%d]: error read\n", getpid());
                kill(getppid(), SIGUSR2);
            }
        }
        if (fgets(line, sizeof(line), f_in_wrt) != NULL){
            line_count++; // increase read line counter
        }
        else
        {
            kill(getppid(), SIGUSR2);
        }
        fclose(f_in_wrt); // closing the source file
    }

    int f_out_wrt = open(filename_out, O_WRONLY | O_CREAT | O_TRUNC, 0644); // opening the output file

    if (f_out_wrt == -1)
    {
        fprintf(stderr, "Process_Write[%d]: error opening file:\t%s\n\n", getpid(), filename_out);
        kill(getppid(), SIGUSR2);
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
        fprintf(stderr, "Process_Write[%d]: error closing file:\t%s\n\n", getpid(), filename_out);
        kill(getppid(), SIGUSR2);
    }
}


void display_line()
{
    int f_out_disp = open(filename_out, O_RDONLY, 0); // opening the output file to read

    if(f_out_disp == -1)
    {
        fprintf(stderr, "Process_Display[%d]: error opening file: %s\n", getpid(), filename_out);
        kill(getppid(), SIGUSR2);
    }
    else
    {
        char *line = (char *) calloc(100, sizeof(char));
        
        read(f_out_disp, line, 100); // reading the line

        printf("%s", line); // display the line

        // closing the file
        if(close(f_out_disp) < 0)
        {
            fprintf(stderr, "Process_Display[%d]: error closing file:\t%s\n\n", getpid(), filename_out);
            kill(getppid(), SIGUSR2);
        }
    }
}
