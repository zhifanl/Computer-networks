#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>
  
#define PORT     8889
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

    printf("Translator Micro-server is running...\n");

    char message_buffer[1024];
    // big while loop to keep server running
    while(1){
        int char_received;
        int len=sizeof(server);
        // receive msg from indir server
        char_received = recvfrom(socket_desc, (char *)message_buffer, 1024, 
                    MSG_WAITALL, (struct sockaddr *) &server,
                    (socklen_t *)&len);
        
        message_buffer[char_received]='\0';

        // printf("%s\n",message_buffer);

        char response_buffer[1024];
        // check client content, make it lower case first
        for(int i=0;i<strlen(message_buffer);i++)
        {
            message_buffer[i]=tolower(message_buffer[i]);
        }
        // if it is hello
        if(strncmp(message_buffer,"hello",5)==0)
        {
            strcpy(response_buffer,"> French translation: Bonjour\n");
        }else if(strncmp(message_buffer,"bye",3)==0)
        {
            strcpy(response_buffer,"> French translation: Au revoir\n");
        }else if(strncmp(message_buffer,"how are you",11)==0)
        {
            strcpy(response_buffer,"> French translation: Comment vas-tu\n");
        }else if(strncmp(message_buffer,"i am zhifan",11)==0)
        {
            strcpy(response_buffer,"> French translation: je suis Zhifan\n");
        }else if(strncmp(message_buffer,"what is your name",17)==0)
        {
            strcpy(response_buffer,"> French translation: Quel est ton nom\n");
        }else if(strncmp(message_buffer,"nice to see you",15)==0)
        {
            strcpy(response_buffer,"> French translation: Ravi de vous voir\n");
        }
        else
        {
            strcpy(response_buffer,"The word is not found, try another one\n");
        }

        // send back to indir server, let indir server send back to msg to client
        sendto(socket_desc, (const char *)response_buffer, strlen(response_buffer),
                    MSG_CONFIRM, (const struct sockaddr *) &server, 
                    sizeof(server));
        // set to empty
        memset(&response_buffer, 0, sizeof(response_buffer));
        memset(&message_buffer, 0, sizeof(message_buffer));

    }
    return 0;
}