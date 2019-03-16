#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED 1 /* XPG 4.2 - needed for WCOREDUMP() */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

pthread_mutex_t mutex_getTicket =   PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_await =       PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_await =         PTHREAD_COND_INITIALIZER;

volatile int ticketNumb     = 0;
volatile int actualTicket   = 0;
/**
 * @brief Výstupní hodnotou této funkce je unikátní číslo lístku, který určuje pořadí vstupu do kritické sekce. 
 * První získaný lístek má hodnotu 0, další 1, 2, atd.
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
 * @brief Vstup do kritické sekce, kde parametr aenter je číslo přiděleného lístku funkcí getticket(). 
 *      Na počátku programu je vstup umožněn jen vláknu s lístkem 0. V kritické sekci může být v daném okamžiku maximálně jedno vlákno. 
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
 * @brief Výstup z kritické sekce, což umožní vstup jinému vláknu přes funkci await() s lístkem o jedničku vyšším,
 *      než mělo vlákno kritickou sekci právě opouštějící.
 */
void advance(void){
    actualTicket ++;
    pthread_cond_broadcast(&cond_await);
    pthread_mutex_unlock(&mutex_await);
}
/**
 * @brief 
 * 
 */
void printHelp(){
    printf("./ticket N M\n");
    printf("kde:\n");
    printf("\t\t N (int)-> pocet vlaken, ktore sa maju vytvorit\n");
    printf("\t\t M (int)-> celkovy pocet priechodov kritickou sekciou\n");
}

/**
 * @brief 
 * 
 * @param arg 
 * @return void* 
 */
void *thread(void *arg)
{
	int ticket = 0;
    int M = *(int *) arg;
    unsigned int id = (unsigned int)pthread_self();
    struct timespec ts1,ts2;
    unsigned int seed = id;
    
    ts1.tv_sec = 0;
    ts1.tv_nsec = rand_r(&seed) % 500000000+1;

    ts2.tv_sec = 0;
    ts2.tv_nsec = rand_r(&seed) % 500000000+1;

    /* Přidělení lístku */
    while ((ticket = getticket()) < M) { 
        nanosleep(&ts1, NULL);              /* Náhodné čekání v intervalu <0,0 s, 0,5 s> */
        await(ticket);                      /* Vstup do KS */
        printf("%d\t(%u)\n", ticket, id);   /* fflush(stdout); */
        advance();                          /* Výstup z KS */
        nanosleep(&ts2, NULL);              /* Náhodné čekání v intervalu <0,0 s, 0,5 s> */
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

    for(int i=0; i < threadsNumb; i++){
        pthread_create( &thread_id[i], &attr, thread, &critSecNumb );
    }

   for(int i=0; i < threadsNumb; i++){
      pthread_join( thread_id[i], NULL);
   }
   

    free(thread_id);
	return(0);
}