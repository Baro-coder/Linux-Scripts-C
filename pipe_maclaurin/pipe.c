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
#include <math.h>
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
void PP3_do();
double calc_component(double x, int i);
double factorial(int n);

// PIDs
pid_t pp1, pp2, pp3;

// PIPEs
int pipe1[2];   // pipe PP1 > PP2
int pipe2[2];   // pipe PP2 > PP3
int pipe3[2];   // pipe PP3 > PP2
int pipe4[2];   // pipe PP3 > PP1


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
            if((pp3 = fork()) == 0)
            {
                // PP3 func
                PP3_do();
            }
            else
            {
                wait(NULL);
            }
        }
    }

    return 0;
}

/*
     -------------
    -- FUNCTIONS --
     -------------
*/
void PP1_do()
{
    char x_str[BUFFERSIZE];
    char eps_str[BUFFERSIZE];
    char data[BUFFERSIZE*2];
    char resp[BUFFERSIZE];

    // close unnecessary channels
    close(pipe1[READ]);
    close(pipe2[READ]);
    close(pipe2[WRITE]);
    close(pipe3[READ]);
    close(pipe3[WRITE]);
    close(pipe4[WRITE]);

    while(true)
    {
        printf(" PP1[%d]: Podaj X: \n\t> ", getpid());
        scanf("%s", &x_str);
        printf(" PP1[%d]: Podaj Eps: \n\t> ", getpid());
        scanf("%s", &eps_str);

        sprintf(data, "%s|%s", x_str, eps_str);

        write(pipe1[WRITE], data, strlen(data));

        read(pipe4[READ], resp, sizeof(resp));
    }
}

void PP2_do()
{
    char x_str[BUFFERSIZE] = "";
    char eps_str[BUFFERSIZE] = "";
    char component[BUFFERSIZE] = "";
    char data[BUFFERSIZE*2];
    char *eptr;

    int i = 0;
    bool snd = false;
    char c = 0;

    double x;
    double eps;

    // close unnecessary channels
    close(pipe1[WRITE]);
    close(pipe2[READ]);
    close(pipe3[WRITE]);
    close(pipe4[READ]);
    close(pipe4[WRITE]);

    while(true)
    {
        read(pipe1[READ], data, sizeof(data));

        // Convert data from PP1 to double x, eps
        i = 0;
        c = data[i];
        while(c != '\0')
        {
            if(snd == false)
            {
                if(c == '|')
                {
                    snd = true;
                }
                else
                {
                    sprintf(x_str, "%s%c", x_str, c);
                }
            }
            else
            {
                sprintf(eps_str, "%s%c", eps_str, c);
            }
            i++;
            c = data[i];
            data[i-1] = '\0';
        }
        x = strtod(x_str, &eptr);
        eps = strtod(eps_str, &eptr);

        snd = false;
        strcpy(x_str, "");
        strcpy(eps_str, "");

        // Calculating the components
        i = 0;
        while(true)
        {
            double result = calc_component(x, i);
            i++;

            sprintf(component, "%f", result);
            write(pipe2[WRITE], component, strlen(component));

            read(pipe3[READ], data, sizeof(data));

            if(result == 0 || (result < eps && result > -eps) || isnan(result) || isinf(result))
            {
                result = 0;
                sprintf(component, "%f", result);
                write(pipe2[WRITE], component, strlen(component));
                read(pipe3[READ], data, sizeof(data));
                break;
            }
        }
    }
}

void PP3_do()
{
    char cmp_str[BUFFERSIZE];
    char resp[BUFFERSIZE] = "ready";
    char *eptr;
    double cmp;
    double sum = 0.0;

    // close unnecessary channels
    close(pipe1[READ]);
    close(pipe1[WRITE]);
    close(pipe2[WRITE]);
    close(pipe3[READ]);
    close(pipe4[READ]);

    while(true)
    {   
        read(pipe2[READ], cmp_str, sizeof(cmp_str));
        write(pipe3[WRITE], resp, sizeof(resp));

        cmp = strtod(cmp_str, &eptr);

        if(cmp == 0 || (cmp > -0.000001 && cmp < 0.000001))
        {
            printf("\nPP3[%d]: Sum = %f\n", getpid(), sum);
            sum = 0;
            write(pipe4[WRITE], resp, sizeof(resp));
        }
        else
        {
            sum += cmp;
        }

        cmp = 0;
    }

}

double calc_component(double x, int i)
{
    double pow1 = pow(-1, i);
    double pow2 = pow(x, 2*i);
    double fact = factorial(2*i);
    double result = (pow1 * pow2) / fact;

    return result;
}

double factorial(int n)
{
    int c;
    double result = 1;

    for (c = 1; c <= n; c++){
        result = result * c;
    }

    return result;
}