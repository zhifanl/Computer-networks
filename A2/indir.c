/* CPSC 441 A2                                        */
/*                                                    */
/* Usage:  gcc indir.c & ./a.out                     */
/*                                                    */
/* Written by Zhifan Li 30089428        SEP 28, 2021  */

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>

#define MSG_CONFIRM 0

#define SERVERPORTNUM 8888 // port is 8888
#define IPADDR "136.159.5.25" // ip address
#define INPUT_BUFFER_SIZE 4096
#define MESSAGE_LENGTH 4096


int main(int argc, char *argv[])
{
    int socket_desc;
    struct sockaddr_in server;

    /*create socket, AF_INET means ipv4 version, SOCKET_STREAM means connection via TCP, 0 means IP protocol*/
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    memset(&server, 0, sizeof(server)); // clear to 0 first

    server.sin_addr.s_addr = inet_addr(IPADDR);
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVERPORTNUM);

    // create server, bind TCP socket to the server
    if(bind(socket_desc,(struct sockaddr *)&server,sizeof(server))<0)
    {
        perror("Bind failed\n");
    }

    //listen for connections through TCP
    listen (socket_desc,3);

    printf("Waiting for connections...\n");

    int new_socket; // socket from browser side ( client )
    int c=sizeof(struct sockaddr_in);
    struct sockaddr_in client;
    char client_content[MESSAGE_LENGTH];

    // while loop to keep receiving requests
    while(1)
    {
        // accept request
        new_socket=accept(socket_desc,(struct sockaddr*)&client, (socklen_t*)&c);

        // if cannot accept the new socket
        if(new_socket<0)
        {
            // show error msg
            perror("Connection failed");
        }

        printf("New Connection accepted\n");
        
        
        // while loop to keep receiving request from that socket user.
        while(1)
        {
            // prompt user to enter command: 1,2,3,4
            char * menu="\nPlease select one of the following service: \n1. Translator Service\n2. Currency Converter Service\n3. Voting\n4. Quit\n> Enter your response: ";
            if (send(new_socket, menu, strlen(menu), 0) < 0)
            {
                puts("Send failed");
                return 1;
            }
            memset(&client_content, 0, sizeof(client_content));
            

            // receive message from socket and put to client_content buffer
            recv(new_socket, client_content, MESSAGE_LENGTH,0);
            
        
            // if input is 1, Trasnlator service 
            if (strncmp(client_content, "1", 1) == 0)
            {
                memset(&client_content, 0, sizeof(client_content));
                puts("Received Translator Service request");
                char * translator_prompt="> Enter an English word: ";
                    if (send(new_socket, translator_prompt, strlen(translator_prompt), 0) < 0)
                    {
                        puts("Send failed");
                        return 1;
                    }
                // will get the english word sent by user
                recv(new_socket, client_content, MESSAGE_LENGTH,0);
                int udp_socket_1;
                struct sockaddr_in udp_server_1;

                //create UDP socket
                udp_socket_1 = socket(AF_INET, SOCK_DGRAM, 0);

                if (udp_socket_1 == -1)
                {
                    printf("Could not create UDP socket");
                }

                memset(&udp_server_1, 0, sizeof(udp_server_1)); // clear to 0 first
                // create UDP destination server ip and port
                udp_server_1.sin_addr.s_addr = inet_addr(IPADDR);
                udp_server_1.sin_family = AF_INET;
                udp_server_1.sin_port = htons(8889);

                // send message through udp protocol
                sendto(udp_socket_1, (const char *)client_content, strlen(client_content),
                MSG_CONFIRM, (const struct sockaddr *) &udp_server_1, 
                sizeof(udp_server_1));
                
                int char_received=0;
                char response[1024];
                int size=sizeof(udp_server_1);
                
                // receive the message first, make it do not wait.
                char_received = recvfrom(udp_socket_1, (char *)response, 1024, 
                MSG_DONTWAIT, (struct sockaddr *) &udp_server_1,
                (socklen_t *)&size);

                // flag to indicate if package is lost or no
                int udp_lost=0;
                // current time
                clock_t before=clock();
                // limit time=5 second, if indir wait more than 5 seconds, package is lost
                int sec=5;
                // polling
                while(char_received<=0){
                    
                    // check time
                    clock_t now=clock();
                    // another receive request from the udp socket, see if message is received
                    char_received = recvfrom(udp_socket_1, (char *)response, 1024, 
                    MSG_DONTWAIT, (struct sockaddr *) &udp_server_1,
                    (socklen_t *)&size);
                    // if time limit exceeded
                    if(((now-before)/CLOCKS_PER_SEC)>=sec){
                        udp_lost=1;
                        break;
                        // break while loop. set flag=1, means udp package is lost
                    }
                }
                // if udp package is lost
                if(udp_lost==1){
                    char udp_fail_response[1024]="\n*** UDP Package is lost... Enter again your command... *** \n";
                    if (send(new_socket, udp_fail_response, strlen(udp_fail_response), 0) < 0)
                    {
                        puts("Send failed");
                        return 1;
                    }
                    continue; // go to top while loop
                }

                // udp package is received from server now
                response[char_received]='\0';
                // send to user (TCP)
                if (send(new_socket, response, strlen(response), 0) < 0)
                {
                    puts("Send failed");
                    return 1;
                }
                close(udp_socket_1);
                
                // go back to top of while loop
                continue;
                
            }else if (strncmp(client_content, "2", 1) == 0) // if user input is 2
            {
                memset(&client_content, 0, sizeof(client_content));
                puts("Received Currency Converter Service request");
                // currency service request from user
                // enter money
                char * amount_prompt="> Enter the amount of money: ";
                if (send(new_socket, amount_prompt, strlen(amount_prompt), 0) < 0)
                {
                    puts("Send failed");
                    return 1;
                }
                int money_digit=0;
                char money_amount[1024];
                money_digit=recv(new_socket, money_amount, MESSAGE_LENGTH,0);
                strcat(client_content,money_amount);
                client_content[money_digit]='-';
                client_content[money_digit+1]='\0';

                // enter source currency
                char * source_prompt="> Enter the source currency: ";
                // sendto the user
                if (send(new_socket, source_prompt, strlen(source_prompt), 0) < 0)
                {
                    puts("Send failed");
                    return 1;
                }
                int source_digit=0;
                char source_currency[1024];
                // receive source currency
                source_digit=recv(new_socket, source_currency, MESSAGE_LENGTH,0);
                strcat(client_content,source_currency);
                client_content[money_digit+source_digit+1]='-';
                client_content[money_digit+source_digit+2]='\0';

                // enter destination currency
                char * dest_prompt="> Enter the destination currency: ";
                if (send(new_socket, dest_prompt, strlen(dest_prompt), 0) < 0)
                {
                    puts("Send failed");
                    return 1;
                }
                int dest_digit=0;
                char dest_currency[1024];
                // receive destination currency
                dest_digit=recv(new_socket, dest_currency, MESSAGE_LENGTH,0);
                dest_currency[dest_digit]='\0';
                strcat(client_content,dest_currency);
                client_content[money_digit+source_digit+dest_digit+2]='\0';
                // parse the currency command string, put them together as one string to send to the server


                int udp_socket_2;
                struct sockaddr_in udp_server_2;

                /*create UDP socket */
                udp_socket_2 = socket(AF_INET, SOCK_DGRAM, 0);

                if (udp_socket_2 == -1)
                {
                    printf("Could not create UDP socket");
                }
                memset(&udp_server_2, 0, sizeof(udp_server_2)); // clear to 0 first

                // set UDP server ip and port
                udp_server_2.sin_addr.s_addr = inet_addr(IPADDR);
                udp_server_2.sin_family = AF_INET;
                udp_server_2.sin_port = htons(8890);
                
                // need to modift the client content here
                sendto(udp_socket_2, (const char *)client_content, strlen(client_content),
                MSG_CONFIRM, (const struct sockaddr *) &udp_server_2, 
                sizeof(udp_server_2));
                
                int char_received=0;
                char response[1024];
                int size=sizeof(udp_server_2);

                // receive from server through udp
                char_received = recvfrom(udp_socket_2, (char *)response, 1024, 
                    MSG_DONTWAIT, (struct sockaddr *) &udp_server_2,
                    (socklen_t *)&size);
                    // udp lost flag
                    int udp_lost=0;
                    clock_t before=clock();
                    // time limit is 5 seconds
                    int sec=5;
                    // see if udp package is lost within a time limit of 5
                    while(char_received<=0){
                        clock_t now=clock();
                        // polling, keep receiving
                        char_received = recvfrom(udp_socket_2, (char *)response, 1024, 
                        MSG_DONTWAIT, (struct sockaddr *) &udp_server_2,
                        (socklen_t *)&size);
                        if(((now-before)/CLOCKS_PER_SEC)>=sec){
                            udp_lost=1;
                            break;
                        }
                    }
                // if package is lost, then prompt user to enter the command again
                if(udp_lost==1){
                    char udp_fail_response[1024]="\n*** UDP Package is lost... Enter again your command... *** \n";
                    if (send(new_socket, udp_fail_response, strlen(udp_fail_response), 0) < 0)
                    {
                        puts("Send failed");
                        return 1;
                    }
                    continue; // go to top while loop
                }


                response[char_received]='\0';

                char response_with_prompt[1024];
                // create response string to send back to user
                memset(&response_with_prompt, 0, sizeof(response_with_prompt));
                strcpy(response_with_prompt, "\nCalculated currency is: ");
                strcat(response_with_prompt, response);
                strcat(response_with_prompt, " ");
                strcat(response_with_prompt, dest_currency);
                strcat(response_with_prompt, "\n");
                strcat(response_with_prompt, "\0");
                // send to the user
                printf("%li",(strlen(response_with_prompt)));
                if (send(new_socket, response_with_prompt, strlen(response_with_prompt), 0) < 0){
                    puts("Send failed");
                    return 1;
                }
                // close the socket
                close(udp_socket_2);

                continue;// go back to the while loop
                
                
            }else if (strncmp(client_content, "3", 1) == 0) // if user entered 3 for voting
            {
                // flag to show if the user has voted or no
                int voted_flag=0;
                // while loop to let user keep selecting services from voting
                // 1 is show candidate, 2 is vote, 3 is show summary, 4 is quit servcie and go back to main menu
                while(1){
                    memset(&client_content, 0, sizeof(client_content));
                    puts("Received Voting request");
                    char * voting_prompt="\nPlease select one of the following voting service: \n1. Show Candidates\n2. Secure voting\n3. Voting summary\n4. Back to main menu.\n\n> Your response is: ";
                    // send the prompt to user
                    if (send(new_socket, voting_prompt, strlen(voting_prompt), 0) < 0)
                    {
                        puts("Send failed");
                        return 1;
                    }
                    // receive the response from user
                    recv(new_socket, client_content, MESSAGE_LENGTH,0);

                    // create udp socket

                    int udp_socket_3;
                    struct sockaddr_in udp_server_3;

                    /*create socket, AF_INET means ipv4 version, SOCK_DGRAM means connection via UDP, 0 means IP protocol*/
                    udp_socket_3 = socket(AF_INET, SOCK_DGRAM, 0);

                    if (udp_socket_3 == -1)
                    {
                        printf("Could not create UDP socket");
                    }
                    memset(&udp_server_3, 0, sizeof(udp_server_3)); // clear to 0 first
                    // configure udp server
                    udp_server_3.sin_addr.s_addr = inet_addr(IPADDR);
                    udp_server_3.sin_family = AF_INET;
                    udp_server_3.sin_port = htons(8891);

                    // if user input is 1
                    if (strncmp(client_content, "1", 1) == 0)
                    {
                        strcpy(client_content,"show candidates");

                    // if user input is 2
                    }else if (strncmp(client_content, "2", 1) == 0)
                    {
                        strcpy(client_content,"secure voting-encryption-key");

                        char client_encrypt[1024]="Now fetching encryption key from server... Use the way you know to encrypt by using this value and send us the result.\n The value is: ";
                        if (send(new_socket, client_encrypt, strlen(client_encrypt), 0) < 0)
                        {
                            puts("Send failed");
                            return 1;
                        }

                        // send to the udp server the request to get the encrypted key
                        sendto(udp_socket_3, (const char *)client_content, strlen(client_content),
                        MSG_CONFIRM, (const struct sockaddr *) &udp_server_3, 
                        sizeof(udp_server_3));
                        
                        int char_received=0;
                        char response[1024];
                        memset(&response, 0, sizeof(response)); // clear to 0 first
                        

                        int size=sizeof(udp_server_3);

                        // get the key from voting server.
                        char_received = recvfrom(udp_socket_3, (char *)response, 1024, 
                        MSG_CONFIRM, (struct sockaddr *) &udp_server_3,
                        (socklen_t *)&size);

                        // int udp_lost=0;
                        // clock_t before=clock();
                        // int sec=5;
                        // while(char_received<=0){
                            
                        //     clock_t now=clock();
                        //     char_received = recvfrom(udp_socket_3, (char *)response, 1024, 
                        //     MSG_DONTWAIT, (struct sockaddr *) &udp_socket_3,
                        //     (socklen_t *)&size);
                        //     if(((now-before)/CLOCKS_PER_SEC)>=sec){
                        //         udp_lost=1;
                        //         break;
                        //     }
                        // }
                        // if(udp_lost==1){
                        //     char udp_fail_response[1024]="\n*** UDP Package is lost... Enter again your command... *** \n";
                        //     if (send(new_socket, udp_fail_response, strlen(udp_fail_response), 0) < 0)
                        //     {
                        //         puts("Send failed");
                        //         return 1;
                        //     }
                        //     continue; // go to top while loop
                        // }


                        response[char_received]='\0';
                        strcat(response,"\n> Now type your key here: "); // add a new line to show to the end user.
                        // send the response to user
                        if (send(new_socket, response, strlen(response), 0) < 0)
                        {
                            puts("Send failed");
                            return 1;
                        }
                        memset(&client_content, 0, sizeof(client_content)); // clear to 0 first

                        // the encrypted key
                        char encrypt_key[1024];

                        memset(&encrypt_key, 0, sizeof(encrypt_key)); // clear to 0 first
                        // receive response from user for the encrypted key
                        recv(new_socket, encrypt_key, MESSAGE_LENGTH,0);

                        strcpy(client_content,"secure voting-response");
                        strcat(client_content,":");
                        strcat(client_content,encrypt_key);

                        printf("send to voting server: %s\n",client_content);
                        // send to server
                        sendto(udp_socket_3, (const char *)client_content, strlen(client_content),
                        MSG_CONFIRM, (const struct sockaddr *) &udp_server_3, 
                        sizeof(udp_server_3));

                        char updated_message[1024];
                        strcpy(updated_message,"\n******* Your vote has been updated. ******* \n");

                        // send to user that the vote is updated
                        if (send(new_socket, updated_message, strlen(updated_message), 0) < 0)
                        {
                            puts("Send failed");
                            return 1;
                        }
                        voted_flag=1; // global flag for this user to show that he/she has voted.

                        close(udp_socket_3);

                        continue;
                        // go to top of loop 

                    }else if (strncmp(client_content, "3", 1) == 0)
                    {
                        // if user has not voted yet
                        if( ! voted_flag)
                        {
                            char not_voted_message[1024];
                            // prompt user to vote first, then can see the summary
                            strcpy(not_voted_message,"\n******* You have not voted, you need to vote in order to see the result. ******* \n");
                            if (send(new_socket, not_voted_message, strlen(not_voted_message), 0) < 0)
                            {
                                puts("Send failed");
                                return 1;
                            }
                            continue;
                            // go back to top of while loop

                        }
                        strcpy(client_content,"voting summary");

                    }else if (strncmp(client_content, "4", 1) == 0)
                    { // if user enters 4, then break this voting while loop, show the main menu again
                        break;
                    }
                    else
                    {
                        // any other input will be considered as wrong input
                        char error[100];
                        strcpy(error,"Wrong input, try again.\n");
                        if (send(new_socket, error, strlen(error), 0) < 0)
                        {
                            puts("Send failed");
                            return 1;
                        }
                        continue;
                        
                    }

                    
                    
                    // send the client's request to udp server
                    sendto(udp_socket_3, (const char *)client_content, strlen(client_content),
                    MSG_CONFIRM, (const struct sockaddr *) &udp_server_3, 
                    sizeof(udp_server_3));
                    
                    int char_received=0;
                    char response[1024];

                    int size=sizeof(udp_server_3);
                    char_received = recvfrom(udp_socket_3, (char *)response, 1024, 
                    MSG_DONTWAIT, (struct sockaddr *) &udp_server_3,
                    (socklen_t *)&size);


                    // UDP package monitoring, if lost, let user enter request again
                    int udp_lost=0;
                    clock_t before=clock();
                    int sec=5;
                    while(char_received<=0){
                        
                        clock_t now=clock();
                        char_received = recvfrom(udp_socket_3, (char *)response, 1024, 
                        MSG_DONTWAIT, (struct sockaddr *) &udp_server_3,
                        (socklen_t *)&size);
                        if(((now-before)/CLOCKS_PER_SEC)>=sec){
                            udp_lost=1;
                            break;
                        }
                    }
                    // if package is lost
                    if(udp_lost==1){
                        char udp_fail_response[1024]="\n*** UDP Package is lost... Enter again your command... *** \n";
                        if (send(new_socket, udp_fail_response, strlen(udp_fail_response), 0) < 0)
                        {
                            puts("Send failed");
                            return 1;
                        }
                        continue; // go to top while loop
                    }



                    response[char_received]='\0';

                    // send to user
                    if (send(new_socket, response, strlen(response), 0) < 0)
                    {
                        puts("Send failed");
                        return 1;
                    }


                    close(udp_socket_3);

                    continue;

                }
                // if user request is 4
            }else if (strncmp(client_content, "4", 1) == 0)
            {
                // close everything and break the loop
                close(new_socket);
                close(socket_desc);
                memset(&client_content, 0, sizeof(client_content));
                break;


            }else
            {
                // if user enter something else, then go to top of loop and show the menu again
                continue;
            }
        }

    memset(&client_content, 0, sizeof(client_content));

    }
    
    return 0;
    // end of indir server program
}