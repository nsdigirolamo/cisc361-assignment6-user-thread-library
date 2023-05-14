#include "ud_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TIME_TO_EAT 3
#define PHILOSOPHERS 5
static sem_t *forks[PHILOSOPHERS];
int eaten = 0;

sem_t *lock;
int forkInUse[PHILOSOPHERS];

void philosopher(int id)
{
        int right = id;
        int left = (id+1)%PHILOSOPHERS;

        printf("Philosopher %d is hungry\n", id);
        if (id % 2 == 0)
        {
                sem_wait(forks[right]);
                sem_wait(lock);
                if (forkInUse[right])
                {
                        printf("ERROR: fork %d is being used\n", right);
                        exit(0);
                }
                forkInUse[right] = 1;
                sem_signal(lock);
                printf("Philosopher %d has picked up right fork %d\n", id, id);

                sem_wait(forks[left]);
                sem_wait(lock);
                if (forkInUse[left])
                {
                        printf("ERROR: fork %d is being used\n", left);
                        exit(0);
                }
                forkInUse[left] = 1;
                sem_signal(lock);
                printf("Philosopher %d has picked up left  fork %d\n", id, left);
        }
        else
        {
                sem_wait(forks[(id+1)%PHILOSOPHERS]);
                printf("Philosopher %d has picked up left  fork %d\n", id, (id+1)%PHILOSOPHERS);
                sem_wait(forks[id]);
                printf("Philosopher %d has picked up right fork %d\n", id, id);
        }

        int i = 0;
        for (; i < TIME_TO_EAT; i++)
        {
                t_yield();
                printf("Philosopher %d is still eating\n", id);
        }


        sem_signal(forks[id]);
        sem_signal(forks[(id+1)%PHILOSOPHERS]);

        printf("Philosopher %d has eaten\n", id);
        sem_wait(lock);

        forkInUse[left] = 0;
        forkInUse[right] = 0;
        eaten++;
        sem_signal(lock);

        t_terminate();
}

int main()
{
        int i;
        srand(time(NULL));

        t_init();
        sem_init(&lock, 1);

        //  Create a semaphore for each fork
        for (i = 0; i < PHILOSOPHERS; i++)
        {
                sem_init(&forks[i], 1);
        }

        //  Randomize the order the philosophers are created in
        int order[PHILOSOPHERS];
        for (i = 0; i < PHILOSOPHERS; i++)
                order[i] = i;

        for (i = 0; i < 20; i++)
        {
                int s1 = rand() % PHILOSOPHERS;
                int s2 = rand() % PHILOSOPHERS;
                int tmp = order[s2];
                order[s2] = order[s1];
                order[s1] = tmp;
        }

        for (i = 0; i < PHILOSOPHERS; i++)
        {
                t_create(philosopher, order[i], 1);
        }

        //  Wait for all philosophers to eat
        while (eaten != PHILOSOPHERS)
                t_yield();

        //  Clean up all data
        for (i = 0; i < PHILOSOPHERS; i++)
        {
                sem_destroy(&forks[i]);
        }
        sem_destroy(&lock);
        t_shutdown();
}
