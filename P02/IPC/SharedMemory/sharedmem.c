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
	int FD_ShMem;							//File Descriptor to shared memory
	void* ptr_ShMem;  
	int PID = getpid();
	IPC_Packet txpacket = {0};
    const int txSize = sizeof(txpacket);
	char const *Msg_P1[] = {"From Process 1","IPC: POSIX QUEUE"," Hello P2","/*content*/","Bye P2"};
	
    printf("Process 1 Entered\n");
    
	Signal_Init();

    FILE *fptr;
    fptr = fopen(LOGFILE,"w+");
    if(fptr == NULL)
    {
        printf("ERROR : File cant be opened \n");
    }
    LogTime(fptr);
    
	/* Create Semaphore */
    sem_t *sem = sem_open(SEMAPHORE, O_CREAT, 0666, 0); 
    if(SEM_FAILED == sem)
    {
        printf("ERROR : Sempahore Open failed \n");
        return -1;
    }
	
    /* Create Shared Memory Object */
    FD_ShMem = shm_open(IPC_SHAREDMEM, O_CREAT | O_RDWR, 0666);
	
    if(FD_ShMem < 0)
    {
        printf("Error : Shared Memory opening failed \n");
        return -1;
    } 
    fprintf(fptr,"--------------------------------------------------------------------\n");
    LogTime(fptr);fprintf(fptr,"[Process 1]Shared Memory Opened\n");
	fclose(fptr);
    
    if(ftruncate(FD_ShMem, txSize))
	{
		printf("Error: ftruncate failed \n");
	}
  
    for(int i = 0; i < 5; i++)
    {
        memcpy(txpacket.PacketData[i],Msg_P1[i],strlen(Msg_P1[i])+1);
        txpacket.PacketSize[i] = strlen(Msg_P1[i]);
    }

    txpacket.PID = PID;
    txpacket.LED_State = 0;
	
    ptr_ShMem = mmap(NULL, txSize, PROT_WRITE | PROT_READ, MAP_SHARED, FD_ShMem, 0);
    if(MAP_FAILED == ptr_ShMem)
    {
        printf("Error : Memory Mapping failed");
        shm_unlink(IPC_SHAREDMEM); 
        return -1;
    } 

    while(!flag)
    {
        sleep(1);
        memcpy((char*)ptr_ShMem,(char*)&txpacket, sizeof(txpacket));
        fptr = fopen(LOGFILE,"a");
        // fprintf(fptr,"--------------------------------------------------------------------\n");
        // LogTime(fptr);fprintf(fptr,"[Process 1] Shared Memory : Copying Data from Process1 to Shared Memory\n");
        // for(int i = 0; i < 5; i++)
        // {
        //     fprintf(fptr, "Message : %s \n", txpacket.PacketData[i]);
        //     fprintf(fptr,"Message size :%d \n", txpacket.PacketSize[i]);
        // }
        // fprintf(fptr,"Led Status: %d \n",txpacket.LED_State);
        // fprintf(fptr,"Process 1 Pid :%d \n",txpacket.PID);
        // LogTime(fptr);fprintf(fptr,"Shared Memory : Copying data Successfully\n" );
        // fprintf(fptr,"--------------------------------------------------------------------\n");
        // fclose(fptr);
        
        sem_post(sem);
        sem_wait(sem);
		
        IPC_Packet rxpacket;
        printf("Before mrmcopy \n");
        memcpy((char*)&rxpacket, (char*)ptr_ShMem,sizeof(IPC_Packet));
        
        printf("after mrmcopy \n");
        fptr = fopen(LOGFILE,"a");
        printf("Before rx from p2 \n");
        fprintf(fptr,"--------------------------------------------------------------------\n");
        LogTime(fptr);
        fprintf(fptr,"[Process 1] Shared Memory : Received data from Process2\n");
	   
        for(int i = 0; i < 5; i++)
        {
            fprintf(fptr,"Message : %s \n", rxpacket.PacketData[i]);
            fprintf(fptr,"Message size :%d \n", rxpacket.PacketSize[i]);
        }
       
        fprintf(fptr,"Led Status %d \n", rxpacket.LED_State);
        fprintf(fptr,"Process 2 Pid %d \n", rxpacket.PID);
	    LogTime(fptr);fprintf(fptr,"[Process 1] Shared Memory : Received data successfully\n");
        fprintf(fptr,"--------------------------------------------------------------------\n");
        fclose(fptr);
    }
	
    printf("SIGINT Received \n");
    fopen(LOGFILE,"a");
    fprintf(fptr,"--------------------------------------------------------------------\n");
    LogTime(fptr);fprintf(fptr,"[Interrupt] SIGINT RECEIVED, Terminating Shared Memory\n");
    fprintf(fptr,"--------------------------------------------------------------------\n");
    fclose(fptr);
    shm_unlink(IPC_SHAREDMEM);
    sem_unlink(SEMAPHORE); 
    return 0; 
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
	struct sigaction SigAct;
    memset(&SigAct, 0, sizeof(SigAct));
    SigAct.sa_handler = Signal_Handler;
    sigaction(SIGINT, &SigAct, NULL);
}

void Signal_Handler(int signum)
{
	flag = 1;
}
