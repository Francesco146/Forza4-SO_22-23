#include "../include/Forza4.h"

// global variables:
int ROWS = 0, COLS = 0, WIN = 0;

char **scores;
char header[MAX_COL];
char horbar[MAX_COL];
int bin = 0;
bool randomMode = !DEFAULT_MODE;
bool got2Players = DEFAULT_MODE;
char giocatoreCorrente = P1CHAR;
char player1 = P1CHAR;
char player2 = P2CHAR;
static int count = 1;
int shm_id;

void create_shared_matrix() {
    scores = (char**)shmat(shm_id, NULL, 0);
    if (scores == (char**)-1) {
        perror("shmat");
        exit(1);
    }
    for (int i = 0; i < ROWS; i++)
        scores[i] = (char*)(scores + ROWS) + i * COLS;
}

// function to detach the shared memory
void detach_shared_matrix() {
    // Detach the shared memory segment
    if (shmdt(scores) == -1) {
        perror("Error detaching shared memory segment");
        exit(EXIT_FAILURE);
    }
    // Destroy the shared memory segment
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("Error destroying shared memory segment");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    // set the SIGINT handler:
    signal(SIGINT, sigint_handler);
    signal(SIGALRM, reset_count);

    for (int i = 0; i < 32; i++)
        if (i != SIGINT && i != SIGALRM)
            signal(i, SIG_IGN);

    srand(time(NULL));
    parse_args(argc, argv);

    shm_id = shmget(IPC_PRIVATE, ROWS * sizeof(char*) + ROWS * COLS * sizeof(char), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    create_shared_matrix();

    inizializza_matrice();
    stampa_matrice();
    stampa_tavolo();
    bool pareggio = 0;
    do {
        printf("Giocatore %c, è il tuo turno!\n", giocatoreCorrente);
        scegli_colonna();
        if (tavoloPieno())
            pareggio = 1;
        printf("\n\n");
        stampa_tavolo();
    } while (!(controllaVincitore(player1) || controllaVincitore(player2)) &&
             !pareggio);
    if (pareggio)
        printf("\n\nPareggio!\n\n");
    else
        printf("\n\nPlayer %c wins!\n\n",
               (giocatoreCorrente == player1) ? player2 : player1);
    detach_shared_matrix();
    return 0;
}

void inizializza_matrice(void) {
    // fill the scores with the empty character:
    for (int riga = 0; riga < ROWS; riga++)
        for (int colonna = 0; colonna < COLS; colonna++)
            scores[riga][colonna] = EMPTY;
}

void stampa_matrice(void) {
    // print the initialized scores, no scores is printed yet:
    for (int riga = 0; riga < ROWS; riga++) {
        for (int colonna = 0; colonna < COLS; colonna++)
            printf("%c ", scores[riga][colonna]);
        printf("\n");
    }
}

void stampa_tavolo(void) {
    // print the scores and the scores according to current game:
    printf("%s\n", header);
    printf("%s\n", horbar);

    for (int riga = 0; riga < ROWS; riga++) {
        for (int colonna = 0; colonna < COLS; colonna++)
            printf("| %c ", scores[riga][colonna]);
        printf("|\n");
        printf("%s\n", horbar);
    }
}

void scegli_colonna(void) {
    // if mode is 1, the function chooses a random column, otherwise it asks the
    // user to choose a column:
    printf("\n\n");
    if (randomMode) {
        int colonnaRaw = 0;
        do {
            colonnaRaw = rand() % COLS + 1;
        } while (colonnaRaw > COLS || scores[0][colonnaRaw - 1] != EMPTY);
        bin = colonnaRaw - 1;
        riempi_colonna();
        giocatoreCorrente = (giocatoreCorrente == player1) ? player2 : player1;
        randomMode = !randomMode;
        return;
    }
    int rawInput = 0;
    do {
        fflush(stdin);
        printf("\nScegli la colonna: ");
        scanf(" %d", &rawInput);
        if (rawInput <= 0 || rawInput > COLS || isalpha(rawInput)) {
            printf("\nScelta errata, riprova.\n\n");
            stampa_tavolo();
        } else if (scores[0][rawInput - 1] != EMPTY) {
            printf("\nColonna piena, riprova.\n\n");
            stampa_tavolo();
        }
    } while (rawInput <= 0 || rawInput > COLS || isalpha(rawInput) ||
             scores[0][rawInput - 1] != EMPTY);
    bin = rawInput - 1;
    riempi_colonna();
    if (!got2Players)
        randomMode = !randomMode;
    giocatoreCorrente = (giocatoreCorrente == player1) ? player2 : player1;
}

void riempi_colonna(void) {
    // fills the bin according to what's already in there:
    /* lowest level or bottom of the scores */
    for (int level = ROWS - 1; level >= 0; level--)
        if (scores[level][bin] == EMPTY) {
            scores[level][bin] = giocatoreCorrente;
            break;
        }
}

bool controlloRiga(char giocatoreCorrente) {
    // Controllo righe
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j <= COLS - WIN; j++) {
            int trovato = 1;
            for (int k = 0; k < WIN; k++)
                if (scores[i][j + k] != giocatoreCorrente) {
                    trovato = 0;
                    break;
                }
            if (trovato)
                return 1;
        }
    return 0;
}

bool controlloColonna(char giocatoreCorrente) {
    // Controllo colonne
    for (int i = 0; i <= ROWS - WIN; i++)
        for (int j = 0; j < COLS; j++) {
            int trovato = 1;
            for (int k = 0; k < WIN; k++)
                if (scores[i + k][j] != giocatoreCorrente) {
                    trovato = 0;
                    break;
                }
            if (trovato)
                return 1;
        }
    return 0;
}

bool controlloDiagonaleSinistraADestra(char giocatoreCorrente) {
    // Controllo diagonali da sinistra a destra
    for (int i = 0; i <= ROWS - WIN; i++)
        for (int j = 0; j <= COLS - WIN; j++) {
            int trovato = 1;
            for (int k = 0; k < WIN; k++)
                if (scores[i + k][j + k] != giocatoreCorrente) {
                    trovato = 0;
                    break;
                }
            if (trovato)
                return 1;
        }
    return 0;
}

bool controlloDiagonaleDestraASinistra(char giocatoreCorrente) {
    // Controllo diagonali da destra a sinistra
    for (int i = 0; i <= ROWS - WIN; i++)
        for (int j = WIN - 1; j < COLS; j++) {
            int trovato = 1;
            for (int k = 0; k < WIN; k++)
                if (scores[i + k][j - k] != giocatoreCorrente) {
                    trovato = 0;
                    break;
                }
            if (trovato)
                return 1;
        }
    return 0;
}

bool controlloDiagonali(char giocatoreCorrente) {
    return controlloDiagonaleSinistraADestra(giocatoreCorrente) ||
           controlloDiagonaleDestraASinistra(giocatoreCorrente);
}

bool tavoloPieno(void) {
    // doc: controlla se il tavolo è pieno
    for (int i = 0; i < COLS; i++)
        if (scores[0][i] == EMPTY)
            return 0;
    return 1;
}

bool controllaVincitore(char giocatoreCorrente) {
    return controlloRiga(giocatoreCorrente) ||
           controlloColonna(giocatoreCorrente) ||
           controlloDiagonali(giocatoreCorrente);
}

void parse_args(int argc, char *argv[]) {
    if (argc == 7) {
        if (strcmp(argv[1], "-c") == 0 && strcmp(argv[3], "-r") == 0 &&
            strcmp(argv[5], "-w") == 0) {
            COLS = atoi(argv[2]);
            ROWS = atoi(argv[4]);
            WIN = atoi(argv[6]);
        } else if (strcmp(argv[1], "-COLS") == 0 &&
                   strcmp(argv[3], "-ROWS") == 0 &&
                   strcmp(argv[5], "-WIN") == 0) {
            COLS = atoi(argv[2]);
            ROWS = atoi(argv[4]);
            WIN = atoi(argv[6]);
        } else {
            printf("Usage: ./connect4 -c 7 -r 6 -w 4 OR ./connect4 -COLS 7 "
                   "-ROWS 6 -WIN 4\n");
            exit(1);
        }
    } else if (argc == 1) {
        // default values
        COLS = 7;
        ROWS = 6;
        randomMode = 0;
        WIN = 4;
    }
    // if ./connect4 * is called, the program will choose random mode
    else if (argc == 2 && strcmp(argv[1], "*") == 0) {
        randomMode = 1;
        got2Players = 0;
        COLS = 7;
        ROWS = 6;
        WIN = 4;
    } else {
        printf("Usage: ./connect4 -c 7 -r 6 -w 4 OR ./connect4 -COLS 7 -ROWS 6 "
               "-win 4\n");
        exit(1);
    }

    sprintf(header, " ");
    for (int i = 1; i <= COLS; i++)
        sprintf(header + strlen(header), " %d  ", i);

    // Inizializzazione della stringa horbar
    horbar[0] = '|'; // Primo carattere della barra sinistra
    for (int i = 0; i < COLS; i++) {
        horbar[4 * i + 1] = '-'; // Prima barra orizzontale
        horbar[4 * i + 2] = '-'; // Seconda barra orizzontale
        horbar[4 * i + 3] = '-'; // Terza barra orizzontale
        horbar[4 * i + 4] = '|'; // Carattere della barra destra
    }
    horbar[4 * COLS + 1] = '\0'; // Terminatore di stringa

    //scores = malloc(sizeof(char *) * ROWS);
    //for (int i = 0; i < ROWS; i++)
    //   scores[i] = malloc(sizeof(char) * COLS);
}

// function called at SIGINT:
void sigint_handler(int sig __attribute__((unused))) {
    // check if number of SIGINT is 2, if so, exit the program:
    if (count++ == N_SIGINT) {
        printf("\n\nRicevuti %i CTRL+C, sto uscendo...\n\n", count - 1);
        detach_shared_matrix();
        exit(0);
    }
    alarm(N_SEC);
}

void reset_count(int sig __attribute__((unused))) {
    count = 1;
    printf("\n\nNon hai premuto CTRL+C di nuovo entro %i secondi. Il contatore "
           "è stato resettato.\n\n",
           N_SEC);
    stampa_tavolo();
    // set up the handler for SIGINT again:
    signal(SIGINT, sigint_handler);
}
