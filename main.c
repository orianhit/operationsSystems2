#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t mutex[5];

void increase_other_sems(int current_mux)
{
    for (int i = 0; i < 5; i++)
    {
        if (i != current_mux)
            sem_post(&mutex[i]);
    }
}

void decrease_mux_to_zero(int current_mux)
{
    for (int i = 1; i <= 4; i++)
        sem_wait(&mutex[current_mux]);
}

void wait_until_value_four(int current_mux)
{
    int sem_value;

    do
    {
        sem_getvalue(&mutex[current_mux], &sem_value);
    } while (sem_value < 4);
}

void sem_action(int to_print, int current_mux)
{
    printf("\t%d\t\tfrom sem %d\n", to_print, current_mux + 1);
}

void *print_with_five(void *arg)
{
    int current_mux = *((int *)arg);
    int next_to_print;
    int counter = 0;

    while (1)
    {
        next_to_print = counter * 5 + (current_mux + 1);

        if (next_to_print > 100)
            break;

        wait_until_value_four(current_mux);

        decrease_mux_to_zero(current_mux);

        sem_action(next_to_print, current_mux);

        increase_other_sems(current_mux);

        counter++;
    }
}

int main()
{
    for (int i = 0; i < 5; i++)
        sem_init(&mutex[i], 0, 4 - i);

    pthread_t t[5];
    int threadNum[5] = {0, 1, 2, 3, 4};

    for (int i = 0; i < 5; i++)
    {
        pthread_create(&t[i], NULL, print_with_five, &threadNum[i]);
    }

    for (int i = 0; i < 5; i++)
        pthread_join(t[i], NULL);

    for (int i = 0; i < 5; i++)
        sem_destroy(&mutex[i]);

    return 0;
}