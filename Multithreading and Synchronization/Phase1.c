#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

#define MAXTHREAD 10
pthread_t threadID[MAXTHREAD + 1];
sem_t *mutex;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
sem_t *Recieve[MAXTHREAD];   // Consumer semaphores for each thread

sem_t *Send[MAXTHREAD];      // Producer semaphores for threads

int args[MAXTHREAD];  //Thread Ids to pass in addr fn
struct msg {
    int iFrom;      //Thread Id of who sent the message
    int value;      //Value of message
    int cnt;        //Count of opeations
    int tot;        // Total time
};
struct Mailbox {
    int ID;
    struct msg *message;
};

void CopyMsg(struct msg *src, struct msg *dest);

void PrintMessage(struct msg *Summary);

int RecvMsg(int iRecv, struct msg *message);

void SendMsg(int iTo, struct msg *message);

void initMailBox(int Threads);

void freeSems(int thr);

int Input(int *IDs, int *Values);

int SendInputMessages(int *IDs, int *Values, int lines);

void Terminate(int thr);


struct Mailbox Mails[MAXTHREAD];


// Thread start routine
// Each runs thread runs within this function
void *adder(void *arg) {
    time_t Start;
    time_t End;
    int Index = *((int *) arg);

    struct msg *temp = (struct msg *) malloc(sizeof(struct msg));
    struct msg *Summary = (struct msg *) malloc(sizeof(struct msg));
    // Lock for calling thread so no other thread can run after this point until unlocked
    time(&Start);  // Start timer for thread run
    int count = 0;
    while (1) {
        if (RecvMsg(Index, temp)) {
            pthread_mutex_lock(&m);

          // RecvMsg from mailbox if any, copies and sleeps for 1 sec
        // sends summary and exits after unlocking mutex it lock previously

        if (temp->value > 0) {
            Summary->cnt = ++count;
            Summary->iFrom = Index;
            Summary->value = temp->value + Summary->value;
            pthread_mutex_unlock(&m);
            sleep(1);

        } else if (temp->value == -1) {
            time(&End);
            Summary->tot = (int) (End - Start);
            SendMsg(0, Summary);
            free(Summary);
            pthread_mutex_unlock(&m);
            pthread_exit(NULL);
        }}
    }
}

int main(int argc, char *argv[]) {
    int Threads;
    int *IDs = malloc(sizeof(int) * 100);
    int *Values = malloc(sizeof(int) * 100);
    struct msg *printMsg = (struct msg *) malloc(sizeof(struct msg));

    if (argc == 2) {
        Threads = atoi(argv[1]);
        if (Threads > 10) {
            Threads = MAXTHREAD;
        }
    } else Threads = MAXTHREAD;
    int lines = Input(IDs, Values);

    initMailBox(Threads);

    // Set main thread as thread 0
    threadID[0] = pthread_self();

    // creating child threads and starting the routine adder with the loop index representing Thread ID
    // and being sent to thread. NOTE: this is simply the loop index being associated with the order the thread was created
    // and is different from the value recieved pthread_self() routine.
    for (int i = 1; i < Threads + 1; i++) {
        args[i] = i;
        if (pthread_create(&threadID[i], NULL, adder, &args[i]) != 0) {
            perror("pthread_create");
            exit(1);
        }
        // printf("Thread created\n");
    }
    // Once all threads are created Main Thread(Thread 0) sending stdin messages
    // After which on EOF detection sends the termination message to each thread
    SendInputMessages(IDs, Values, lines);
    Terminate(Threads);

    // Main thread recieveing message from threads after they complete and send thier summary message
    // Prints Value, time thread run and operations it computed
    // Value doesn't take into account the termination value -1

    for (int t = 0; t < Threads; t++) {
        RecvMsg(0, printMsg);
        PrintMessage(printMsg);
    }

    for (int j = 1; j < Threads + 1; j++) {
        pthread_join(threadID[j], NULL);
    }
    // freeing allocated memory before main completes
    free(printMsg);
    freeSems(Threads);
    free(IDs);
    free(Values);
    for (int f = 0; f <= Threads; f++) {
        free(Mails[f].message);
    }
    return 0;
}

// initial Mailboxes for number of threads + 1(main thread)
// Semaphohres also initialized for each Mailbox
// Producer(Send) and consumer(Recieve) semaphores created for each Mailbox
void initMailBox(int threads) {
    for (int i = 0; i < threads + 1; i++) {
        Mails[i].message = (struct msg *) malloc(sizeof(struct msg));
        Mails[i].ID = i;
        sem_t new, new2;
        Send[i] = (sem_t *) malloc(sizeof(sem_t));
        Recieve[i] = (sem_t *) malloc(sizeof(sem_t));
        *(Send[i]) = new;
        *(Recieve[i]) = new2;
        sem_init(Send[i], 0, 1);
        sem_init(Recieve[i], 0, 0);
    }
}

// Sends Termination message to child threads
void Terminate(int thr) {
    struct msg *Termination;
    Termination = (struct msg *) malloc(sizeof(struct msg));
    Termination->iFrom = 0;
    Termination->value = -1;
    Termination->tot = 0;
    Termination->cnt = 0;
    for (int i = 1; i < thr + 1; i++) {
        SendMsg(i, Termination);
    }
    free(Termination);
}
// Prints final results summary
// The argument Summary is the message sent by child threads
// Outputs printed in main thread after it calls RecvMsg()
// Only Prints when there has been a message other than termination for that thread

void PrintMessage(struct msg *Summary) {
    if (Summary->value > 0) {
        printf("The result from thread %d is %d from %d operations during %d secs.\n", Summary->iFrom, Summary->value,
               Summary->cnt,
               Summary->tot);
    }
}

// Gets the IDs and Values recvieved from stdin within Input()
// Sends messages to respective mailboxs
// Main Thread will be making the call and sending the message
int SendInputMessages(int *IDs, int *Values, int lines) {
    int count = 0;
    struct msg temp;
    temp.cnt = 0;
    temp.iFrom = 0;
    while (lines > 0) {
        count++;
        if ((*IDs > 0 && *Values > 0)) {
            temp.value = *Values;
            SendMsg(*IDs, &temp);

        } else {
            printf("Message contained error value\n");
        }
        Values++;
        IDs++;
        lines--;
    }
    return 1;// Successful
}

// Input takes the stdin Inputs in the form of IDs and MessageValues
// Input prompt terminates when EOF is reached
// MAX 100 values for IDs and Values
int Input(int *IDs, int *Values) {
    char Inputs[200];
    int size = 0;
    int lines;
    printf("-------------Welcome-------------\n");
    printf("Please input ID and Value you would like to send to\n");
    printf("Value     ID\n");
    while (fgets(Inputs, 200, stdin) != NULL) {
        printf("");
        sscanf(Inputs, "%d  %d", Values, IDs);
        Values++;
        size++;
        IDs++;
    }
    lines = size;
    while (size > 0) { // Setting Values and IDs to point to first inputs
        Values--;
        IDs--;
        size--;
    }
    return lines;
}

// RecvMsg function to receive messages from Mailbox
// params: message is struct msg that will recveive message info from one recvieved
//       : iRecv is the Mailbox ID to recieve from
int RecvMsg(int iRecv, struct msg *message) {
    int semVal;
    sem_getvalue(Recieve[iRecv], &semVal);
    if (sem_wait(Recieve[iRecv]) == 0) {
        CopyMsg(Mails[iRecv].message, message);
        sem_post(Send[iRecv]);
        return 1;
    } else if (semVal == 0) {
        printf("Return when blocked\n");
        return semVal;
    }
}

// SendMsg function to Send message to Mailbox
// params: message represents the message with info to be sent
//       : iTo repsents Mailbox ID to send to
void SendMsg(int iTo, struct msg *message) {
    //Mailbox to send to and message to be sent
    sem_wait(Send[iTo]);
    CopyMsg(message, Mails[iTo].message);
    sem_post(Recieve[iTo]);
}

// Copy struct msg from src to dest
void CopyMsg(struct msg *src, struct msg *dest) {
    dest->iFrom = src->iFrom;
    dest->value = src->value;
    dest->cnt = src->cnt;
    dest->tot = src->tot;
}

void freeSems(int thr) {
    thr++;
    for (int i = 0; i < thr; i++) {
        free(Send[i]);
        free(Recieve[i]);
    }
}

