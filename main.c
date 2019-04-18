/**
 * @file main.c
 * @author Andrej Klocok (xkloco00@stud.fit.vutbr.cz)
 * @brief Simple ticket algorithm
 * @version 1.0
 * @date 2019-04-18
 */
#define _GNU_SOURCE
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED 1 /* XPG 4.2 - needed for WCOREDUMP() */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

//static initialization
pthread_mutex_t mutex_getTicket =   PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_await =       PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_await =         PTHREAD_COND_INITIALIZER;

//ticket golobal variables
volatile int ticketNumb     = 0;
volatile int actualTicket   = 0;

//thread arguments
typedef struct
{
    int M;
    int id;
} Arguments;

/**
 * @brief The output value of this function is a unique ticket number that determines the order of entry to the critical section.
 * The first received ticket is 0, the next 1, 2, etc.
 * @return int 
 */
int getticket(void){
    int ticket;

    pthread_mutex_lock(&mutex_getTicket);
    ticket = ticketNumb;
    ticketNumb++;
    pthread_mutex_unlock(&mutex_getTicket);
    return ticket;
}
/**
 * @brief Enter the critical section, where the aenter parameter is the assigned ticket number from getticket() function.
 * At the beginning of the program, only a thread with ticket 0 is permitted and only one thread can be in critical section at the same time.
 * 
 * @param aenter 
 */
void await(int aenter){
    pthread_mutex_lock(&mutex_await);

    while(aenter != actualTicket){
        pthread_cond_wait(&cond_await, &mutex_await);
    }
}
/**
 * @brief Output from the critical section, allowing another thread to be passed through the await() function with a ticket one higher
 * than the thread just leaving a critical section.
 */
void advance(void){
    actualTicket ++;
    pthread_cond_broadcast(&cond_await);
    pthread_mutex_unlock(&mutex_await);
}
/**
 * @brief Help function
 * 
 */
void printHelp(){
    printf("./ticket N M\n");
    printf("kde:\n");
    printf("\t N (int)-> pocet vlaken, ktore sa maju vytvorit\n");
    printf("\t M (int)-> celkovy pocet priechodov kritickou sekciou\n");
}

/**
 * @brief Thread worker function
 * 
 * @param arg 
 * @return void* 
 */
void *thread(void *arg)
{
	int ticket = 0;                                 //local var ticket
    Arguments* arguments = (Arguments*) arg;        //get thread arguments
    struct timespec ts1;                            //time structure

    unsigned int seed = time(NULL)^arguments->id^getpid();  //init seed from actual time xor id xor pid
    ts1.tv_sec = 0;     //initialize seconds


    /* Ticket assignment */
    while ((ticket = getticket()) < arguments->M) { 
        ts1.tv_nsec = rand_r(&seed) % 500000000+1;
        nanosleep(&ts1, NULL);                          /* Random waiting <0,0 s, 0,5 s> */
        await(ticket);                                  /* Enrty to KS */
        printf("%d\t(%u)\n", ticket, arguments->id);    /* Print*/
        fflush(stdout);
        advance();                                      /* Return from KS */
        ts1.tv_nsec = rand_r(&seed) % 500000000+1;
        nanosleep(&ts1, NULL);                          /* Random waiting <0,0 s, 0,5 s> */
    } 
	return (void *)0;
}

int main(int argc, char **argv)
{
    int threadsNumb = 0;
    int critSecNumb = 0;

    if(argc != 3){
        fprintf(stderr, "Zlý formát argumentov\n");
        printHelp();
        return 0;
    }

    threadsNumb = atoi(argv[1]);
    critSecNumb = atoi(argv[2]);
    if(threadsNumb == 0 || critSecNumb == 0){
        fprintf(stderr, "Program vyzaduje celocislene argumenty (nenulove)\n");
        printHelp();
        return 0;
    }
    pthread_t* thread_id = (pthread_t*) malloc(sizeof(pthread_t) * threadsNumb);
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); 
    
    Arguments arguments[threadsNumb];

    for(int i=0; i < threadsNumb; i++){
        arguments[i].M = critSecNumb;
        arguments[i].id = i;
        pthread_create( &thread_id[i], &attr, thread, &arguments[i] );
    }

   for(int i=0; i < threadsNumb; i++){
      pthread_join( thread_id[i], NULL);
   }
   

    free(thread_id);
	return(0);
}