#include <stdio.h>
#include <stdint.h>
#include <string.h> 
#include <stdlib.h> 
#include <dirent.h>
#include <memory.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h> 
#include <time.h>


#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h> 
#include <sys/select.h>



#define PORT 2000
#define Filename "socketlog.txt"
int  LogTime(FILE *logptr);
void Signal_Init();
void Signal_Handler(int signum);





typedef struct{
    uint32_t PacketSize[5];
    uint8_t PacketData[5][1000];
	uint16_t LED_Status;
    uint16_t PID;

} IPC_Packet;

IPC_Packet client_packet;


 int Interrupt = 0;

/*Functions*/

int main()
{
    
	
	struct sockaddr_in server_address; 
   
	
	char *Socket_Packets[] = { "From Client 1","IPC: Sockets "," Hello server","/*content*/","Bye server"}; 
     
    int PID = getpid();
	IPC_Packet * Received_Buffer;
    
	int Read_Packet_size = 0;
	 int Bytes_Received = 0;
	 
    FILE *logptr = fopen(Filename, "a");
    if(logptr == NULL)
    {
        perror("file not opened");
        return -1;
    }
	LogTime(logptr);
    fprintf(logptr,"  [CLIENT] Process ID: %d\n" ,PID);
	fclose(logptr);
	int Socket_Create;
    if ((Socket_Create = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
	
	logptr = fopen(Filename, "a");
    fprintf(logptr,"--------------------------------------------------------------------\n"); 
    LogTime(logptr);

	fprintf(logptr,"[CLIENT] Socket Created\n" );  
    
	memset(&server_address, '0', sizeof(server_address)); 
    fprintf(logptr,"--------------------------------------------------------------------\n");
	fclose(logptr);
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(PORT); 
       
    if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(Socket_Create, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    }
    printf("[CLIENT LOG] Socket Connection Established\n"); 
    
	while(!Interrupt)
	{
    	for(int i = 0; i < 5 ; i++)
    	{
        	memcpy(client_packet.PacketData[i],Socket_Packets[i],strlen(Socket_Packets[i])+1);
        	client_packet.PacketSize[i] = strlen(Socket_Packets[i]);
    	}
    	client_packet.PID = PID;
    	client_packet.LED_Status = 1;

		int Byte_send = 0;
    	int Packet_Size = sizeof(client_packet);
    	Byte_send = send(Socket_Create,&Packet_Size,sizeof(int), 0);
		logptr = fopen(Filename, "a");
		fprintf(logptr,"--------------------------------------------------------------------\n");
   		LogTime(logptr);fprintf(logptr," CLIENT sending.........\n ");
		fprintf(logptr,"Packet Size %ld\n",Packet_Size);

		fclose(logptr);
	    Byte_send = send(Socket_Create , (char*)&client_packet , sizeof(client_packet), 0 );
		logptr = fopen(Filename, "a");
		fprintf(logptr," PID %d\n",client_packet.PID);
	    for(int i =0 ; i<5 ; i++)
		{
		fprintf(logptr,"Message %s\n" ,client_packet.PacketData[i]);
	    fprintf(logptr,"Size %d\n" ,client_packet.PacketSize[i]);
		}
	    fprintf(logptr,"LED STATUS %d\n" ,client_packet.LED_Status);

		LogTime(logptr);
		fprintf(logptr,"Send Packets Successfully\n");
	    fprintf(logptr,"--------------------------------------------------------------------\n");
	    fclose(logptr);
	    
	   
	    int byte_read = read(Socket_Create, &Read_Packet_size, sizeof(int));
	    logptr = fopen(Filename, "a");
		fprintf(logptr,"--------------------------------------------------------------------\n");
		LogTime(logptr);
		fprintf(logptr,"[Client]Receiving Packets ..... \n");
	    fprintf(logptr,"[CLIENT] Size of Server Packets %ld\n",Read_Packet_size);
		fclose(logptr);

	    int Bytes_Received = 0;
		char *Buffer =  (char*) malloc(Packet_Size);
	    while(Bytes_Received < Packet_Size)
	    {
	        byte_read = read(Socket_Create,Buffer+Bytes_Received, 1024);
	        Bytes_Received= Bytes_Received+ byte_read;
	    }
	 
		Received_Buffer = (IPC_Packet *)Buffer;
		logptr = fopen(Filename, "a");
    	fprintf(logptr,"  Bytes Received : %ld\n" ,Bytes_Received);
 		fprintf(logptr,"  PID %d\n",Received_Buffer->PID);
	    for(int i = 0; i < 5; i++)
	    {
	        fprintf(logptr,"Message: %s\n" , Received_Buffer->PacketData[i]);
	        fprintf(logptr," SIZE %d\n" , Received_Buffer->PacketSize[i]);
	    }
 
	    fprintf(logptr,"LED Status : %d\n" , Received_Buffer->LED_Status);
	   
	  
		LogTime(logptr);
		fprintf(logptr,"Received packets Successfully\n");
	   	fprintf(logptr,"--------------------------------------------------------------------\n");
		fclose(logptr);free(Buffer);
	}
	
	
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
    Interrupt = 1;
    fclose(logptr);
}
