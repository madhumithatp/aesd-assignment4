#include <stdio.h> 
#include <stdlib.h> 
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>        
#include <fcntl.h>           
#include <string.h> 
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <semaphore.h>
#include <string.h> 
#include <time.h>

#define SEMAPHORE "/mysemaphore"  
#define IPC_SHAREDMEM "/my_shared_memory" 
#define  LOGFILE "shmemlog.txt"

/*Functions*/
int  LogTime(FILE *logptr);
void Signal_Init();
void Signal_Handler(int signum);

typedef struct{
	pid_t PID;
	uint8_t PacketData[5][1000];
	uint32_t PacketSize[5];
    uint16_t LED_State;
} IPC_Packet;

volatile sig_atomic_t flag = 0;
  
int main() 
{ 
	int FD_ShMem;
	void* ptr_ShMem; 
	const int txSize = sizeof(txpacket); 
	int PID = getpid();
	IPC_Packet txpacket = {0};
	char *Msg_P2[] = {"From Process 2","IPC: POSIX SHARED QUEUE","Hello P1", "/*Content*/","Bye P1"};
   
	Signal_Init();
	
    FILE *fptr;
    fptr = fopen(LOGFILE,"w+");
    if(fptr == NULL)
    {
        perror("ERROR : File cant be opened ");
    }
    fprintf(fptr,"\t\t\tInter Process Communication\n \t\t\t\t SHARED MEMORY\n\n");
    LogTime(fptr);fprintf(fptr,"[Process 2] Shared Memory Opened\n");
    fclose(fptr);
	
    sem_t *sem = sem_open(IPC_SHAREDMEM, O_CREAT, 0666, 0); 
    if(SEM_FAILED == sem)
    {
        perror("ERROR : Sempahore Open failed ");
        return -1;
    } 
	
   
    FD_ShMem = shm_open(name, O_CREAT | O_RDWR, 0666); 
    if(FD_ShMem < 0)
    {
        perror("shmopen fail");
        return -1;
    }
 
    fclose(fptr);
    

    // Creating Tx Data Packet
	for(int i = 0; i < 5; i++)
    {
        memcpy(txpacket.PacketData[i],Msg_P2[i],strlen(Msg_P2[i])+1);
        txpacket.PacketSize[i] = strlen(Msg_P2[i]);
    }
       
    txpacket.PID = PID;
    txpacket.LED_State = 0;

    ptr_ShMem = mmap(NULL, txSize, PROT_WRITE | PROT_READ, MAP_SHARED, FD_ShMem, 0);
    if(MAP_FAILED == ptr_ShMem)
    {
        perror("Error : Memory Mapping failed");
        shm_unlink(name); 
        return -1;
    } 
	
    while(!flag)
    {
		sleep(1);
        sem_wait(sem);
        IPC_Packet *rxPacket = (IPC_Packet*)ptr_ShMem;
        printf("%s\n",rxPacket->PacketData[0]) ;
        fopen(LOGFILE,"a");
        fprintf(fptr,"--------------------------------------------------------------------\n");
        LogTime(fptr);fprintf(fptr,"[Process 2] Shared Memory : Recieved Data from Process 1\n" ); 
        for(int i = 0; i < 5; i++)
        {
            fprintf(fptr,"Message : %s \n", rxPacket->PacketData[i]);
            fprintf(fptr,"Message size :%d \n", rxPacket->PacketSize[i]);
        }
        
        fprintf(fptr,"Led Status %d \n" , rxPacket->LED_State);
        fprintf(fptr,"Process 1 Pid %d \n" , rxPacket->PID);
        LogTime(fptr);fprintf(fptr,"[Process 2] Shared Memory : Recieved Data Successfully\n" );
        fprintf(fptr,"--------------------------------------------------------------------\n"); 
        LogTime(fptr);
        fprintf(fptr,"[Process 2] Shared Memory : Copying data from Process 2 to shared memory-\n" );
        for(int i = 0; i < 5; i++)
        {
            fprintf(fptr,"Message : %s \n" , txpacket.PacketData[i]);
            fprintf(fptr,"Message size :%d \n" , txpacket.PacketSize[i]);
        }
        fprintf(fptr,"LED Status : %d \n" , txpacket.LED_State);
        fprintf(fptr,"Process 2 PID :%d \n" , txpacket.PID);
        LogTime(fptr);fprintf(fptr,"[Process 2] Shared Memory : Copied data successfully\n" );
        fprintf(fptr,"--------------------------------------------------------------------\n");
        
        sleep(1);
        memcpy((char*)ptr_ShMem,(char*)&txpacket, sizeof(txpacket));
        fclose(fptr);
        sem_post(sem);
        sleep(1);
        
    }
    fopen(LOGFILE,"a");
    fprintf(fptr,"--------------------------------------------------------------------\n");
    fprintf(fptr,"[Interrupt] SIGINT RECEIVED, Terminating Shared Memory\n");
    fprintf(fptr,"--------------------------------------------------------------------\n");
    fclose(fptr);
    shm_unlink(name);
    sem_unlink(IPC_SHAREDMEM); 
    return 0; 
	
} 

int LogTime(FILE *logptr)
{
	struct timespec logtime;
	clock_gettime(CLOCK_REALTIME, &logtime);
	fptrrintf(logptr,"[%ld s %ld us]",logtime.tv_sec,logtime.tv_nsec );
	return 1;
}

void Signal_Init()
{
	struct sigaction SigAct;
    memset(&SigAct, 0, sizeof(SigAct));
    SigAct.sa_handler = Signal_Handler;
    sigaction(SIGINT, &SigAct, NULL);
}

void Signal_Handler(int signum)
{
	flag = 1;
    printf("SIGINT Received \n");
}
