#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>

#define NUM_CHILDREN 4
#define SEM_NAME "/sync_semaphore"

pid_t children[NUM_CHILDREN];
sem_t *sem;

void handle_sigusr1(int sig) {
}


void handle_sigusr2(int sig) {

}

void child_task(int child_num) {
    signal(SIGUSR1, handle_sigusr1);


    printf("Enfant %d attend le signal de départ.\n", child_num);
    pause();
    sem_wait(sem);

    printf("Enfant %d commence sa tâche...\n", child_num);
    sleep(2);

    sem_post(sem);

    printf("Enfant %d a terminé sa tâche et envoie une confirmation.\n", child_num);
    kill(getppid(), SIGUSR2);

    exit(0);
}

void parent_task() {
    signal(SIGUSR2, handle_sigusr2);
    sem = sem_open(SEM_NAME, O_CREAT, 0644, NUM_CHILDREN);

    // Crée les processus enfants
    for (int i = 0; i < NUM_CHILDREN; i++) {
        children[i] = fork();
        if (children[i] == 0) {
            // Processus enfant
            child_task(i);
        }
    }

    // Envoie le signal de départ à tous les enfants
    for (int i = 0; i < NUM_CHILDREN; i++) {
        printf("Parent envoie le signal de départ à l'enfant %d.\n", i);
        kill(children[i], SIGUSR1);
    }

    // Attend les signaux de confirmation des enfants
    int confirmed = 0;
    while (confirmed < NUM_CHILDREN) {
        pause();  // Attend un signal SIGUSR2
        confirmed++;
        printf("Parent a reçu une confirmation de l'enfant %d.\n", confirmed);
    }

    // Nettoie le sémaphore
    sem_close(sem);
    sem_unlink(SEM_NAME);

    // Attend que tous les enfants terminent
    for (int i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL);
    }

    printf("Toutes les tâches sont terminées. Le parent quitte.\n");
}

int main() {
    parent_task();
    return 0;
}
