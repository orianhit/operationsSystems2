#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#define NUM_OF_SEM 5

union semun
{
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO
                              (Linux-specific) */
};

void increase_other_sems(int semid, int sem_n )
{
    struct sembuf *sops = (struct sembuf *)malloc(NUM_OF_SEM * sizeof(struct sembuf *));
    for (int i = 0; i < NUM_OF_SEM; i++) {  
        sops[i].sem_num = i;
        sops[i].sem_flg = 0;
        if (i == sem_n) sops[i].sem_op = 0;
        else sops[i].sem_op = 1;
    }
    
    semop(semid, sops, NUM_OF_SEM);
    free(sops);
}

void wait_until_value_four(int semid, int sem_n )
{
    struct sembuf *sops = (struct sembuf *)malloc(1 * sizeof(struct sembuf *));
    sops[0].sem_num = sem_n;
    sops[0].sem_flg = 0;
    sops[0].sem_op = -4;

    semop(semid, sops, 1);
    free(sops);
}

int init_sems() {
    union semun smearg1;
    int semid = semget(IPC_PRIVATE, NUM_OF_SEM, 0600);
    smearg1.array = (short unsigned *)malloc(NUM_OF_SEM * sizeof(short unsigned));
    for (int i = 0; i < NUM_OF_SEM; i++)
        smearg1.array[i] = 4 - i;

    semctl(semid, 0, SETALL, smearg1);
    return semid;
}

void sem_action(int to_print, int sem_n)
{
    printf("%d\t\tfrom sem %d\n", to_print, sem_n + 1);
}

void print_with_all(int semid, int sem_n) {
        int counter = 0;
        int to_print = -1;
        while (to_print + NUM_OF_SEM <= 100) {
            to_print = counter * NUM_OF_SEM + (sem_n + 1);
            wait_until_value_four(semid, sem_n);

            sem_action(to_print, sem_n);

            increase_other_sems(semid, sem_n);

            counter++;
        }
        if (sem_n == 4) semctl(semid, 0 , IPC_RMID, 1);
}

int main()
{
    int semid = init_sems();

    for(int i =0; i < 4; i++) {
        if (fork() == 0) {
            print_with_all(semid, i);
            exit(1);
        }
    }
    print_with_all(semid, 4);

    return 0;
}