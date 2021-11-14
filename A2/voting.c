#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>
  
#define PORT     8891
#define MSG_CONFIRM 0

// Candidate struct with name, id and number of votes
struct Candidate{
    char* name;
    int id;
    int vote;
};

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
    // server created
    server.sin_family    = AF_INET; // IPv4
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
      
    // Bind the socket with the server
    if ( bind(socket_desc, (const struct sockaddr *)&server, sizeof(server)) < 0 )
    {
        perror("bind failed");
        exit(1);
    }
    
    printf("Voting Micro-server is running...\n");

    // initialize a list of candidates
    struct Candidate candidates[5];
    candidates[0].name="Justin Trudeau";
    candidates[0].id=1;
    candidates[0].vote=666;
    candidates[1].name="Donald Trump";
    candidates[1].id=2;
    candidates[1].vote=666;
    candidates[2].name="JinPing Xi";
    candidates[2].id=3;
    candidates[2].vote=666;
    candidates[3].name="Jagmeet Singh";
    candidates[3].id=4;
    candidates[3].vote=666;
    candidates[4].name="Erin O'Toole";
    candidates[4].id=5;
    candidates[4].vote=666;




    char message_buffer[1024];
    // while loop to keep server running
    while(1)
    {
        int char_received;
        int len=sizeof(server);
        // receive msg from indir server
        char_received = recvfrom(socket_desc, (char *)message_buffer, 1024, 
                    MSG_CONFIRM, (struct sockaddr *) &server,
                    (socklen_t *)&len);
        
        message_buffer[char_received]='\0';

        // printf("%s\n",message_buffer);
        
        char response_buffer[1024];
        memset(&response_buffer, 0, sizeof(response_buffer));
        // make it lower case letters.
        for(int i=0;i<strlen(message_buffer);i++)
        {
            message_buffer[i]=tolower(message_buffer[i]);
        }
        // if indir server send request for showing candidate list
        if(strncmp(message_buffer,"show candidates",15)==0)
        {
            strcat(response_buffer,"Candidates list: \n");
            // build a string for showing candidates
            for(int i=0;i<sizeof(candidates)/sizeof(candidates[0]);i++)
            {
                strcat(response_buffer,"Name: ");
                strcat(response_buffer,candidates[i].name);
                strcat(response_buffer," ID: ");
                char temp_id[1024];
                memset(&temp_id, 0, sizeof(temp_id));
                sprintf(temp_id, "%d", candidates[i].id);   
                strcat(response_buffer,temp_id);
                strcat(response_buffer,"\n");
            }
            // if indir server want to vote
        }else if(strncmp(message_buffer,"secure voting-encryption-key",28)==0)
        {
            // let the content to be 4, which is the encryption key
            strcat(response_buffer,"4"); // this can be not hard coded later

            // if want to vote with a encrypted key from indir server
        }else if(strncmp(message_buffer,"secure voting-response",22)==0)
        {
            // now time to get the response and decrypt the key for voting.
            int start=0;
            for(int i=0;i<strlen(message_buffer);i++)
            {
                if(message_buffer[i]==':')
                {
                    start=i;
                }
            }
            char decrypt_key [1024];
            memset(&decrypt_key, 0, sizeof(decrypt_key));

            
            int j=0;
            for(int i=start+1;i<strlen(message_buffer);i++)
            { 
                if(message_buffer[i]=='\n'){
                    
                    break;
                }
                decrypt_key[j]=message_buffer[i];
                j++;
                
            }
            // got the real id that user want to vote for
            decrypt_key[j]='\0';
            printf("key received: %s\n",decrypt_key);
            // convert from string to int
            int number=atoi(decrypt_key);
            int decrypted_number=number/4;
            for(int i=0;i<sizeof(candidates)/sizeof(candidates[0]);i++)
            {
                if(decrypted_number==candidates[i].id)
                {
                    // increase the vote for that candidate
                    candidates[i].vote++;
                }
            }
            // Now votes have been updated.

            // show summary of voting
        }else if(strncmp(message_buffer,"voting summary",14)==0)
        {
            strcat(response_buffer,"Voting result list: \n");
            for(int i=0;i<sizeof(candidates)/sizeof(candidates[0]);i++)
            {
                strcat(response_buffer,"Name: ");
                strcat(response_buffer,candidates[i].name);

                strcat(response_buffer," ID: ");
                char temp_id[1024];
                memset(&temp_id, 0, sizeof(temp_id));
                sprintf(temp_id, "%d", candidates[i].id);   
                strcat(response_buffer,temp_id);

                strcat(response_buffer," Vote: ");
                char temp_vote[1024];
                memset(&temp_vote, 0, sizeof(temp_vote));
                sprintf(temp_vote, "%d", candidates[i].vote);   
                strcat(response_buffer,temp_vote);

                strcat(response_buffer,"\n");
            }

        }else
        {   // else return the word is not found in the voting server.
            strcpy(response_buffer,"The word is not found, try another one\n");
        }

        // send to the indir server
        sendto(socket_desc, (const char *)response_buffer, strlen(response_buffer),
                    MSG_CONFIRM, (const struct sockaddr *) &server, 
                    sizeof(server));
                    
        // empty the buffers
        memset(&response_buffer, 0, sizeof(response_buffer));
        memset(&message_buffer, 0, sizeof(message_buffer));

    }
    return 0;
}