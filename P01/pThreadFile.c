/*
 * @file pThreadFile.c
 * 
 * @description C Program to create implement pThreads
 *
 * @author Madhumitha Tolakanahalli Pradeep
 * @date 24 February 2019
 *
 * @citation https://www3.physnet.uni-hamburg.de/physnet/Tru64-Unix/HTML/APS33DTE/DOCU_007.HTML
 *			 https://www.techiedelight.com/print-current-date-and-time-in-c/
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <sched.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>

#define THREAD_NUM 2
typedef struct threadParam
{
	pthread_t tid;
	char * fileName;	
} threadParam_t;

/* Create THREAD_NUM number of structure arrays */
threadParam_t threadData[THREAD_NUM];
pthread_t threads[THREAD_NUM];
char * fileName;
timer_t t1timerID;
sem_t mutex;
sem_t timer;

int sig_mask()
{
	sigset_t sset;
	sigemptyset(&sset);
	sigaddset(&sset,SIGUSR1);
	sigaddset(&sset,SIGUSR2);

	pthread_sigmask(SIG_BLOCK,&sset,NULL);

	return 0;
}

double TimeGet()
{
  struct timespec ts = {0, 0};
  clock_gettime(CLOCK_REALTIME, &ts);
  return ((ts.tv_sec)) + ((ts.tv_nsec)*1000000000.0);
}

/* User-Defined Signal Handler for handling USR1 and USR2 Signals */
void sigusrx_handler(int sig)
{
	int ctr = 0;
	FILE *fptr_sh = fopen(fileName,"a+");

	if(!fptr_sh) 
	{
		printf("FILE ERROR IN SIGNAL HANDLER: UNABLE TO OPEN FILE \n");
		exit(1);
	}

	switch(sig)
	{
		case SIGUSR1:
		{
			printf("USR1 \n");
			fprintf(fptr_sh,"<%lf>[THREAD1] Exited...\n",TimeGet());
			fprintf(fptr_sh,"\n\n<%lf>[THREADS] TERMINATING DUE TO SIGUSR1\n\n",TimeGet());	
			timer_delete(t1timerID);
			fclose(fptr_sh);
			pthread_cancel(threadData[0].tid);
			pthread_cancel(threadData[1].tid);
		} break;
		case SIGUSR2:
		{
			fprintf(fptr_sh,"<%lf>[THREAD1] Exited...\n",TimeGet());
			fprintf(fptr_sh, "\n\n<%lf>[THREAD] TERMINATING DUE TO SIGUSR2\n\n",TimeGet());
			timer_delete(t1timerID);
			fclose(fptr_sh);
			pthread_cancel(threadData[0].tid);
			pthread_cancel(threadData[1].tid);
		}break;
		default:
			break;
	}
	return;
}

void statlogger()
{
	sem_post(&timer);
}

void* threadFunc0(void * threadData)
{
	sig_mask();
	int semval;
	sem_wait(&mutex);
	
	printf("Child Thread0 : Entered \n");
	int charCount[26],ctr=0;
	char ch;
	memset(charCount,0,26*sizeof(int));
	
	/* Logging into File */
	threadParam_t * tData0 = (threadParam_t *)(threadData);
	FILE * t0fptr = fopen(tData0->fileName,"a"); 

	if(!t0fptr) 
	{
		printf("[THREAD0] FILE ERROR : UNABLE TO OPEN FILE \n");
		exit(1);
	}
	fprintf(t0fptr, "-------------- THREAD0 : Write Operation --------------\n");
	fprintf(t0fptr,"<%lf>[THREAD0] Entered...\n",TimeGet());

	fprintf(t0fptr,"[THREAD0] POSIX ID : %ld\n",pthread_self());
	fprintf(t0fptr,"[THREAD0] LINUX ID : %d\n",(pid_t)syscall(SYS_gettid));
	

	/* Character Counting Operation */
	FILE * txtfptr = fopen("gdb.txt","r");
	if(!txtfptr)
	{
		perror("[THREAD0] FILE ERROR : READ OPERATION FAILED \n");
	}

	while(!feof(txtfptr))
	{
		ch = fgetc(txtfptr);
		if ((ch >= 65) && (ch <= 90)) ch+=32;
		if ((ch >= 97) && (ch <= 122))
			charCount[ch-97]++;
	}

	fprintf(t0fptr,"[THREAD0] LOG : CHARACTERS WITH COUNT < 100 \n");
	
	for(ctr = 0; ctr < 26 ;ctr++)  
	{
		if((charCount[ctr] < 100) && (charCount[ctr] > 0))
			fprintf(t0fptr,"\t (%c) : %d \n", (char)(ctr+97), charCount[ctr]);
	}
	 
	fclose(txtfptr);   
	printf("Child Thread0 : Exited \n");
	fprintf(t0fptr,"<%lf>[THREAD0] Exited normally...\n",TimeGet());  
	fflush(t0fptr);
	fclose(t0fptr);  
	sem_post(&mutex);
	
	pthread_exit(NULL);
}

void* threadFunc1(void * threadData)
{
	sem_wait(&mutex);
	
	printf("Child Thread1 : Entered \n");

	threadParam_t * tData1 = (threadParam_t *)(threadData);
	FILE * t1fptr ;
	t1fptr = fopen(tData1->fileName,"a"); 
	if(!t1fptr) printf("[THREAD1] FILE ERROR : UNABLE TO OPEN FILE \n");

	fprintf(t1fptr, "-------------- THREAD1 : Write Operation --------------\n");
	fprintf(t1fptr,"<%lf>[THREAD1] Entered...\n",TimeGet());
	fprintf(t1fptr,"[THREAD1] POSIX ID : %ld\n",(pthread_t)pthread_self());
	fprintf(t1fptr,"[THREAD1] LINUX ID : %d\n",(pid_t)syscall(SYS_gettid));
	fclose(t1fptr);
	/* Posix Timer */
	struct itimerspec t1timer;
	struct sigevent t1sigevnt;
	memset(&t1sigevnt, 0, sizeof(struct sigevent));
    memset(&t1timer, 0, sizeof(struct itimerspec));
	t1sigevnt.sigev_notify = SIGEV_THREAD;
	t1sigevnt.sigev_notify_function = statlogger;
	t1sigevnt.sigev_notify_attributes =  NULL;
	t1timer.it_value.tv_sec = 0;				// Expiration Value
	t1timer.it_value.tv_nsec = 100000000;
	t1timer.it_interval.tv_sec = 0;				// Interval Reload Value to use as a Periodic Timer
	t1timer.it_interval.tv_nsec = 100000000;

	/* Create Timer */
	int status =timer_create(CLOCK_MONOTONIC, &t1sigevnt, &t1timerID) ;
	if(status == -1)
	{
		perror("[THREAD1] TIMER ERROR : UNABLE TO CREATE TIMER \n");
		printf("ERROR %d",errno);
	}
	status= timer_settime(t1timerID,0,&t1timer,NULL);
	if( status != 0) 
	{
		perror("[THREAD1] TIMER ERROR : UNABLE TO SET TIMER \n");
		printf("ERROR %d",errno);
	}
	
	while(1)
	{
			sem_wait(&timer);
			int ctr = 0;
			char ch[40];
			FILE *statfptr = fopen("/proc/stat", "r");
			if(!statfptr) perror("FILE ERROR : /proc/stat COULD NOT BE OPENED \n");	
			t1fptr = fopen(tData1->fileName,"a");
			fprintf(t1fptr, "<CPU STAT> \n");
			while (ctr < 2)
			{
				fgets(ch,sizeof(ch),statfptr);
				fprintf(t1fptr,"\t %s",ch);
				ctr++;
			}

			fclose(statfptr);
			fclose(t1fptr);
	}
	printf("Child Thread1 : Exited \n");
	fclose(t1fptr);
	timer_delete(t1timerID);
	sem_post(&mutex);
	pthread_exit(NULL);
}

int main(int argc, char * argv[])
{
	sem_init(&mutex,0,1);
	struct sigaction tsig;
	memset(&tsig,0,sizeof(struct sigaction));
	tsig.sa_handler = &sigusrx_handler;
	sig_mask();

	/* Check if file name has been passed in Command Line */
	if( argc < 2 )
		printf( "ERROR : NO FILE NAME PASSED IN COMMAND LINE FOR PROGRAM %s\n", argv[0] );

	/* Update GLobal Variable fileName */
	fileName = argv[1];

	/*Signal Handling */
	if(!(sigaction(SIGUSR1,&tsig,NULL)) && !(sigaction(SIGUSR2,&tsig,NULL)))
		printf("SIGNAL HANDLER : SUCCESSFULLY SET \n");

	FILE *fptr = fopen("testlog.txt","w");
	if(fptr == NULL) perror("FILE ERROR : LOGFILE COULD NOT BE OPENED \n");
	fprintf(fptr,"MASTER THREAD ENTERED \n");

	fclose(fptr);
	printf("Entered Main PID %lu \n",getpid());
	
	/*Create THREAD_NUM threads */
	for ( int i = (THREAD_NUM-1); i >= 0 ; i-- )
	{
		threadData[i].fileName = fileName; 					//Initilaize filename
	}

	/* Create Thread0 */
	int err= pthread_create(&threads[0], NULL, threadFunc0, (void *)&threadData[0]) ;
	
	if (err)
	{
		printf(" ERROR : pthread_create() FAILED FOR CHILD THREAD0\n");
		exit(1);
	}

	/* Create Thread1 */
	err= pthread_create(&threads[1], NULL, threadFunc1, (void * )&threadData[1]) ;
	if (err)
	{
		printf(" ERROR : pthread_create() FAILED FOR CHILD THREAD1\n");
		exit(1);
	}

	/* Run child threads and wait till they execute */

	if(pthread_join(threads[0], NULL))
	{
		printf("ERROR : pThread_join() FAILED for CHILD THREAD 0");
	}
	
	if(pthread_join(threads[1], NULL))
	{
		printf("ERROR : pThread_join() FAILED for CHILD THREAD 1");
	}
	
	printf("Exit Main");
	return 0;

}
