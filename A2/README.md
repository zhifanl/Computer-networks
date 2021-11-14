Assignment 2
Zhifan Li 
UCID: 30089428 
CPSC 441 Fall 2021 L01 T01


Service: Translator Service, Currency Service, Voting Service

Indirection Server c file: indir.c
Translator Server c file: translator.c
Currency Server c file: currency.c
Voting Server c file: voting.c

To compile:
    gcc -Wall indir.c -o indir
    gcc -Wall translator.c -o translator
    gcc -Wall currency.c -o currency
    gcc -Wall voting.c -o voting

Run the server:
    ./translator
    ./currency
    ./voting
    ./indir

indir server is using port 8888
translator server is using port 8889
currency server is using port 8890
voting server is using port 8891

To connect to the server by telnet:
    telnet 127.0.0.1 8888

Then type your response through command line

1 is translator service:
(Hello, Bye, nice to see you, what is your name, how are you)

2 is currency service: 
type the amount, source currency(CAD only), dest currency(USD,EURO, GBP, BTC)

3 is voting service 

4 is quit the telnet socket and indir server will quit too.

The testing was done both at home and at school, and it worked fine without any bugs, the servers gave valid responses.
The server supports all the required features that mentioned in the assignment description.
The indir server supports reasonable response to any invalid input from user.
The program will prompt the user if the indir server's UDP packages are not received within 5 seconds. Since the UDP protocol is not very stable, and package may get lost due to traffic. indir server will prompt the user to enter the response again.