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
#define Filename socketlog.txt
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
	IPC_packet Received_Buffer;
    
	int Read_Packet_size = 0;
	 int Bytes_Received = 0;
	 
    FILE *logptr = fopen("log.txt", "a");
    if(logptr == NULL)
    {
        perror("file not opened");
        return -1;
    }
	LogTime(logptr);
    fprintf(logptr,"  [CLIENT] Process ID: %d\n" ,process_id);
    printf("Client Process ID: %d\n",process_id);
	int Socket_Create;
    if ((Socket_Create = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
	
    fprintf(logptr,"--------------------------------------------------------------------\n"); 
    LogTime(logptr);printf("[CLIENT] Socket Created\n");

	fprintf(logptr,"[CLIENT] Socket Created\n" );  
    
	memset(&server_address, '0', sizeof(server_address)); 
    fprintf(logptr,"--------------------------------------------------------------------\n");

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
    fclose(logptr);
	while(!done)
	{
    	for(int i = 0; i < 5 ; i++)
    	{
        	memcpy(c_packet.PacketData[i],Socket_Packets[i],strlen(Socket_Packets[i])+1);
        	client_packet.PacketSize[i] = strlen(Socket_Packets[i]);
    	}
    	client_packet.PID = process_id;
    	client_packet.LED_Status = 1;

    	fopen(Filename, "a");
	
    	int PayloadSize = sizeof(Client_packet);
    	Byte_send = send(Socket_Create,&PayloadSize,sizeof(int), 0);
		fprintf(logptr,"--------------------------------------------------------------------\n");
   		LogTime(logptr);fprintf(logptr," CLIENT sending.........\n ");
		fprintf(logptr,"Packet Size %ld\n",Packet_Size);



	    Byte_send = send(Socket_Create , (char*)&Client_packet , sizeof(Client_packet), 0 );
		fprintf(logptr," PID %d\n",Client_packet.PID);
	    for(int i =0 ; i<5 ; i++)
		{
		fprintf(logptr,"Message %s\n" ,Client_packet.PacketData[i]);
	    fprintf(logptr,"Size %d\n" ,Client_packet.PacketSize[i]);
		}
	    fprintf(logptr,"LED STATUS %d\n" ,Client_packet.LED_Status);

		LogTime(logptr);
		fprintf(logptr,"Send Packets Successfully\n");
	    fprintf(logptr,"--------------------------------------------------------------------\n");
	    fclose(logptr);
	    
	   
	    int byte_read = read(Socket_Create, &Read_Packet_size, sizeof(int));
	    fopen("log.txt", "a");
		fprintf(logptr,"--------------------------------------------------------------------\n");
		LogTime(logptr);
		fprintf(logptr,"[Client]Receiving Packets ..... \n");
	    fprintf(logptr,"[CLIENT] Size of Server Packets %ld\n",Read_Packet_size);
   
	   
	    while(Bytes_Received < Packet_Size)
	    {
	        byte_read = read(Socket_Create, recv_buffer+Bytes_Received, 1024);
	        Bytes_Received= Bytes_Received+ byte_read;
	    }
	 


    	fprintf(logptr,"  Bytes Received : %ld\n" ,Bytes_Received);
 		fprintf(logptr,"  PID %d\n",Bytes_Received.PID);
	    for(int i = 0; i < 5; i++)
	    {
	        fprintf(logptr,"Message: %s\n" , Bytes_Received.PacketData[i]);
	        fprintf(logptr," SIZE %d\n" , Bytes_Received.PacketSize[i]);
	    }
 
	    fprintf(logptr,"LED Status : %d\n" , Bytes_Received.LED_Status);
	   
	  
		LogTime(logptr);
		fprintf(logptr,"Received packets Successfully\n");
	   	fprintf(logptr,"--------------------------------------------------------------------\n");
    
	}
	fclose(logptr);
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
    fclose(logptr);
    Interrupt = 1;
}