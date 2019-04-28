/**
 *
 * ./sekerpare  .
 * CSE344 System Programming HomeWork 4 - Make Sekerpare
 * Sinan Elveren - Gebze Technical University - Computer Engineering
 */

#define _POSIX_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <semaphore.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/mman.h>



//#define NDBUG
#define IPC ( 1 )
#define PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
#define EGGS 1
#define FLOUR 2
#define BUTTER 3
#define SUGAR 4




pid_t   parentPID = 0;      //for check child pid


void chef1();
void chef2();
void chef3();
void chef4();
void chef5();
void chef6();


void wholesaler();
void generateSupplies();
void makeDesert();

pid_t myWait(int *status);
void myAtexit(void);
void signalCatcher(int signum);     // or exit funcs
void finish(int exitNum);




typedef struct sharedMemory {
    int running;            //condition
    int desertCount;        //total desertcount to DEBUG (dont need this variable)

    //un--named semaphores
    sem_t takeTry;          //mutex for check 2 supplies & use it all Chefs
    sem_t readyDesert;      //mutex for desert is ready, say to wholesaler
    sem_t eggs;             //is eggs ready
    sem_t flour;            //is flour ready
    sem_t butter;           //is buffe ready
    sem_t sugar;            //is sugar ready
    sem_t getSupplies;      //wholesaler generating 2 supplies for one chef
} SharedMem;

static SharedMem *sharedMemory;




/* * * * * * * * * * * * * * * * *START_OF_MAIN* * * * * * * * * * * * * * * * * * * * * * * * * * */

int main(int argc, char *argv[]) {


    if(atexit(myAtexit) !=0 ) {
        perror("atexit");
        return 1;
    }
    parentPID = getpid();

    int pid[5] = {0,0,0,0,0};
    int running;
    int status;
    int semCheck1 = 0,
            semCheck2 = 0,
            semCheck3 = 0,
            semCheck4 = 0,
            semCheck5 = 0,
            semCheck6 = 0;



    /**Signal**/
    struct sigaction newact;
    newact.sa_handler = signalCatcher;
    /* set the new handler */

    newact.sa_flags = 0;

    /*install sigint*/
    if ((sigemptyset(&newact.sa_mask) == -1) || (sigaction(SIGINT, &newact, NULL) == -1)){
        perror("Failed to install SIGINT signal handler");
        return EXIT_FAILURE;
    }



    //SHARED MEMORY
    sharedMemory = mmap( NULL, sizeof( SharedMem ), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,   0);

    if( MAP_FAILED == sharedMemory ) {
        perror( "mmap failed" );
        return EXIT_FAILURE;
    }

    //initial 1 for run
    sharedMemory->running = 1;

    sharedMemory->desertCount = 0;


    //Semaphore
    semCheck1 = sem_init( &sharedMemory->getSupplies, IPC, 1 );
    semCheck2 = sem_init( &sharedMemory->readyDesert, IPC, 0 );
    semCheck2 = sem_init( &sharedMemory->takeTry, IPC, 0 );
    semCheck3 = sem_init( &sharedMemory->eggs, IPC, 0 );
    semCheck4 = sem_init( &sharedMemory->flour, IPC, 0 );
    semCheck5 = sem_init( &sharedMemory->butter, IPC, 0 );
    semCheck6 = sem_init( &sharedMemory->sugar, IPC, 0 );

    if( semCheck1 == -1 || semCheck2 == -1 || semCheck3 == -1 ||
        semCheck4 == -1 || semCheck5 == -1 || semCheck6 == -1 ) {
        perror("sem_init failed");
        munmap(sharedMemory, sizeof(SharedMem));
        return EXIT_FAILURE;
    }




    //create 6 child(chefs) from parent process
    for (int i = 0; i < 6 && parentPID == getpid(); ++i) {
        pid[i] = fork();

        if( pid[i] == -1) {
            //destroy in atExit
            //ready for exit
            sharedMemory->running = 0;

            exit(EXIT_FAILURE);
        }
        else if(pid[i] == 0){
            pid[i] = getpid();

        }

    }



    if(getpid() == pid[0]) {

        chef1();
        //ready for exit
        sharedMemory->running = 0;

        exit(EXIT_FAILURE);
    }



    if(getpid() == pid[1]) {

        chef2();
        //ready for exit
        sharedMemory->running = 0;

        exit(EXIT_FAILURE);
    }


    if(getpid() == pid[2]) {

        chef3();
        //ready for exit
        sharedMemory->running = 0;

        exit(EXIT_FAILURE);
    }


    if(getpid() == pid[3]) {

        chef4();
        //ready for exit
        sharedMemory->running = 0;

        exit(EXIT_FAILURE);
    }


    if(getpid() == pid[4]) {

        chef5();
        //ready for exit
        sharedMemory->running = 0;

        exit(EXIT_FAILURE);
    }


    if(getpid() == pid[5]) {

        chef6();
        //ready for exit
        sharedMemory->running = 0;

        exit(EXIT_FAILURE);
    }



    if(getpid() == parentPID) {
        srand ( time(NULL) );

        wholesaler();

        wait(&status);
        if (WIFEXITED(status)) {

            printf("the child exited normally\n");
            printf("child's exit status: %d\n", WEXITSTATUS(status));
        } else {
            fprintf(stderr, "something went wrong in the child process\n");
        }


        //ready for exit
        sharedMemory->running = 0;

        sem_post( &sharedMemory->getSupplies );


        //free/destroy shared memory
        sem_destroy(&sharedMemory->getSupplies);
        sem_destroy(&sharedMemory->readyDesert);
        sem_destroy(&sharedMemory->takeTry);
        sem_destroy(&sharedMemory->eggs);
        sem_destroy(&sharedMemory->flour);
        sem_destroy(&sharedMemory->butter);
        sem_destroy(&sharedMemory->sugar);

        //unmap - map ' allocated memory
        munmap(sharedMemory, sizeof(SharedMem));

        fprintf(stdout,"\n--All semaphores has been destroyed succesfuly\n");
        fprintf(stdout,"--MAP has been unmaped succesfuly\n");
        fflush(stdout);

        exit(EXIT_FAILURE);
    }
}




/* * * * * * * * * * * * * * * * * * * * * * *_END_OF_MAIN* * * * * * *** * * ** * * * * * * *** ** * *** **/



pid_t myWait(int *status) {
    pid_t rtrn;

    while (((rtrn = wait(status)) == -1) && (errno == EINTR));

    return rtrn;
}




void myAtexit(void){
    //childreen is coming
    if (getpid() != parentPID) {

        fprintf(stdout,"\n    [%ld]Child Process has been exit succesfuly\n", getpid());
        fflush(stdout);
        //exit
        return;
    }

    while (myWait(NULL ) > 0);
    fprintf(stdout,"\n All child proccess is exited\n");
    fflush(stdout);

    fprintf(stdout,"\n    [%d]PARENT: I'm going to exit\n", getpid());
    fflush(stdout);
    //free/destroy shared memory
    sem_destroy(&sharedMemory->getSupplies);
    sem_destroy(&sharedMemory->readyDesert);
    sem_destroy(&sharedMemory->takeTry);
    sem_destroy(&sharedMemory->eggs);
    sem_destroy(&sharedMemory->flour);
    sem_destroy(&sharedMemory->butter);
    sem_destroy(&sharedMemory->sugar);

    //unmap - map ' allocated memory
    munmap(sharedMemory, sizeof(SharedMem));

    fprintf(stdout,"All semaphores has been destroyed succesfuly\n");
    fprintf(stdout,"MAP has been unmaped succesfuly\n");
    fflush(stdout);


    fprintf(stdout,"\n    [%d]Parent Process has been exit succesfuly\n", getpid());
    fflush(stdout);
    return;
}




void signalCatcher(int signum) {

    //ready for exit
    sharedMemory->running = 0;
    sem_post(&sharedMemory->getSupplies );
    sem_post(&sharedMemory->takeTry );


    fflush(stdout);
    if(getpid() != parentPID)
        return;


    if( msync( sharedMemory, sizeof( SharedMem ), MS_SYNC | MS_INVALIDATE ) ) {
        perror( "msync failed" );
    }

    switch (signum) {
        case SIGUSR1: puts("\ncaught SIGUSR1");
            break;
        case SIGUSR2: puts("\ncaught SIGUSR2");
            break;
        case SIGINT:
            fprintf(stderr,"\n\n[%d]SIGINT:Ctrl+C signal detected, exit (%d)\n", (long)getpid(), signum);
            finish(1);
            break;
        default:
            fprintf(stderr,"Catcher caught unexpected signal (%d)\n", signum);

            finish(1);
            break;
    }
}


// or exit funcs
void finish(int exitNum) {
    //  myAtexit();         //for wait children
    exit(exitNum);
}


void chef1() {

    int running = 1;
    int firstSupply = 0;
    int secondSupply = 0;

    while( running ) {
        fprintf(stdout, "Chef 1 is waiting for EGGS and FLOUR\n");
        fflush(stdout);
        sem_wait(&sharedMemory->takeTry);

        //check shared memory area, for take the 2 supplies
        sem_getvalue(&sharedMemory->eggs, &firstSupply);
        sem_getvalue(&sharedMemory->flour, &secondSupply);


        if (firstSupply == 1 && secondSupply == 1) {
            fflush(stdout);

            sem_post(&sharedMemory->readyDesert);

            sem_wait(&sharedMemory->eggs);
            fprintf(stdout, "Chef 1 has taken EGGS\n");

            sem_wait(&sharedMemory->flour);
            fprintf(stdout, "Chef 1 has taken FLOUR\n");

            sharedMemory->desertCount = sharedMemory->desertCount + 1;
            running = sharedMemory->running;


            fprintf(stdout, "Chef 1 is preparing %d. desert\n", sharedMemory->desertCount);

            fprintf(stdout, "Chef 1 has the delivered %d. desert to the Wholesaler\n", sharedMemory->desertCount);
            fflush(stdout);

            sem_post(&sharedMemory->getSupplies);

        } else {
            sem_post(&sharedMemory->takeTry);
            running = sharedMemory->running;
        }

    }
}




void chef2() {

    int running = 1;
    int firstSupply = 0;
    int secondSupply = 0;

    while( running ) {
        fprintf(stdout, "Chef 2 is waiting EGGS and BUTTER\n");
        fflush(stdout);
        sem_wait(&sharedMemory->takeTry);

        //check shared memory area, for take the 2 supplies
        sem_getvalue(&sharedMemory->eggs, &firstSupply);
        sem_getvalue(&sharedMemory->butter, &secondSupply);


        if (firstSupply == 1 && secondSupply == 1) {
            fflush(stdout);
            sem_post(&sharedMemory->readyDesert);

            sem_wait(&sharedMemory->eggs);
            fprintf(stdout, "Chef 2 has taken EGGS\n");

            sem_wait(&sharedMemory->butter);
            fprintf(stdout, "Chef 2 has taken BUTTER\n");

            sharedMemory->desertCount = sharedMemory->desertCount + 1;
            running = sharedMemory->running;


            fprintf(stdout, "Chef 2 is preparing %d. desert\n", sharedMemory->desertCount);

            fprintf(stdout, "Chef 2 has the delivered %d. desert to the Wholesaler\n", sharedMemory->desertCount);
            fflush(stdout);

            sem_post(&sharedMemory->getSupplies);

        } else {
            sem_post(&sharedMemory->takeTry);
            running = sharedMemory->running;
        }
    }
}




void chef3() {

    int running = 1;
    int firstSupply = 0;
    int secondSupply = 0;

    while( running ) {
        fprintf(stdout, "Chef 3 is waiting EGGS and SUGAR\n");
        fflush(stdout);
        sem_wait(&sharedMemory->takeTry);

        //check shared memory area, for take the 2 supplies
        sem_getvalue(&sharedMemory->eggs, &firstSupply);
        sem_getvalue(&sharedMemory->sugar, &secondSupply);


        if (firstSupply == 1 && secondSupply == 1) {
            fflush(stdout);
            sem_post(&sharedMemory->readyDesert);

            sem_wait(&sharedMemory->eggs);
            fprintf(stdout, "Chef 3 has taken EGGS\n");

            sem_wait(&sharedMemory->sugar);
            fprintf(stdout, "Chef 3 has taken SUGAR\n");

            sharedMemory->desertCount = sharedMemory->desertCount + 1;
            running = sharedMemory->running;


            fprintf(stdout, "Chef 3 is preparing %d. desert\n", sharedMemory->desertCount);

            fprintf(stdout, "Chef 3 has the delivered %d. desert to the Wholesaler\n", sharedMemory->desertCount);
            fflush(stdout);

            sem_post(&sharedMemory->getSupplies);

        } else {
            sem_post(&sharedMemory->takeTry);
            running = sharedMemory->running;
        }
    }
}





void chef4() {

    int running = 1;
    int firstSupply = 0;
    int secondSupply = 0;

    while( running ) {
        fprintf(stdout, "Chef 4 is waiting FLOUR and BUTTER\n");
        fflush(stdout);
        sem_wait(&sharedMemory->takeTry);

        //check shared memory area, for take the 2 supplies
        sem_getvalue(&sharedMemory->flour, &firstSupply);
        sem_getvalue(&sharedMemory->butter, &secondSupply);

        if (firstSupply == 1 && secondSupply == 1) {
            fflush(stdout);

            sem_post(&sharedMemory->readyDesert);

            sem_wait(&sharedMemory->flour);
            fprintf(stdout, "Chef 4 has taken FLOUR\n");

            sem_wait(&sharedMemory->butter);
            fprintf(stdout, "Chef 4 has taken BUTTER\n");

            sharedMemory->desertCount = sharedMemory->desertCount + 1;
            running = sharedMemory->running;


            fprintf(stdout, "Chef 4 is preparing %d. desert\n", sharedMemory->desertCount);

            fprintf(stdout, "Chef 4 has the delivered %d. desert to the Wholesaler\n", sharedMemory->desertCount);
            fflush(stdout);

            sem_post(&sharedMemory->getSupplies);

        } else {
            sem_post(&sharedMemory->takeTry);
            running = sharedMemory->running;
        }
    }
}



void chef5() {

    int running = 1;
    int firstSupply = 0;
    int secondSupply = 0;

    while( running ) {
        fprintf(stdout, "Chef 5 is waiting FLOUR and SUGAR\n");
        fflush(stdout);
        sem_wait(&sharedMemory->takeTry);

        //check shared memory area, for take the 2 supplies
        sem_getvalue(&sharedMemory->flour, &firstSupply);
        sem_getvalue(&sharedMemory->sugar, &secondSupply);


        if (firstSupply == 1 && secondSupply == 1) {
            fflush(stdout);
            sem_post(&sharedMemory->readyDesert);

            sem_wait(&sharedMemory->flour);
            fprintf(stdout, "Chef 5 has taken FLOUR\n");

            sem_wait(&sharedMemory->sugar);
            fprintf(stdout, "Chef 5 has taken SUGAR\n");

            sharedMemory->desertCount = sharedMemory->desertCount + 1;
            running = sharedMemory->running;


            fprintf(stdout, "Chef 5 is preparing %d. desert\n", sharedMemory->desertCount);

            fprintf(stdout, "Chef 5 has the delivered %d. desert to the Wholesaler\n", sharedMemory->desertCount);
            fflush(stdout);

            sem_post(&sharedMemory->getSupplies);

        } else {
            sem_post(&sharedMemory->takeTry);
            running = sharedMemory->running;
        }
    }
}



void chef6() {

    int running = 1;
    int firstSupply = 0;
    int secondSupply = 0;

    while( running ) {
        fprintf(stdout, "Chef 6 is waiting BUTTER and SUGAR\n");
        fflush(stdout);
        sem_wait(&sharedMemory->takeTry);

        //check shared memory area, for take the 2 supplies
        sem_getvalue(&sharedMemory->butter, &firstSupply);
        sem_getvalue(&sharedMemory->sugar, &secondSupply);

        if (firstSupply == 1 && secondSupply == 1) {
            fflush(stdout);
            sem_post(&sharedMemory->readyDesert);

            sem_wait(&sharedMemory->butter);
            fprintf(stdout, "Chef 6 has taken BUTTER\n");

            sem_wait(&sharedMemory->sugar);
            fprintf(stdout, "Chef 6 has taken SUGAR\n");

            sharedMemory->desertCount = sharedMemory->desertCount + 1;
            running = sharedMemory->running;


            fprintf(stdout, "Chef 6 is preparing %d. desert\n", sharedMemory->desertCount);

            fprintf(stdout, "Chef 6 has the delivered %d. desert to the Wholesaler\n", sharedMemory->desertCount);
            fflush(stdout);

            sem_post(&sharedMemory->getSupplies);

        } else {
            sem_post(&sharedMemory->takeTry);
            running = sharedMemory->running;
        }
    }
}




void wholesaler(){
    int i = 0;


    //wait for generate and get 2 supplies
    sem_wait(&sharedMemory->getSupplies);

    // triggers SignalHandlerFunc (you'll need to send it in the terminal)
    int running = 1;
    while (running) {
        //generate and post 2 supplies
        generateSupplies();
        //delivers to

        //push takeTry for  in order chefs can access
        sem_post(&sharedMemory->takeTry);

        //chef got supplies succesfuly
        sem_wait(&sharedMemory->readyDesert);

        fprintf(stdout, "Wholesaler waiting desert\n");

        //wait for the chec is finish desert?
        running = sharedMemory->running;
        sem_wait(&sharedMemory->getSupplies);

        fprintf(stdout, "wholesaler has obtained the %d.dessert and left to sell it\n\n",  sharedMemory->desertCount);
        fflush(stdout);
    }
}


void generateSupplies() {
    int random1 = 0;
    int random2 = 0;
    int temp = 0;
    random1 = (rand() % 4) +1;
    random2 = (rand() % 4) +1;


    while (random2 == random1){
        random2 = (rand() % 4) +1;
    }


    if(random1 > random2){      //Swap
        temp = random1;
        random1 = random2;
        random2 = temp;
    }

    if( random1 == EGGS && random2 == FLOUR){
        //push 2 supply
        sem_post(&sharedMemory->eggs);
        sem_post(&sharedMemory->flour);


        fprintf(stdout, "wholesaler delivers EGGS and FLOUR\n");
        fflush(stdout);
    }
    else if( random1 == EGGS && random2 == BUTTER){
        //push 2 supply
        sem_post(&sharedMemory->eggs);
        sem_post(&sharedMemory->butter);


        fprintf(stdout, "wholesaler delivers EGGS and BUTTER\n");
        fflush(stdout);
    }
    else if( random1 == EGGS && random2 == SUGAR){
        //push 2 supply
        sem_post(&sharedMemory->eggs);
        sem_post(&sharedMemory->sugar);


        fprintf(stdout, "wholesaler delivers EGGS and SUGAR\n");
        fflush(stdout);
    }
    else if( random1 == FLOUR && random2 == BUTTER){
        //push 2 supply
        sem_post(&sharedMemory->flour);
        sem_post(&sharedMemory->butter);


        fprintf(stdout, "wholesaler delivers FLOUR and BUTTER\n");
        fflush(stdout);
    }
    else if( random1 == FLOUR && random2 == SUGAR){
        //push 2 supply
        sem_post(&sharedMemory->flour);
        sem_post(&sharedMemory->sugar);


        fprintf(stdout, "wholesaler delivers FLOUR and SUGAR\n");
        fflush(stdout);
    }
    else if( random1 == BUTTER && random2 == SUGAR){
        //push 2 supply
        sem_post(&sharedMemory->butter);
        sem_post(&sharedMemory->sugar);


        fprintf(stdout, "wholesaler delivers BUTTER and SUGAR\n");
        fflush(stdout);
    }

}