// Biblioteki
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

// Struktura wynikowa watku
struct watek_wynik
{
    double wartosc;
};

// Deklaracje
double licz(double p);      // funkcja obliczeniowa dla watku
void *run(void *arg);       // funkcja watku

// Main
int main()
{
    double H = 0;           // wartosc entropii
    double suma_skladnikow = 0;    // suma skladnikow
    int i = 1;              // indeks parametru p

    while(1)
    {
        pthread_t tid;             // id watku
        void* t_out;               // wynik watku
        struct watek_wynik *out;   // wynik watku do odczytu z watku glownego
        double p;                  // parametr p

        printf("p%d = ", i);
        scanf("%lf", &p);       // odczytanie parametru p z stdin

        pthread_create(&tid, NULL, &run, (void *)&p);  // uruchomienie watku
        pthread_join(tid, &t_out);           // oczekiwanie na wynik
        out = t_out;

        // obliczenie entropii
        suma_skladnikow += out->wartosc;
        H = -suma_skladnikow;

        // inkrementacja indeksu parametru p             
        i++;

        // Wypisanie informacji o zakonczonym watku
        printf("\nThread[%ld]: wynik = %lf\n", tid, out->value);    // wynik watku
        printf("\tH = %lf\n", H);                                   // aktualna wartosc H

        // Sprawdzenie czy suma skladnikow nie przekracza wartosci 1
        if(suma_skladnikow > 1)
        {
            printf("\nSuma skladnikow jest wieksza od 1 o %lf\n", suma_skladnikow-1);
            break;
        }
        else if(suma_skladnikow == 1)
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
    double wartosc = calc(*p);

    struct watek_wynik *wynik = malloc(sizeof *wynik);
    wynik->wartosc = wartosc;

    return wynik;
}

double licz(double p)
{
    return p * log2(p);
}