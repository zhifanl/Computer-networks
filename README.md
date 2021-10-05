CPSC 441 FALL 2021 Assignment 1 
Student: Zhifan Li 
UCID: 30089428

**********Features**********

1. Function web proxy can handle HTTP GET requests between client and server.
2. Can deliver web pages in an unaltered form, unless there are key words that are to be blocked, then the web page will instead show the client the error page.
3. The file has reasonable documentations.
4. Web proxy can parse http requests and responses, and identify what is being requested, and whether to block it or no.
5. Can dynamically update the list of keywords to be blocked (using BLOCK and UNBLOCK)
6. Used multi-threading for dynamically updating keyword, when a socket for asking user keywords is created and connected, the server will still run normaly for handling the HTTP request and respones without being blocked by that socket.

**********How to configure your network settings:**********
On MacOS:
Go to settings, search proxy setting, then in the protocal conifguration, 
enable Web Proxy (HTTP), then in the web proxy configuration page: select ip to be 127.0.0.1, and Port to be 80. (Port 80 is hard-coded in the program code)

**********How to compile and run:**********

Before running the server, close all the uncessary tabs to avoid any accidental crash.

    gcc -Wall proxy.c -lpthread

    ./a.out

**********How to use it**********

FIREFOX is used as the browser for testing.

after the a.out file is executed, server is on, you can add key words to be blocked by the website by first type command: 

    telnet 127.0.0.1 80

to connect to server through terminal.
And then there are three options for you:

1. BLOCK {keyword}
2. UNBLOCK
3. DONE

that you can send through that socket to server.
"BLOCK keyword" is for blocking the word so that if server sees that keyword in the url from any http request, web proxy will block that page and instead, user will receive an /error.html page
"UNBLOCK" is for unblocking the previous word stored in the keyword list, if no word in that list, it does nothing.
"DONE" is for closing the socket, once you type DONE, server will no longer receive any message through that socket (telnet), if you still want to send message through telnet to proxy, you have to open a new terminal and type "telnet 127.0.0.1 80" again and send message.

When you use your web browser to try to access these test websites,and other bigger web pages. It should work fine, and you can see the original contents as the proxy is not running between you and the website host server.
Once you have key word stored in the proxy server for blocking certain urls, For example: SpongeBob, you will no longer have access to any url contain the word: SpongeBob. Instead, an error.html page will be shown to you.

The testing was done at both home and school. And everything works well, there is nothing that is not working properly. Note that since it is not a proxy that supports concurrent requests, it can only handle request one by one, therefore, it is recommanded that close all tabs when test the proxy. 