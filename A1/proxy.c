/* Written by Zhifan Li September 17, 2021 */
/* UCID 30089428 */
/* CPSC 441 FALL 2021 L01 T01 */
/* Assignment 01: Web Censorship Proxy */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> //for threading , link with lpthread

#define PORT 80
#define MESSAGE_LENGTH 4096

int socket_desc; // server's socket
int new_socket; // socket from browser side ( client )
int socket_proxy_server; // server

char list_of_words[1024][1024]; // list of bad words that user manually add by sending message to server socket
int number_of_key_words=0; // record the number of bad words
char pthread_client_content[MESSAGE_LENGTH]; //buffer SHARED BY THREAD

// catch any exceptions, release the resource and exit
void catcher( int sig )
  {
    close(socket_proxy_server);
    close(new_socket);
    close(socket_desc);
    exit(0);
  }

// thread function for receving key words from the client
void *connection_handler(void *socket_desc){
    
    int sock = *(int*)socket_desc;

    // DEBUGGING message
    // printf("pthread_client_content is: %s\n",pthread_client_content);
    // printf("pthread_client_content length: %lu\n",strlen(pthread_client_content));

    int done=0;
    while(!done){
        
        // if the content that got from client side is BLOCK, do these commands
        if(strncmp(pthread_client_content,"BLOCK",5)==0)
        {
            // copy the key word need to be blocked
            strncpy(list_of_words[number_of_key_words],pthread_client_content+6,strlen(pthread_client_content)-8);
            
            // printf("%lu",strlen(list_of_words[number_of_key_words]));

            list_of_words[number_of_key_words][strlen(list_of_words[number_of_key_words])]='\0';
            
            // printf("after modified, the length stored is %lu\n",strlen(list_of_words[number_of_key_words]));
            
            // Print to terminal that the key word just entered.
            printf("Word to be blocked: %s\n",list_of_words[number_of_key_words]);
            
            // Add number of key words stored
            number_of_key_words++;

            // clear the buffer, set all to 0
            memset(&pthread_client_content,0,sizeof(pthread_client_content));

            
        }else if(strncmp(pthread_client_content,"UNBLOCK",7)==0)
        {   
            // if user types UNBLOCK, it simply remove the last key word just typed from the list of key words
            // if no key word stored in the list, then do nothing, just clear the buffer
        
            if(number_of_key_words>0)
            {
                memset(&list_of_words[number_of_key_words-1],0,sizeof(list_of_words[number_of_key_words-1]));
                number_of_key_words--;
            }
            // clear the buffer
            memset(&pthread_client_content,0,sizeof(pthread_client_content));

        }else if(strncmp(pthread_client_content,"DONE",4)==0)
        {   
            // if user types DONE, then this socket process will be aborted, 
            // server will no longer receive commands from user throught this socket, 
            // if user wants to talk to server, they have to create a new socket again.

            done=1;
            memset(&pthread_client_content,0,sizeof(pthread_client_content));
            return 0;
            // this thread exits itself.
        }else
        {
            // if the content that got from client side is not BLOCK, then it could be CONNECT, or other http request, we simply ignore them

            memset(&pthread_client_content,0,sizeof(pthread_client_content));
            done=1;
            continue;
            // go to the while loop and then return 0.
        }

        
        memset(&pthread_client_content,0,sizeof(pthread_client_content));

        // receieve message from the client socket, and put into pthread_client_content buffer. size=4096
        recv(sock, pthread_client_content, MESSAGE_LENGTH,0);
    }


    return 0;
}
// end of thread function


// Main function starts here
int main(int argc, char*argv[]){
    struct sockaddr_in server; // server
    struct hostent *he; // host server (proxy to server side)

    // setup signal handler to catch unusual termination conditions
    static struct sigaction act;
    act.sa_handler = catcher;
    sigfillset(&(act.sa_mask));
    sigaction(SIGPIPE, &act, NULL);

    socket_desc=socket(AF_INET,SOCK_STREAM,0); //create server socket
    if(socket_desc==-1)
    {
        perror("Cannot create socket\n");
    }

    memset(&server, 0, sizeof(server)); // clear to 0 first
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(PORT); // set PORT=80

    //bind server socket to server
    if(bind(socket_desc,(struct sockaddr *)&server,sizeof(server))<0)
    {
        perror("Bind failed\n");
    }

    //listen for connections through TCP
    listen (socket_desc,3);

    printf("Waiting for connections...\n");

    int c=sizeof(struct sockaddr_in);
    struct sockaddr_in client;
    char client_content[MESSAGE_LENGTH]; //buffer for fetching content from client side 
    char server_reply[MESSAGE_LENGTH]; //buffer for fetching content from server side
    
    
    while(1)
    {   // big while loop keep looking for accepting sockets
        new_socket=accept(socket_desc,(struct sockaddr*)&client, (socklen_t*)&c);

        if(new_socket<0)
        {
            perror("Connection failed");
        }

        printf("New Connection accepted\n");
        

        memset(&client_content, 0, sizeof(client_content));

        // receive message from socket and put to client_content buffer
        recv(new_socket, client_content, MESSAGE_LENGTH,0);

        //print out the content of socket's message
        printf("%s\n",client_content);

        // if the message received does not start with GET http://, then it is executed here
        if(strncmp(client_content,"GET http://",11)!=0)
        {
            // FOR DEBUGGING
            // printf("Client content is %s\n",client_content);

            // create a thread
            pthread_t sniffer_thread;
		    int * new_sock = malloc(1); //allocate memory for thread
		    *new_sock = new_socket; // argument for thread func to be passed 
            strcpy(pthread_client_content,client_content); // copy the client content to shared resource for thread (global var)
            
            // Create thread and let thread run the connection_handler thread function, meanwhile main thread is still running
            if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
		    {
                // If thread cannot be created, error
			    perror("could not create thread");
			    return 1;
		    }

            // Main goes back to while loop. thread not going here
            continue;
        }

        // *********************************************************************
        // Now it is if the message is about GET http://, the program goes here.
        // *********************************************************************

        // First, get position of new line char
        char * temp=strchr(client_content,'\n');

        // printf("temp address: %li\n",temp);
        // printf("temp is pointing at **%d**\n",temp-client_content);

        // trying to parse the http request and modify it 
        char parsed_http_request[1024];

        // parsed_http_request will be the first line of client_content message, which is GET http://url HTTP/1.1
        strncpy(parsed_http_request,client_content,temp-client_content);
        
        parsed_http_request[temp-client_content-1]='\0';

        // printf("Parsed http request is: %s\n",parsed_http_request);
        // printf("Parsed http length: %i\n",strlen(parsed_http_request));
        // ************ check key word from url here ************
        
        char * pch;

        // printf ("Splitting string \"%s\" into tokens:\n",parsed_http_request);

        char check_url[1024];
        // create a copy
        strcpy(check_url,parsed_http_request);

        // split the string to tokens, when it sees the following chars
        pch = strtok (check_url," ,.-/?=");
        int found_key_word=0;
        while (pch != NULL)
        {
            // printf ("%s\n",pch);
            // printf("Number of key words are: %i\n",number_of_key_words);

            // for loop to check key words
            for(int i=0;i<number_of_key_words;i++)
            {
                // printf("List of words are: %s\n",list_of_words[i]);
                if(!strcmp(pch,list_of_words[i]))
                {
                    // search in the key word list, if see any matches, found_key_word=1, and web page will be blocked
                    found_key_word=1;
                }
            }
            pch = strtok (NULL, " ,.-/?=");

        }
        // if key word is found, then modify the url
        if(found_key_word==1)
        {
            // if found key word, then we need to change the url to direct the client to error.html page
            int start;
            int counter_start=0;
            // modifying the url here
            for(int i=strlen(parsed_http_request);i>=0;i--)
            {
                if(parsed_http_request[i]=='/')
                {
                    counter_start++;
                    if(counter_start==2)
                    {
                        start=i+1;
                    }
                }
            }
            // erasing the url from the back, to the first occurance of /
            for(int i=start;i<strlen(parsed_http_request);i++){
                parsed_http_request[i]='\0'; //reset to empty first;
            }

            // printf("modifed http error page: %s",parsed_http_request);

            // put error.html from the last / in the url for showing error page
            strcat(parsed_http_request,"error.html");

            // printf("modifed http error page: %s",parsed_http_request);

        }
        // ************* end of checking and modifying*****************
        int counter=0;
        int start;
        int end;
        for(int i=0;i<strlen(client_content);i++)
        {
            if(client_content[i]=='/')
            {
                counter++;
                if(counter==2)
                {
                    start=i+1;
                }
                if(counter==3)
                {
                    end=i;
                }
            }
        }

        char hostname[1024];

        // get hostname in the hostname string 
        strncpy(hostname,client_content+start,end-start+1);
        hostname[end-start]='\0';

        // FOR DEBUGGING
        // printf("Parsed hostname is: %s\n",hostname);
        // printf("host name length: %i\n",strlen(hostname));

        // assemble a proper http request and to send the the server
        strcat(parsed_http_request,"\r\n");
        strcat(parsed_http_request,"Host: ");
        strcat(parsed_http_request,hostname);
        strcat(parsed_http_request,"\r\n\r\n");
        printf("http request: %s\n",parsed_http_request);

        // now got the parsed http request
        // need to create a new socket and talk to the server to retreive the content
        // int socket_proxy_server;
        socket_proxy_server = socket(AF_INET , SOCK_STREAM , 0);
	
	    if (socket_proxy_server == -1)
	    {
		    printf("Could not create socket");
	    }

        struct sockaddr_in server_proxy_server;
        memset(&server_proxy_server, 0, sizeof(server_proxy_server));
        // get host's ip address by using gethostbyname to the hostent* he. then later use its attributes
        he=gethostbyname(hostname);
        struct in_addr **addr_list;

        //ip address string
        char ip[100];
        addr_list = (struct in_addr **) he->h_addr_list;
	
	    for(int i = 0; addr_list[i] != NULL; i++) 
	    {
		    //Return the first one;
		    strcpy(ip , inet_ntoa(*addr_list[0]) ); // convert to proper ipv4 ip address
            break;
		    
	    }

        // printf("ip address is: %s",ip);

        // configure the server here
        server_proxy_server.sin_addr.s_addr = inet_addr(ip); //use server's ip address
        server_proxy_server.sin_family = AF_INET;
        server_proxy_server.sin_port = htons( 80 ); // port number is 80

        //Connect to remote server
        if (connect(socket_proxy_server , (struct sockaddr *)&server_proxy_server , sizeof(server_proxy_server)) < 0)
        {
            perror("Connect error");
        }
        
        // Send request message to server.
        if( send(socket_proxy_server , parsed_http_request , strlen(parsed_http_request) , 0) < 0)
        {
            perror("Send failed");
        }
        
        int bytesReceived;// for recording the bytes it received from server

        // while loop to keep receiving message from server until there is not any left to be received
        while( (bytesReceived=recv(socket_proxy_server, server_reply , 1024 , 0)) > 0)
        {
        
            puts("Reply received\n");

            // puts(server_reply);

            // copy to buffer
            
            bcopy(server_reply,client_content,bytesReceived);

            //now show the response back to the browser. (in a while loop)
            send(new_socket, client_content, bytesReceived, 0);

            // puts(server_reply);
            memset(&client_content, 0, sizeof(client_content)); // set original buffer to empty
            memset(&server_reply, 0, sizeof(server_reply)); // set to empty
        }

        memset(&client_content, 0, sizeof(client_content)); // set original buffer to empty
        memset(&server_reply, 0, sizeof(server_reply)); // set to empty
        
        close(new_socket); // job done, release the new_socket from client
        close(socket_proxy_server); // job done, release the socket_proxy_server for server
    }

    close(socket_desc); // once job is done, release the proxy server's socket for client side communication
    
    return 0;
}
// END OF PROGRAM