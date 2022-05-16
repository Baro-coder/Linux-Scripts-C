/*
    ---------------------------------------
    | Sygnały - Synchronizacja procesów C |
    ---------------------------------------
*/

/*
     --------------
    -- Biblioteki --
     --------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> 
#include <time.h>

/*
     --------------
    -- Deklaracje --
     --------------
*/
void sig_handler_write(int signum);     // sig handler for process write
void sig_handler_display(int signum);   // sig handler for process display
void sig_handler_main(int signum);      // sig handler for process main
void p_write_run();             // main function for process write
void p_display_run();           // main function for process display
void write_src_line();          // read line from source file and write to output file by process write
void display_line();            // read and display line from output file by proces display

/*
     --------------------
    -- Zmienne globalne --
     --------------------
*/
// PIDs
pid_t p_write;
pid_t p_display;

int turn = 0;   // 0 - operacja procesu 'write' | 1 - operacja procesu 'display'
int ready = 0;  // licznik gotowości procesów

// Files
const char *filename_in  = "./sig.c";   // Plik źródłowy
const char *filename_out = "./tmp.txt"; // Plik pomocniczy

int line_count = 0; // Licznik odczytanych linii

// -----------------------------------------------------------------------
/*
     ------------------
    -- Funkcja główna --
     ------------------
*/
int main(int argc, char *argv[])
{
    p_display = fork(); // powołanie procesu wyświetlającego linie

    if(p_display == 0){
        p_display_run();
    }
    else if(p_display == -1){
        fprintf(stderr, "Process_Display Failed...\n");
        exit(-1);
    }
    else{
        p_write = fork();  // powołanie procesu czytającego plik źródłowy

        if(p_write == 0){
            p_write_run();
        }
        else if(p_write == -1){
            fprintf(stderr, "Process_Write Failed...\n");
            exit(-1);
        }
        else{
            
            signal(SIGUSR1, sig_handler_main); // interpreter sygnału SIGUSR1 dla procesu głównego 
            signal(SIGUSR2, sig_handler_main); // interpreter sygnału SIGUSR2 dla procesu głównego
            wait(NULL); // oczekiwanie na zakończenie procesów potomnych
        }
    }

    if(remove(filename_out) != 0)   // usuwanie pliku pomocniczego
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
        write_src_line(); // wykonanie iteracji
        kill(getppid(), SIGUSR1); // potwierdzenie wykonania iteracji
    }
    else if(signum == SIGUSR2)
    {
        kill(getpid(), SIGINT); // zakończenie procesu
    }
    else
    {
        // Nieznany sygnał
        fprintf(stderr, "Process_Write[%d]: Received: Unknown signal, Code: %d.\n", getpid(), signum);
    }
}

void sig_handler_display(int signum)
{
    if(signum == SIGUSR1)
    {  
        display_line(); // wykonanie iteracji
        kill(getppid(), SIGUSR1); // potwierdzenie wykonania iteracji
    }
    else if(signum == SIGUSR2)
    {
        kill(getpid(), SIGINT); // zakończenie procesu
    }
    else
    {
        // Nieznany sygnał
        fprintf(stderr, "Process_Display[%d]: Received: Unknown signal, Code: %d\n", getpid(), signum);
    }
}

void sig_handler_main(int signum)
{
    if(signum == SIGUSR1)
    {
        if(ready < 2)
        {
            // oczekiwanie na gotowość wszystkich procesów potomnych
            ready++;
            if(ready == 2)
            {
                kill(getpid(), SIGUSR1); // rozpoczęcie procedury
            }
        }
        else
        {
            // rozpoczęcie procedury
            if(turn == 0){
                // Operacja procesu 'write'
                turn = 1;
                kill(p_write, SIGUSR1);
            } else {
                // Operacja procesu 'display'
                turn = 0;
                kill(p_display, SIGUSR1);
            }
        }
    }
    else if(signum == SIGUSR2)
    {
        // Zakończenie procedury
        kill(p_write, SIGUSR2);
        kill(p_display, SIGUSR2);
    }
    else
    {
        // Nieznany sygnał
        fprintf(stderr, "MAIN: Received: Unknown Signal, Code: %d\n", signum);
    }
}

// -----------------------------------------------------------------------
/*
     ------------------------------
    -- Funkcje procesów potomnych --
     ------------------------------
*/
void p_write_run()
{
    signal(SIGUSR1, sig_handler_write); // interpreter sygnału SIGUSR1 dla procesu 'write'
    signal(SIGUSR2, sig_handler_write); // interpreter sygnału SIGUSR2 dla procesu 'write'

    kill(getppid(), SIGUSR1); // potwierdzenie gotowości

    while(1){ ;; } // pętla nieskończona

    return;
}

void p_display_run()
{
    signal(SIGUSR1, sig_handler_display); // interpreter sygnału SIGUSR1 dla procesu 'display'
    signal(SIGUSR2, sig_handler_display); // interpreter sygnału SIGUSR2 dla procesu 'display'

    kill(getppid(), SIGUSR1); // potwierdzenie gotowości

    while(1){ ;; } // pętla nieskończona

    return;
}


void write_src_line()
{
    FILE *f_in_wrt = fopen(filename_in, "r"); // otwarcie pliku źródłowego

    char line[100];

    if(f_in_wrt == NULL)
    {
        // błąd otwarcia pliku
        fprintf(stderr, "Process_Write[%d]: error opening file:\t%s\n\n", getpid(), filename_in);
        kill(getppid(), SIGUSR2);
    }
    else
    {
        int i = 0;

        // wybór odpowiedniej linii
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
            line_count++; // zwiększenie licznika odczytanych linii
        }
        else
        {
            kill(getppid(), SIGUSR2);
        }
        fclose(f_in_wrt); // zamknięcie pliku źródłowego
    }

    int f_out_wrt = open(filename_out, O_WRONLY | O_CREAT | O_TRUNC, 0644); // otwarcie pliku pomocniczego

    if (f_out_wrt == -1)
    {
        fprintf(stderr, "Process_Write[%d]: error opening file:\t%s\n\n", getpid(), filename_out);
        kill(getppid(), SIGUSR2);
    }
    else
    {
        int i = 0;
        char c = line[i];

        // zapisanie linii do pliku pomocniczego
        while(c != '\0'){
            write(f_out_wrt, &c, sizeof(c));
            i++;
            c = line[i];
        }
    }

    // zamknięcie pliku pomocniczego
    if(close(f_out_wrt) < 0)
    {
        fprintf(stderr, "Process_Write[%d]: error closing file:\t%s\n\n", getpid(), filename_out);
        kill(getppid(), SIGUSR2);
    }
}


void display_line()
{
    int f_out_disp = open(filename_out, O_RDONLY, 0); // otwarcie pliku pomocniczego

    if(f_out_disp == -1)
    {
        fprintf(stderr, "Process_Display[%d]: error opening file: %s\n", getpid(), filename_out);
        kill(getppid(), SIGUSR2);
    }
    else
    {
        char *line = (char *) calloc(100, sizeof(char));
        
        read(f_out_disp, line, 100); // odczytanie linii

        printf("%s", line); // wyświetlenie odczytanej linii

        // zamknięcie pliku pomocniczego
        if(close(f_out_disp) < 0)
        {
            fprintf(stderr, "Process_Display[%d]: error closing file:\t%s\n\n", getpid(), filename_out);
            kill(getppid(), SIGUSR2);
        }
    }
}
