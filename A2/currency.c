#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>
  
#define PORT     8890
#define MSG_CONFIRM 0


int main(){
    int socket_desc;
    struct sockaddr_in server;

    if ( (socket_desc = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
    {
        perror("socket creation failed");
        exit(1);
    }
    memset(&server, 0, sizeof(server));
    // memset(&client, 0, sizeof(client));
      
    //create server  
    server.sin_family    = AF_INET; // IPv4
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
      
    // Bind the socket with the server
    if ( bind(socket_desc, (const struct sockaddr *)&server, sizeof(server)) < 0 )
    {
        perror("bind failed");
        exit(1);
    }

    printf("Currency Micro-server is running...\n");

    char message_buffer[1024];
    // big while loop to keep running
    while(1)
    {
        int char_received;
        int len=sizeof(server);
        // receive msg from indir server
        char_received = recvfrom(socket_desc, (char *)message_buffer, 1024, 
                    MSG_WAITALL, (struct sockaddr *) &server,
                    (socklen_t *)&len);
        
        message_buffer[char_received]='\0';

        // printf("%s\n",message_buffer);

        char response_buffer[1024];
        for(int i=0;i<strlen(message_buffer);i++)
        {
            message_buffer[i]=tolower(message_buffer[i]);
        }
        
        char temp_1[1024];
        int i=0;
        // parse the content of the msg, to get amount, source currency and dest currency
        for(;i<strlen(message_buffer);i++)
        {
            if(message_buffer[i]=='-')
            {
                break;
            }
            temp_1[i]=message_buffer[i];
            
        }
        temp_1[i]='\0';
        // turn string to int
        int amount=atoi(temp_1);
        // print the amount out
        printf("Amount is: %i\n",amount);


        i++; // get rid of the '-'
        char temp_2[1024];
        int j=0;
        // keep parsing
        for(;i<strlen(message_buffer);i++)
        {
            if(message_buffer[i]=='-')
            {
                break;
            }
            temp_2[j]=message_buffer[i];
            j++;
            
        }
        temp_2[j-1]='\0'; // there is a \n at temp_2[j]
        printf("Source currency is: %s\n",temp_2);

        i++; // get rid of the '-'
        char temp_3[1024];
        int k=0;
        // keep parsing the content
        for(;i<strlen(message_buffer);i++)
        {
            if(message_buffer[i]=='-')
            {
                break;
            }
            temp_3[k]=message_buffer[i];
            k++;
            
        }
        temp_3[k-1]='\0'; // there is a \n at temp_3[k]
        printf("Destination currency is: %s\n",temp_3);

        // check if the source and dest currency match the conditions here
        // CAD to USD
        if(strncmp(temp_2,"cad",3)==0 && strncmp(temp_3,"usd",2)==0)
        {
            amount*=0.78;
            sprintf(response_buffer, "%d", amount);   
        }
        // CAD to EURO
        else if(strncmp(temp_2,"cad",3)==0 && strncmp(temp_3,"euro",4)==0)
        {
            amount*=0.68;
            sprintf(response_buffer, "%d", amount);   
        }
        // CAD to GBP
        else if(strncmp(temp_2,"cad",3)==0 && strncmp(temp_3,"gbp",3)==0)
        {
            amount*=0.58;
            sprintf(response_buffer, "%d", amount);   
        }
        // CAD to BTC
        else if(strncmp(temp_2,"cad",3)==0 && strncmp(temp_3,"btc",3)==0)
        {
            amount*=0.000016;
            sprintf(response_buffer, "%d", amount);   
        }
        // everything else considered as invalid input
        else
        {
            strcpy(response_buffer,"The currency you are looking for is not found.\n");
        }

        // strcat(response_buffer,"\n");
        // send to user
        sendto(socket_desc, (const char *)response_buffer, strlen(response_buffer),
                    MSG_CONFIRM, (const struct sockaddr *) &server, 
                    sizeof(server));

        memset(&response_buffer, 0, sizeof(response_buffer));
        memset(&message_buffer, 0, sizeof(message_buffer));
    }
    return 0;
}