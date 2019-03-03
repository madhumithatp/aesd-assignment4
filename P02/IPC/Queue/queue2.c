#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#define true   			(1)
#define false 			(0)
#define Filename  		"queuelog.txt"
#define Q_NAME   		"/posix_q"
#define Q_Permisions 	(0666)
#define MESSAGE_LIMIT 	(10)
#define MAX_MSG_SIZE 	(256)
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 8

typedef struct{
    
	pid_t PID;
	uint8_t PacketData[5][1000];
	uint32_t PacketSize[5];
    uint16_t LED_State;
    
} IPC_Packet;

volatile int Interrupt = false;

/*Functions*/
int  LogTime(FILE *logptr);
void Signal_Init();
void Signal_Handler(int signum);

int main ()
{
	
	char *Queue2_Packets[] = {"From Queue 2","IPC: POSIX QUEUE","hello Q1 ","/*content*/","bye Q1"};
     FILE *logptr;
	uint16_t token_number = 1;
	 IPC_Packet Packet_Send = {0};
	 IPC_Packet Packet_Received = {0};
	pid_t PID = getpid();
	char *S_packet_ptr, *R_packet_ptr;
    
	Signal_Init();
   
	 struct mq_attr Q_attr;

  
    Q_attr.mq_maxmsg = sizeof(int);
    Q_attr.mq_msgsize = sizeof(Packet_Send);
    Q_attr.mq_flags = 0;
	
	
	
  
    logptr = fopen(Filename,"w+");
    if(!logptr) perror("[Queue 2] FILE ERROR: UNABLE TO OPEN LOG FILE\n");
	fclose(logptr);
	// Open in append mode 
    logptr = fopen(Filename,"a");
   if(!logptr) perror("[Queue 2] FILE ERROR: UNABLE TO WRITE LOG FILE\n");
    
	fprintf(logptr,"\t\t\tInter Process Communication\n \t\t\t\tPOSIX QUEUES\n");
    printf("[Queue 2] Process ID %d\n",PID);
    LogTime(logptr);
	fprintf(logptr,"[Queue 2] Process ID %d\n",PID );
    fclose(logptr);
	
	
	
	mqd_t Q_descriptor;  //Queue Desctiptor
	
	if ((Q_descriptor = mq_open (Q_NAME, O_CREAT | O_RDWR, 0666, &Q_attr)) == -1)
		{
        perror ("[Queue 2] Error : Queue Open\n");
        exit (1);
    }
	
	/*Populating Send Packet Structure*/
    for(int i = 0; i < 5;i++)
    {
        memcpy(Packet_Send.PacketData[i],Queue2_Packets[i],strlen(Queue2_Packets[i])+1);
        Packet_Send.PacketSize[i] = strlen(Queue2_Packets[i]);
    }
	
    Packet_Send.PID = PID;   
    Packet_Send.LED_State = 1;
    //s_packet.token = 1;

    
    
   S_packet_ptr = (char*)&Packet_Send;
    
    
    while (!Interrupt) 
	{

	sleep(1);
        // send reply message to client
        
        logptr = fopen(Filename,"a");
		
		
		
		
        R_packet_ptr = (char*)&Packet_Received;
		// get the oldest message with highest priority
        fprintf(logptr,"--------------------------------------------------------------------\n");
        LogTime(logptr);
		fprintf(logptr,"[Queue 2 ] Receiving Packets ........  \n");
		
		if (mq_receive (Q_descriptor, R_packet_ptr , sizeof(Packet_Received), NULL) == -1) {
            perror ("Server: mq_receive");
			fprintf(logptr, "Error Receiving Packets\n");
			fprintf(logptr,"--------------------------------------------------------------------\n");
            exit (1);
        }
      
        //printf("In Parent: Reading from pipe 2 – Message is %s\n",p_packet_ptr->PacketData[0]);
		

        fprintf(logptr," Queue 1 PID %d \n", Packet_Received.PID);
        for(int i = 0; i < 5; i++)
        {
            fprintf(logptr," Message : %s \n", Packet_Received.PacketData[i]);
            fprintf(logptr,"Packet size :%d \n", Packet_Received.PacketSize[i]);
        }
        fprintf(logptr,"Led State %d \n", Packet_Received.LED_State);
		fprintf(logptr, "Received Packets\n");
			fprintf(logptr,"--------------------------------------------------------------------\n");
        fclose(logptr);
        printf ("Server: message received.\n");
	
 
        logptr = fopen(Filename,"a");

		fprintf(logptr,"[Queue 2 ] Sending Packets ........  \n");
		
		fprintf(logptr,"\t\t[Queue 2] Process ID : [%d] \n", Packet_Send.PID);
        for(int i = 0; i < 5; i++)
       		 {
            fprintf(logptr,"\t\t Message : %s \n", Packet_Send.PacketData[i]);
            fprintf(logptr,"<\t\tPacket Size :%d \n", Packet_Send.PacketSize[i]);
        }

        	fprintf(logptr," Led State : %d \n", Packet_Send.LED_State);
		if (mq_send (Q_descriptor, S_packet_ptr, sizeof(Packet_Send), 0) == -1)
			{
				
            		perror ("Queue 2 : Error Sending Packets\n");
			LogTime(logptr);
			fprintf(logptr,"[Queue 2 ] Unable to send packets !!!! \n");
			fprintf(logptr,"--------------------------------------------------------------------\n");
			}
		else
		{				
			LogTime(logptr);
			fprintf(logptr,"[Queue 2 ] Packets Sent !!!! \n");
       			fprintf(logptr,"--------------------------------------------------------------------\n");
       			fclose(logptr);
		}
		printf ("Server: response sent to client.\n");
       		 token_number++;

	}
	//fprintf(logptr,"Interrupt Received [ SIGINT] Terminating Queues\n")
    fclose(logptr);
	
    	  if (mq_close (Q_descriptor) == -1) {
        perror (": mq_close");
        exit (1);
    }
	
		
		exit(0);
}




int  LogTime(FILE *logptr)
{
  
  struct timespec logtime;
  clock_gettime(CLOCK_REALTIME, &logtime);
  fprintf(logptr,"[%ld s %ld us]",logtime.tv_sec,logtime.tv_nsec );
  return 1;
}
void Signal_Init()
{
	struct sigaction tsig;
    memset(&tsig, 0, sizeof(tsig));
    tsig.sa_handler = Signal_Handler;
    sigaction(SIGINT, &tsig, NULL);
	
}

void Signal_Handler(int signum)
{
    FILE *logptr = fopen(Filename,"a");
	LogTime(logptr);
    fprintf(logptr,"[Interrupt] SIGINT RECEIVED, Terminating Queue\n");
    fclose(logptr);
    Interrupt = true;
}
