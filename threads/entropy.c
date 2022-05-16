/*
    ------------------------------------------
    | Entropy calculating - multithreading C |
    ------------------------------------------
*/

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

// Thread result structure
struct thread_result
{
    double value;
};

// Declarations
double calc(double p);      // Thraed calculate function
void *run(void *arg);       // Thread function

// Main Drive
int main()
{
    double H = 0;           // entropy result
    double args_sum = 0;    // arguments sum
    int i = 1;              // p index

    while(1)
    {
        pthread_t tid;             // thread id
        void* t_out;               // thread result
        struct thread_result *out; // main read thread out
        double p;                  // p param

        printf("p%d = ", i);
        scanf("%lf", &p);       // read p from stdin

        pthread_create(&tid, NULL, &run, (void *)&p);  // run thread
        pthread_join(tid, &t_out);           // waiting for result
        out = t_out;

        // entropy calculating
        args_sum += out->value;
        H = -args_sum;

        // p index increment             
        i++;

        // MAIN Summary
        printf("\nThread[%ld]: wynik = %lf\n", tid, out->value);   // thread result
        printf("\tH = %lf\n", H);                                   // current H value

        // check the break condition: H is grater or equal 1
        if(args_sum > 1)
        {
            printf("\nSuma skladnikow jest wieksza od 1 o %lf\n", args_sum-1);
            break;
        }
        else if(args_sum == 1)
        {
            printf("\nSuma skladnikow jest rowna 1\n");
            break;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------------------------

void *run(void *arg)
{
    double *p = (double*)arg;
    double value = calc(*p);

    struct thread_result *res = malloc(sizeof *res);
    res->value = value;

    return res;
}

double calc(double p)
{
    return p * log2(p);
} 