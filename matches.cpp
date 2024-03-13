#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <vector>

// structure for message queue
struct Prices {
    int tabaco;
    int paper;
    int matches;
};

struct Product {
    long mtype;
    int price;
};

int makeOfQueue(key_t key);
sem_t *makingOfSem(const char *name);

int main() {
    // making the shared memory - prices
    key_t key_prices = ftok("prices", 64);
    int shmid_prices = shmget(key_prices, sizeof(Prices), 0777);
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
    int msgid_product = makeOfQueue(key_product);

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
    int walet = rand() % 50 + 1;

    Product product_matches, product_paper, product_tabaco;
    product_tabaco.mtype = 3;
    product_matches.mtype = 1;
    product_paper.mtype = 2;
    bool flag = false;

    printf("Matches Smoker: %d\n", walet);

    while (true) {
        // taking money from other smokers
        std::vector<int> prices;
        int temp = walet;
        sem_wait(sem_matches);
        while (msgrcv(msgid_product, &product_matches, sizeof(Product), 1, IPC_NOWAIT) != -1) {
            walet += product_matches.price;
            prices.push_back(product_paper.price);
        }
        sem_post(sem_matches);
        if (temp != walet)
            printf("walet: %d\tgot: ", walet);
        for (int i = 0; i < prices.size(); i++)
            printf("%d ", prices[i]);
        if (temp != walet)
            printf("\n");

        sem_wait(sem_prices);
        if (shm_prices->tabaco + shm_prices->paper <= walet) {
            flag = true;
            product_tabaco.price = shm_prices->tabaco;
            product_paper.price = shm_prices->paper;
            printf("\ttabaco: %d\n\tpaper: %d\n", product_tabaco.price, product_paper.price);
        }
        sem_post(sem_prices);

        if (flag) {
            flag = false;
            // printf("\nWalet: %d\n\tproduct_tabaco price: %d\n\tproduct_paper price: %d\n", walet, product_tabaco.price, product_paper.price);
            walet -= product_tabaco.price;
            walet -= product_paper.price;

            sem_wait(sem_tabaco);
            msgsnd(msgid_product, &product_tabaco, sizeof(Product), 0);
            sem_post(sem_tabaco);
            sem_wait(sem_paper);
            msgsnd(msgid_product, &product_paper, sizeof(Product), 0);
            sem_post(sem_paper);
            printf("\tSmoking...\n");
            sleep(1);
        }
    }
    return 0;
}

int makeOfQueue(key_t key) {
    // making the message queue from key
    int msgid = msgget(key, 0666);

    if (msgid == -1) {
        printf("msgget\n");
        exit(1);
    }

    return msgid;
}

sem_t *makingOfSem(const char *name) {
    sem_t *sem = sem_open(name, 0644, 1);
    if (sem == SEM_FAILED) {
        printf("sem_open\n");
        exit(1);
    }
    return sem;
}