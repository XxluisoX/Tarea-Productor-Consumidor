#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCTORES 2
#define NUM_CONSUMIDORES 2
#define ITEMS_POR_PRODUCTOR 6
#define ITEMS_POR_CONSUMIDOR 6

int buffer[BUFFER_SIZE];
int count = 0;
int in = 0;
int out = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t no_lleno = PTHREAD_COND_INITIALIZER;
pthread_cond_t no_vacio = PTHREAD_COND_INITIALIZER;

void *productor(void *arg) {
    int id = *(int *)arg;
    for (int i = 1; i <= ITEMS_POR_PRODUCTOR; i++) {
        int item = id * 100 + i;

        pthread_mutex_lock(&mutex);
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&no_lleno, &mutex);
        }

        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        printf("Productor %d produjo: %d (Buffer: %d/%d)\n", id, item, count, BUFFER_SIZE);

        pthread_cond_broadcast(&no_vacio);
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
    return NULL;
}

void *consumidor(void *arg) {
    int id = *(int *)arg;
    for (int i = 1; i <= ITEMS_POR_CONSUMIDOR; i++) {
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            pthread_cond_wait(&no_vacio, &mutex);
        }

        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        printf("Consumidor %d consumio: %d (Buffer: %d/%d)\n", id, item, count, BUFFER_SIZE);

        pthread_cond_broadcast(&no_lleno);
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t hilos_prod[NUM_PRODUCTORES];
    pthread_t hilos_cons[NUM_CONSUMIDORES];
    int ids_prod[NUM_PRODUCTORES];
    int ids_cons[NUM_CONSUMIDORES];

    for (int i = 0; i < NUM_PRODUCTORES; i++) {
        ids_prod[i] = i + 1;
        if (pthread_create(&hilos_prod[i], NULL, productor, &ids_prod[i]) != 0) {
            fprintf(stderr, "Error al crear productor %d.\n", i + 1);
            return 1;
        }
    }

    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        ids_cons[i] = i + 1;
        if (pthread_create(&hilos_cons[i], NULL, consumidor, &ids_cons[i]) != 0) {
            fprintf(stderr, "Error al crear consumidor %d.\n", i + 1);
            return 1;
        }
    }

    for (int i = 0; i < NUM_PRODUCTORES; i++) {
        pthread_join(hilos_prod[i], NULL);
    }

    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        pthread_join(hilos_cons[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&no_lleno);
    pthread_cond_destroy(&no_vacio);

    return 0;
}
