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
#define Filename log.txt

int  LogTime(FILE *logptr);
void Signal_Init();
void Signal_Handler(int signum);

typedef struct
{
    uint32_t PacketSize[4];
    uint8_t PacketData[4][1024];
    uint16_t LED_Status;
    uint16_t PID;
} IPC_Packet;

IPC_Packet s_packet;
IPC_Packet Received_Buffer;
int  Interrupt = 0;


int main()
{
    int Socket_Create;
    struct sockaddr_in client_address; 
    int addr_len = sizeof(client_address); 
	
	int options = 1;
	
    char *Socket_Packets[] = {"From Server 1","IPC: Sockets "," Hello client","/*content*/","Bye client"}; 
    char *;
	char Client_Addr[50];
	
	
	int PID = getpid
	
   
    FILE *logptr = fopen(Filename, "w+");
	fprintf(logptr,"\t\t\tInter Process Communication\n \t\t\t\tSocket\n\n");
	fclose(logptr);
	
    logptr = fopen(Filename, "a");
    if(logptr == NULL)
    {
        perror("[ERROR]");
    }
   
    LogTime(logptr);fprintf(logptr," [SERVER] Process ID: %d\n",process_id);
    
    if ((Socket_Create = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(0); 
    }
	
	LogTime(logptr);
	fprintf(logptr,"--------------------------------------------------------------------\n");
    fprintf(logptr,"[SERVER] Socket Created  %d\n", Socket_Create);  
	fprintf(logptr,"--------------------------------------------------------------------\n");
     
	 
	 
    setsockoptions(Socket_Create, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &options, sizeof(options));
    
     
    client_address.sin_family = AF_INET; 
    client_address.sin_addr.s_addr = INADDR_ANY; 
    client_address.sin_port = htons( PORT ); 
    
	
    if (bind(Socket_Create, (struct sockaddr *)&client_address,  
                                 sizeof(client_address))<0) 
    { 
        perror("Error: Binding Failed!\n"); 
        exit(0); 
    }

	
	fprintf(logptr,"--------------------------------------------------------------------\n");
    fprintf(logptr,"  [SERVER LOG] Socket Binded\n" );
	fprintf(logptr,"--------------------------------------------------------------------\n");
    
	if (listen(Socket_Create, 4) < 0) 
    { 
        perror("Error: Listening\n"); 
        exit(0); 
    }
	
    fclose(logptr);

    while(!Interrupt)
    {
        fopen(Filename, "a");
		
		int Socket_new;
        if ((Socket_new = accept(Socket_Create, (struct sockaddr *)&client_address,  
                        (socklen_t*)&addr_len))<0) 
        { 
            perror("accept"); 
            exit(0); 
        } 

        
        if (inet_ntop(AF_INET, &client_address.sin_addr.s_addr, Client_Addr,sizeof(Client_Addr)) != NULL)
		{
        printf("[SERVER ] Client Address = %s/%d\n",Client_Addr,PORT);
		LogTime(logptr);
        fprintf(logptr," [SERVER ] Client Address = %s/%d\n",Client_Addr,PORT);
        }
		else
		{
        printf("Error: Address \n");
        }
        
        
		
		
        for(int i = 0; i < 5 ; i++)
        {
            memcpy(s_packet.PacketData[i],Socket_Packets[i],strlen(Socket_Packets[i])+1);
            s_packet.PacketSize[i] = strlen(Socket_Packets[i]);
        }
           
        s_packet.PID = PID;
        s_packet.LED_Status = 0;
      
        
        
        
        int PacketSize = 0;
		
        int byte_read = read(Socket_new, &PacketSize, sizeof(int));
        
		
        
        fprintf(logptr,"  [SERVER] Size of Client Packet  %ld\n" ,PacketSize);
        int Bytes_Received = 0;

        char *Received_Buffer =  (char*) malloc(PacketSize);
  
        while(Bytes_Received < PacketSize)
        {
            byte_read = read(Socket_new, Received_Buffer+Bytes_Received, 1024);
            Bytes_Received =Bytes_Received + byte_read;
        }
		
		
		fprintf(logptr,"--------------------------------------------------------------------\n");
		LogTime(logptr);fprintf(logptr,"Receiving Data .......\n");
        fprintf(logptr,"Byte Received: %ld\n",Bytes_Received);
		fprintf(logptr,"PID  %d\n" , Received_Buffer.PID);
        for(int i = 0; i < 5; i++)
        {
            fprintf(logptr,"Message: %s\n", Received_Buffer.PacketData[i]);
            fprintf(logptr," Size: %d\n", Received_Buffer.PacketSize[i]);
        }
        fprintf(logptr," LED STATUS %d\n", Received_Buffer.LED_Status);
		LogTime(logptr);fprintf(logptr,"RECEIVED PACKETS SUCCESSFULLY\n");
		fprintf(logptr,"--------------------------------------------------------------------\n");
        
        
		int Send_Packet_size = sizeof(s_packet);
        sbytes = send(Socket_new,&Send_Packet_size,sizeof(int), 0);
		fprintf(logptr,"--------------------------------------------------------------------\n");
		LogTime(logptr);fprintf(logptr,"Server Sending\n");
		
        fprintf(logptr,"Packet Size %ld\n",PacketSize);
		
        fprintf(logptr,"PID  %d\n" ,s_packet.PID);
		
		
		for (int i=0; i < 5 ; i++)
		{     
	 		fprintf(logptr,"=Message :  %s\n" ,s_packet.PacketData[i]);
        	fprintf(logptr," size %d\n",s_packet.PacketSize[i]);
    	}


        fprintf(logptr,"LED Status %d\n" ,s_packet.LED_Status);
        fprintf(logptr,"--------------------------------------------------------------------\n");
   
   
        sbytes = send(Socket_new , (char*)&s_packet , sizeof(s_packet), 0 );
       
	   
        fclose(logptr);
		
        free(Received_Buffer);     
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
    fclose(logptr);
    Interrupt = 1;
}
