#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <time.h>

/* Definizione chiavi per semafori e shared memory */
#define KEYSEMP1 7777
#define KEYSEMP2 8888
#define KEYSHM 9999

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
    struct seminfo  *__buf;
};

int set_sem(int sem_id, int val);
int sem_up(int sem_id);
int sem_down(int sem_id);

int main() {
    int sem_p1, sem_p2, shm_id, run = 1;
    int dado_p2;
    int *dado;
    
    /* Creazione/ottenimento id dei due semafori */
    sem_p1 = semget((key_t)KEYSEMP1, 1, 0666 | IPC_CREAT);
    if(sem_p1 == -1) {
        perror("Errore semget");
        exit(EXIT_FAILURE);
    }
    
    sem_p2 = semget((key_t)KEYSEMP2, 1, 0666 | IPC_CREAT);
    if(sem_p1 == -1) {
        perror("Errore semget");
        exit(EXIT_FAILURE);
    }

    /* Creazione/ottenimento id della shared memory */
    shm_id = shmget((key_t)KEYSHM, sizeof(int), 0666 | IPC_CREAT);
    if(shm_id == -1) {
        perror("Errore shmget");
        exit(EXIT_FAILURE);
    }
    
    /* Attach della shared memory */
    dado = (int *)shmat(shm_id, NULL, 0);
    if(dado == NULL) {
        perror("Errore shmat");
        exit(EXIT_FAILURE);
    }
    
    /* Avvio del ciclo di gioco */
    while(run == 1) {
        sem_down(sem_p2); /* Attende che p1 faccia la down sul semaforo di p2 */
        printf("Valore dado P1: %d\n", (*dado));
        if((*dado) == 0) { /* Se dado == 0 il gioco è finito */
            run = 0;
        } else {
            dado_p2 = (rand() % 6) + 1; /* lancia il proprio dado */
            printf("Valore dado P2: %d\n", dado_p2);
            if(dado_p2 > (*dado)) { /* Controlla e comunica il vincitore */
                printf("Vince P2\n");
            } else if(dado_p2 < (*dado)) {
                printf("Vince P1\n");
            } else {
                printf("Parita'\n");
            }
        }
        fflush(stdout);
        sem_up(sem_p1); /* Sblocca il semaforo di p1 */
    }
    
    /* Al termine del gioco p2 fa la detach della shared memory e la rimuove,
       inoltre vengono rimossi anche i semafori */
    shmdt(dado);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_p1, 0, IPC_RMID);
    semctl(sem_p2, 0, IPC_RMID);
    printf("Ciao da P2\n");
}

/* Funzioni di utilita' per la gestione dei semafori */
int set_sem(int sem_id, int val) {
    union semun un;
    un.val = val;
    return semctl(sem_id, 0, SETVAL, un);
}

int sem_up(int sem_id) {
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = 1;
    buf.sem_flg = SEM_UNDO;
    return semop(sem_id, &buf, 1);
}

int sem_down(int sem_id) {
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.sem_flg = SEM_UNDO;
    return semop(sem_id, &buf, 1);
}