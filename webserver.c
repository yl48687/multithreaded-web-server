#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include "webserver.h"
#define MAX_REQUEST 100

int port, numThread, crashRate;

int request[MAX_REQUEST];
int request_count = 0;

// Semaphores and mutex lock
sem_t sem_full;
sem_t sem_empty;
pthread_mutex_t mutex;

void *worker(void *arg) {
    while (1) {
        // Simulate crash based on the provided crash rate
        if (rand() % 100 < crashRate) {
            pthread_exit(NULL);
        } // if

        // Wait for a request to become available in the buffer
        sem_wait(&sem_full);
        pthread_mutex_lock(&mutex);

        // Get a request from the buffer
        int fd = request[--request_count];

        pthread_mutex_unlock(&mutex);
        sem_post(&sem_empty);

        // Process the request
        process(fd);
    } // while
} // worker

void *listener(void *arg) {
    struct sockaddr_in sin;
    int sock;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket.");
        exit(EXIT_FAILURE);
    } // if

    // Bind socket
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("Error binding socket.");
        exit(EXIT_FAILURE);
    } // if

    // Listen for incoming connections
    if (listen(sock, 5) < 0) {
        perror("Error listening on socket.");
        exit(EXIT_FAILURE);
    } // if

    printf("HTTP server listening on port %d\n", port);

    while (1) {
        int s;
        s = accept(sock, NULL, NULL);
        if (s < 0) break;

        sem_wait(&sem_empty);
        pthread_mutex_lock(&mutex);

        // Put the request into the buffer
        request[request_count++] = s;

        pthread_mutex_unlock(&mutex);
        sem_post(&sem_full);
    } // while

    close(sock);
} // listener

int main(int argc, char *argv[]) {
    // Default values
    int default_num_threads = 10;
    int default_crash_rate = 0;

    // Parse command-line arguments
    if (argc < 2 || argc > 4 || atoi(argv[1]) < 2000 || atoi(argv[1]) > 49999) {
        fprintf(stderr, "Usage: ./webserver PORT(2001 ~ 49999) (#_of_threads) (crash_rate(%))\n");
        return EXIT_FAILURE;
    } // if

    port = atoi(argv[1]);
    
    // Set number of threads
    if (argc >= 3) {
        numThread = atoi(argv[2]);
        if (numThread < 1) {
            numThread = 1;
        } else if (numThread > 10) {
            numThread = 10;
        } // if
    } else {
        numThread = default_num_threads;
    } // if

    // Set crash rate
    if (argc >= 4) {
        crashRate = atoi(argv[3]);
        if (crashRate < 0) {
            crashRate = 0;
        } else if (crashRate > 50) {
            crashRate = 50;
        }
    } else {
        crashRate = default_crash_rate;
    } // if

    // Initialize semaphores and mutex
    sem_init(&sem_empty, 0, MAX_REQUEST);
    sem_init(&sem_full, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    printf("CRASH RATE = %d%%\n", crashRate);

    pthread_t listener_thread;
    pthread_t worker_threads[numThread];

    // Create listener thread
    if (pthread_create(&listener_thread, NULL, listener, NULL) != 0) {
        perror("Error creating listener thread");
        return EXIT_FAILURE;
    } // if

    // Create worker threads
    for (int i = 0; i < numThread; i++) {
        if (pthread_create(&worker_threads[i], NULL, worker, NULL) != 0) {
            perror("Error creating worker thread");
            return EXIT_FAILURE;
        } // if
    } // for

    // Join listener thread
    if (pthread_join(listener_thread, NULL) != 0) {
        perror("Error joining listener thread");
        return EXIT_FAILURE;
    } // if

    // Join worker threads (with crash handling)
    for (int i = 0; i < numThread; i++) {
        if (pthread_tryjoin_np(worker_threads[i], NULL) != 0) {
            // Worker thread has crashed, create a new thread to replace it
            if (pthread_create(&worker_threads[i], NULL, worker, NULL) != 0) {
                perror("Error creating worker thread");
                return EXIT_FAILURE;
            } // if
        } // if
    } // for

    return EXIT_SUCCESS;
} // main