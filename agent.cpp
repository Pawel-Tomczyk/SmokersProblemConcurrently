#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/shm.h>

// structure for message queue
struct Prices {
    int tabaco;
    int matches;
    int paper;
};

struct Product {
    long mtype;
    int price;
};

// Function to make the message queue(if it doesn't exist or makes it empty if it does exist)
// returns the id of the message queue
int makeOfQueue(key_t key);
sem_t *makingOfSem(const char *name);

void determining_the_stock_exchange_price(Prices *p);

int main() {
    // making the shared memory - prices
    key_t key_prices = ftok("prices", 64);
    int shmid_prices = shmget(key_prices, sizeof(Prices), 0777 | IPC_CREAT);
    if (shmid_prices == -1) {
        printf("shmget\n");
        exit(1);
    }
    Prices *shm_prices = (Prices *)shmat(shmid_prices, NULL, 0);
    // making the semaphore for shared memory - prices
    const char *name = "sem_prices";
    sem_t *sem_prices = makingOfSem(name);

    // making the message queues with semaphores
    // making queue
    key_t key_product = ftok("product", 65);
    int msgid_product = makeOfQueue(key_product);//1 -  matches 2 - paper 3 - tabaco

    // tabaco semaphore
    name = "sem_tabaco";
    sem_t *sem_tabaco = makingOfSem(name);

    // matches semaphore
    name = "sem_matches";
    sem_t *sem_matches = makingOfSem(name);

    // paper semaphore
    name = "sem_paper";
    sem_t *sem_paper = makingOfSem(name);

    srand(time(NULL));
    // BASECLY EVERYTHING WHAT AGENT DOES
    while (true) {
        sem_wait(sem_prices);//0 - wait; 1 - go sem--
        determining_the_stock_exchange_price(shm_prices);
        sem_post(sem_prices);//sem++
        printf("Prices: \n");
        printf("\tTabaco: %d\n\tPaper: %d\n\tMatches: %d\n\n", shm_prices->tabaco, shm_prices->matches, shm_prices->paper);
        sleep(1);
    }

    return 0;
}

int makeOfQueue(key_t key) {
    // making the message queue from key
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        printf("msgget\n");
        exit(1);
    }

    // if the message queue already exists, delete everything from it
    Product product;
    while (msgrcv(msgid, &product, sizeof(Product), 3, IPC_NOWAIT) != -1) {
        // Making it empty
    }
    while (msgrcv(msgid, &product, sizeof(Product), 1, IPC_NOWAIT) != -1) {
        // Making it empty
    }
    while (msgrcv(msgid, &product, sizeof(Product), 2, IPC_NOWAIT) != -1) {
        // Making it empty
    }
    return msgid;
}

sem_t *makingOfSem(const char *name) {
    sem_t *sem = sem_open(name, O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        printf("sem_open\n");
        exit(1);
    }
    int value;
    if (sem_getvalue(sem, &value) == -1) {
        printf("sem_getvalue\n");
        exit(1);
    }


    if (value != 1) {
        while (value > 1) {
            sem_wait(sem);
            value--;
        }
        while (value < 1) {
            sem_post(sem);
            value++;
        }
    }
    
    return sem;
}

void determining_the_stock_exchange_price(Prices *p) {
    p->tabaco = rand() % 10 + 1;
    p->paper = rand() % 10 + 1;
    p->matches = rand() % 10 + 1;
}