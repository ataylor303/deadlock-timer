/*
Name: Asiah T and Ben P
Course: <Course Name>
Assignment: Deadlock Timer Simulation
Date: March 1, 2026

This program simulates a deadlock situation using two shared resources.
Each process tries to acquire the resources in a different order.
A timer is used when requesting the second resource.

If the second resource is not obtained in time:
    > the process releases its first resource
    > logs the failure
    > exits

The parent process restarts timed-out processes.
After too many retries, the process is marked as starved.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#define RESOURCE_A "/resA"
#define RESOURCE_B "/resB"
#define LOG_LOCK   "/logLock"

#define TIMEOUT 2
#define MAX_RETRIES 5

sem_t *A;
sem_t *B;
sem_t *logLock;

FILE *logFile;

void writeLog(const char *msg)
{
    sem_wait(logLock);
    fprintf(logFile, "%s\n", msg);
    fflush(logFile);
    sem_post(logLock);
}

int tryTimedLock(sem_t *sem)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += TIMEOUT;

    if (sem_timedwait(sem, &ts) == -1)
        return 0;

    return 1;
}

void worker(int id)
{
    A = sem_open(RESOURCE_A, 0);
    B = sem_open(RESOURCE_B, 0);
    logLock = sem_open(LOG_LOCK, 0);

    logFile = fopen("activity_log.txt", "a");

    char msg[200];
    sem_t *first;
    sem_t *second;

    if (id == 0) {
        first = A;
        second = B;
        sprintf(msg, "Process %d trying A then B", id);
    } else {
        first = B;
        second = A;
        sprintf(msg, "Process %d trying B then A", id);
    }

    writeLog(msg);

    sem_wait(first);
    sprintf(msg, "Process %d acquired first resource", id);
    writeLog(msg);

    sleep(1); // increases chance of deadlock

    sprintf(msg, "Process %d attempting second resource...", id);
    writeLog(msg);

    if (!tryTimedLock(second)) {
        sprintf(msg, "Process %d timed out waiting -> releasing first resource", id);
        writeLog(msg);
        sem_post(first);
        exit(2);
    }

    sprintf(msg, "Process %d entered critical section", id);
    writeLog(msg);

    sleep(1);

    sem_post(second);
    sem_post(first);

    sprintf(msg, "Process %d finished successfully", id);
    writeLog(msg);

    exit(0);
}

int main()
{
    sem_unlink(RESOURCE_A);
    sem_unlink(RESOURCE_B);
    sem_unlink(LOG_LOCK);

    A = sem_open(RESOURCE_A, O_CREAT, 0644, 1);
    B = sem_open(RESOURCE_B, O_CREAT, 0644, 1);
    logLock = sem_open(LOG_LOCK, O_CREAT, 0644, 1);

    logFile = fopen("activity_log.txt", "w");
    fprintf(logFile, "Deadlock Simulation Log\n\n");
    fclose(logFile);

    int retries[2] = {0,0};
    pid_t pids[2];

    for (int i = 0; i < 2; i++) {
        if ((pids[i] = fork()) == 0)
            worker(i);
    }

    while (1) {
        int status;
        pid_t finished = wait(&status);

        int id = (finished == pids[0]) ? 0 : 1;

        if (WEXITSTATUS(status) == 2) {
            retries[id]++;

            logFile = fopen("activity_log.txt", "a");

            if (retries[id] > MAX_RETRIES) {
                fprintf(logFile, "Process %d marked as STARVED\n", id);
                fclose(logFile);
                break;
            }

            fprintf(logFile, "Restarting Process %d\n", id);
            fclose(logFile);

            if ((pids[id] = fork()) == 0)
                worker(id);
        } else {
            break;
        }
    }

    sem_unlink(RESOURCE_A);
    sem_unlink(RESOURCE_B);
    sem_unlink(LOG_LOCK);

    return 0;
}