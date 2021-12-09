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
    int sem_p1, sem_p2, shm_id, i;
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
    
    /* Inizializzazione dei semafori p1 parte sbloccato (ossia, 1) e p2 bloccato (ossia, 0) */
    set_sem(sem_p1, 1);
    set_sem(sem_p2, 0);
    
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
    
    srand(time(NULL));
    
    /* Avvio del ciclo di gioco */
    for(i = 0; i < 10; i++) {
        sem_down(sem_p1); /* Attende che il semaforo di p1 sia > 0 */
        (*dado) = (rand() % 6) + 1;
        printf("P1 valore dado: %d\n", (*dado));
        fflush(stdout);
        sem_up(sem_p2); /* Sblocca il semaforo di p2 */
    }
    
    /* Dopo 10 lanci viene scritto nella shared memory 0, per terminare p2 */
    sem_down(sem_p1);
    (*dado) = 0;
    printf("P1 valore dado: %d\n", (*dado));
    fflush(stdout);
    sem_up(sem_p2);
    
    /* Viene fatta la detach sulla shared memory */
    shmdt(dado);
    printf("Ciao\n");
    exit(EXIT_SUCCESS);
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