#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/shm.h>
#include <sys/sem.h>

#define N_SIGINT 2
#define N_SEC 5
#define P1CHAR 'X'
#define P2CHAR 'O'
#define EMPTY ' '
#define MAX_COL 100

// default mode is 1 (human vs human)
#define DEFAULT_MODE 1

//#define INLINE inline __attribute__((always_inline))

// function prototypes:
void /*INLINE*/ inizializza_matrice(void);
void /*INLINE*/ stampa_matrice(void);
void /*INLINE*/ stampa_tavolo(void);
void /*INLINE*/ scegli_colonna(void);
void /*INLINE*/ riempi_colonna(void);
void /*INLINE*/ parse_args(int, char *[]);
bool /*INLINE*/ controllaVincitore(char);
bool /*INLINE*/ tavoloPieno(void);
bool /*INLINE*/ controlloDiagonaleDestraASinistra(char);
bool /*INLINE*/ controlloDiagonaleSinistraADestra(char);
bool /*INLINE*/ controlloDiagonali(char);
bool /*INLINE*/ controlloColonna(char);
bool /*INLINE*/ controlloRiga(char);
void sigint_handler(int __attribute__((unused)));
void reset_count(int __attribute__((unused)));
