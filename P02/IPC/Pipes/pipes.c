/*
 * @file pipes.c
 * 
 * @description C Program to create implement inter-process communication using pipes
 *
 * @author Madhumitha Tolakanahalli Pradeep
 * @date 24 February 2019
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define LOGFILE		"pipelog.txt"

volatile sig_atomic_t flag = 0;

int LogTime(FILE *logptr)
{
  
  struct timespec logtime;
  clock_gettime(CLOCK_REALTIME, &logtime);
  fprintf(logptr,"[%ld s %ld us]",logtime.tv_sec,logtime.tv_nsec );
  return 1;
}

void Signal_Handler(int signum)
{
   flag = 1;
   printf("SIGINT Received\n");
}

void Signal_Init()
{
	struct sigaction SigAct;
    memset(&SigAct, 0, sizeof(SigAct));
    SigAct.sa_handler = Signal_Handler;
    sigaction(SIGINT, &SigAct, NULL);
}
typedef struct{
    uint8_t PacketSize[5];
    uint8_t PacketData[5][1000];
    uint8_t LED_Status;
    uint16_t PID;
} IPC_Packets;


int main() {
    int fd1[2], fd2[2];
    int bytesRX,TXbytes;
	
    char *Pipe_C_P[] = {"From Child","Hello Parent","IPC: PIPES","/*Content*/","Bye Parent"}; 
    char *Pipe_P_C[] = {"From Parent","Hello Child","IPC: PIPES","/*Content*/","Bye Child"};
   
    int PID = 0, forkPID = 0;
    IPC_Packets ChildPacket; 	// Child Packet
    IPC_Packets ParentPacket;	// Parent Packet
	
    int BufferSize = sizeof(IPC_Packets);
    
	Signal_Init();
	
    FILE *fptr;
    fptr = fopen(LOGFILE,"w+");

  
    if(fptr == NULL)
    {
        perror("ERROR : File could not be opened");
        return -1;
    }
    fprintf(fptr,"\t\t\tInter Process Communication\n \t\t\t\tPipes\n\n");
	fclose(fptr);
		

 
    
    // Creating Pipe 1
    if (pipe(fd1) == -1) 
	{
        perror(" Error in creating Pipe 1\n");
        return 1;
    }

    // Creating Pipe 2
    if (pipe(fd2) == -1) 
	{
        perror(" Error in creating Pipe 2\n");
        return 1;
    }
	
	// Create child processes
    forkPID = fork();
	if(forkPID == -1)
	{
		perror("Fork unsuccessful\n");
		return 1;
	}
	
    while(!flag)
    {
        if (forkPID > 0)
        { 	
			// Parent process 
			fptr= fopen(LOGFILE,"a");	
            PID = getpid();
            fprintf(fptr,"--------------------------------------------------------------------\n");
            LogTime(fptr);
            printf("Parent Process entered \n");
            fprintf(fptr,"[Parent] Parent Process ID %d\n",PID);
            
			// Create data packet
            for(int i = 0; i < 5; i++)
            {
                memcpy(ParentPacket.PacketData[i],Pipe_P_C[i],strlen(Pipe_P_C[i])+1);
                ParentPacket.PacketSize[i] = strlen(Pipe_P_C[i]);
            }
			
            ParentPacket.PID = PID;
            ParentPacket.LED_Status = 0;
            
            close(fd1[0]); close(fd2[1]);
            fprintf(fptr,"--------------------------------------------------------------------\n");
            LogTime(fptr);fprintf(fptr,"[Parent] Pipes : Sending Data to Child\n");
            for(int i = 0; i < 5; i++)
            {
                fprintf(fptr,"Message : %s \n",ParentPacket.PacketData[i]);
                fprintf(fptr,"Message size :%d \n",ParentPacket.PacketSize[i]);
            }
            fprintf(fptr,"LED Status %d \n",ParentPacket.LED_Status);
            fprintf(fptr,"Parent PID %d \n", ParentPacket.PID);
			LogTime(fptr);fprintf(fptr,"[Parent] Pipes : Sent Data successfully\n");
            fprintf(fptr,"--------------------------------------------------------------------\n");
            fclose(fptr);
			
            char *pw_ptr = (char*)malloc(BufferSize);
            pw_ptr = (char*)&ParentPacket;
            write(fd1[1], pw_ptr, BufferSize);
            
            // Read message from child
            char *Pipeptr_read = (char*)malloc(BufferSize);
            size_t TotBytes = 0;
			
            for(;TotBytes < BufferSize;)
            {
                if((bytesRX = read(fd2[0], Pipeptr_read+TotBytes, 1024)) < 0)
                {
                    perror("parent read error");
                }
                TotBytes+=bytesRX;
            }
			
            IPC_Packets *PipePacket_ptr= (IPC_Packets*)Pipeptr_read;
            fopen(LOGFILE,"a");
            fprintf(fptr,"--------------------------------------------------------------------\n");
            LogTime(fptr);fprintf(fptr,"[Parent] Pipes : Receiving data from child\n");
            for(int i = 0; i < 5; i++)
            {
                fprintf(fptr,"Message : %s \n", PipePacket_ptr->PacketData[i]);
                fprintf(fptr,"Message size :%d \n", PipePacket_ptr->PacketSize[i]);
            }
            fprintf(fptr," LED Status %d \n", PipePacket_ptr->LED_Status);
            fprintf(fptr,"Parent PID %d \n", PipePacket_ptr->PID);
            LogTime(fptr);fprintf(fptr,"[Parent] Pipes : Received data successfully\n");
            fprintf(fptr,"--------------------------------------------------------------------\n");
            fclose(fptr);
            sleep(5); 

        }
        else 
        { 
			// Child Process
            PID = getpid();
            fopen(LOGFILE,"a");
            printf("Child Process Entered \n");
            fprintf(fptr,"--------------------------------------------------------------------\n");
            LogTime(fptr);fprintf(fptr,"  Child Process ID %d\n",PID);
            bytesRX = 0;
			
            // Creating packet
            for(int i = 0; i < 5; i++)
            {
                memcpy(ChildPacket.PacketData[i],Pipe_C_P[i],strlen(Pipe_C_P[i])+1);
                ChildPacket.PacketSize[i] = strlen(Pipe_C_P[i]);
            }
            ChildPacket.PID = PID;
            ChildPacket.LED_Status = 1;

            // Message from parent
            char *PipePacket_ptr = (char*)malloc(BufferSize);
            size_t TotBytes = 0;
            close(fd1[1]); close(fd2[0]);
            
            while(TotBytes < BufferSize)
            {
                if((bytesRX = read(fd1[0], PipePacket_ptr+TotBytes, 1024)) < 0)
                {
                    perror("Error in reading");
                }
                TotBytes+=bytesRX;
                
            }
            
            IPC_Packets *ChildPacket_ptr= (IPC_Packets*)PipePacket_ptr;
            fprintf(fptr,"--------------------------------------------------------------------\n");
            LogTime(fptr);fprintf(fptr," Pipes : Received data from Parent\n");
            for(int i = 0; i < 5; i++)
            {
                fprintf(fptr,"Message : %s \n" ,ChildPacket_ptr->PacketData[i]);
                fprintf(fptr,"Message size :%d \n" , ChildPacket_ptr->PacketSize[i]);
            }
            fprintf(fptr,"Led Status %d \n" , ChildPacket_ptr->LED_Status);
            fprintf(fptr,"Child PID %d \n" , ChildPacket_ptr->PID);
            LogTime(fptr);fprintf(fptr," Pipes : Received data successfully\n");
            fprintf(fptr,"--------------------------------------------------------------------\n");

            //send to parent write
            fprintf(fptr,"--------------------------------------------------------------------\n");
            LogTime(fptr);fprintf(fptr,"Pipes :Sending data to Parent\n" );
            for(int i = 0; i < 5; i++)
            {
                fprintf(fptr,"Message : %s \n" , ChildPacket.PacketData[i]);
                fprintf(fptr,"Message size :%d \n" , ChildPacket.PacketSize[i]);
            }
            fprintf(fptr,"LED Status %d \n" , ChildPacket.LED_Status);
            fprintf(fptr,"Child PID %d \n" , ChildPacket.PID);
            LogTime(fptr);fprintf(fptr,"Pipes : Sent data successfully\n" );
            fprintf(fptr,"--------------------------------------------------------------------\n");
            char *PipeWrite_ptr = (char*)malloc(BufferSize);
            PipeWrite_ptr = (char*)&ChildPacket;
            write(fd2[1], PipeWrite_ptr, BufferSize);
            fclose(fptr);
        }
    }
    fopen(LOGFILE, "a");
    fprintf(fptr,"--------------------------------------------------------------------\n");
    fprintf(fptr,"[Interrupt] SIGINT RECEIVED, Terminating Pipes\n");
    fprintf(fptr,"--------------------------------------------------------------------\n");
    fclose(fptr);
    return 0;

}