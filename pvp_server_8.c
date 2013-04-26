///////////////////////////////////////////////////////////////////
// Student names: Alejandro Ramirez and Mark Smullen
// Course: COSC4653 - Advanced Networks
// Assignment: #8 - Two Unique Servers
// Source Code File Name: pve_server_8.c
// Program's Purpose: Responds to connections from multiple clients
//	concurrently by creating a child process to handle each new TCP
//	connection and deals with user versus computer interactions.
//     (Also serves as a sign-in server)
// Program's Limitations: None known
// Development Computer: Mac Book Pro
// Operating System: Mac OS X 10.8.3
// Compiler: gcc
// Operational Status: In development...
///////////////////////////////////////////////////////////////////

#include	<sys/types.h>	// basic system data types
#include	<sys/socket.h>	// basic socket definitions
#include	<sys/time.h>	// timeval{} for select()
#include	<netinet/in.h>	// sockaddr_in{} and other Internet defns
#include	<arpa/inet.h>	// inet(3) functions
#include	<errno.h>
#include	<fcntl.h>		// for nonblocking
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	// for S_xxx file mode constants
#include	<sys/uio.h>	// for iovec{} and readv/writev
#include    <search.h>      // for hash tables
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>		// for Unix domain sockets
#include	"monster_master.h"

// Methods
int listenOn(void);
void clientHandler(int connfd);
void sig_chld(int signo);
void sig_term(int signo);
void sig_pipe(int signo);

// Constants
#define	MAX_LISTEN_QUEUE_LENGTH 10
#define MAX_LINE_LENGTH 256

// Global Variables
int listenfd;
int connfd;

int main(int argc, char *argv[])
{
    
    // Initialize the variables
    int length;
    struct sockaddr_in serverAddress;
    int portNumber;
    char buffer[INET_ADDRSTRLEN];
    pid_t pid;
    struct sigaction sigChldAction;
    struct sigaction sigTermAction;
    struct sigaction sigPipeAction;
    
    // Initialize the fifo file for client handling
    if ( (mkfifo(PARENT_FIFO, (S_IRUSR | S_IWUSR) ) ) == -1) {
        perror("Error creating the fifo:\n");
        exit(1);
    }
    
    if (errno == EEXIST) {
        perror("Fifo already exists\n");
        exit(1);
    }
    
    // Create a listening socket
    listenfd = listenOn();
    if (listenfd < 0) {
        fprintf(stderr, "Bad listen file descriptor! %d\n", listenfd);
        exit(1);
    }
    
    printf("\nMonster Master PvE Server Now Running...\n
           "Ready to receive client requests!\n\n");
    
    // Set up the signal handlers*************************************
    
    // Set up SIGCHLD handler
    sigChldAction.sa_handler = sig_chld;
    sigemptyset(&sigChldAction.sa_mask);
    sigChldAction.sa_flags = 0;
    if(sigaction(SIGCHLD, &sigChldAction, NULL) == -1)
    {
		perror("SIGCHLD action\n");
    } // End SIGCHLD
    
    // Set up the SIGTERM handler
	sigTermAction.sa_handler = sig_term;
    sigemptyset(&sigTermAction.sa_mask);
    sigTermAction.sa_flags = 0;
    if(sigaction(SIGTERM, &sigTermAction, NULL) == -1)
    {
		perror("SIGTERM action\n");
    } // End SIGTERM
    
    // Set up the SIGPIPE handler
    sigPipeAction.sa_handler = sig_pipe;
    sigemptyset(&sigPipeAction.sa_mask);
    sigPipeAction.sa_flags = 0;
    if(sigaction(SIGPIPE, &sigPipeAction, NULL) == -1)
	{
        perror("SIGPIPE action\n");
	} // End if
    
    // End setting up the signal handlers*****************************
	
    for ( ; ; )
	{
		if ( (connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) < 0)
		{
			if (errno == EINTR)
			{
				continue;
			} // end EINTR handler
		} // end Accept
		
		// Create a socket to listen for clients
        getsockname(listenfd, (struct sockaddr *) &serverAddress, &length);
        
        // Make the local IP Address serializable
        inet_ntop(AF_INET, &serverAddress.sin_addr, buffer, sizeof(buffer));
        if (connfd < 0)
		{
            fprintf(stderr, "Connection error\n");
            return 1;
		} // End if
        
        // Create a child process
        pid = fork();
        if (pid == -1)
		{
			// Error in the fork()
            fprintf(stderr, "Fork error\n");
            return 1;
		} // End error creating child process
        else if (pid == 0)
		{
            
            // Close the listener socket
            close(listenfd);
            
            // Handle the client
            clientHandler(connfd);
            
            return 0;
		} // End child process code
        else {
		} // End parent code
		
        close(connfd);
	} // End for
    
    close(listenfd);
    
    return 0;
} // End main

// ################################################################
int listenOn() {
    int status;
    int listenfd;
    struct sockaddr_in servAddr;
    
    // Create a TCP socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Error opening listen socket: ");
        exit(1);
    }
    
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(SERVER_PORT_PVP);
    
    // Set up and bind the server
    status = bind(listenfd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (status < 0) {
        perror("Error binding to socket: ");
        exit(1);
    }
    
    // Make the socket listen for clients
    status = listen(listenfd, MAX_LISTEN_QUEUE_LENGTH);
    if (status < 0) {
        perror("Error listening on socket: ");
        exit(1);
    }
    
    return listenfd;
} // End listenOn

// ################################################################
void clientHandler(int connfd) {
    
    // Initialize variables
    struct sockaddr_in clientAddress;
    char ch_buffer[INET_ADDRSTRLEN];
    int nbrBytesRead;
    PacketType packetId_in;
    PacketType packetId_out;
    char inputLine[MAX_LINE_LENGTH];
    char outputLine[MAX_LINE_LENGTH];
    FILE *fileID; // Child's file () use fileID = fopen(fileName, "r");
    
    // Make the IP Address and port number serializable
    getpeername(connfd, (struct sockaddr *) &clientAddress, &length);
    inet_ntop(AF_INET, &clientAddress.sin_addr, ch_buffer, sizeof(ch_buffer));
    
    printf("Child process #%d has accepted a TCP connection\n"
           "(Foreign Node) IP Address: %s, Port: %d\n"
           "(Local Node) IP Address: %s, Port: %d\n\n", getpid(),
           buffer, ntohs(serverAddress.sin_port),
           ch_buffer, ntohs(clientAddress.sin_port));
    
    // Child enters a loop to respond to each query sent by the client
    for ( ; ; ) {
        
        // Read the buffer from the client socket
        nbrBytesRead = read(connfd, inputLine, sizeof(inputLine));
        
        // Set up the error handlers**********************************
        if (errno == ETIMEDOUT)
        {
            perror("Connection has timed out. Ending connection to client.\n");
            exit(1);
        } // End ETIMEDOUT handler
        else if (errno == ECONNRESET)
        {
            perror("Connection was reset. Ending connection to client.\n");
            exit(1);
        } // End ECONNRESET handler
        if (nbrBytesRead <= 0)
        {
            perror("Error reading from the socket. Terminating program.\n");
            exit(1);
        } // End nbrBytesRead
        // End setting up error handlers******************************
        
        sscanf(inputLine, "%d", (int *)&packetId);
        
        // Deal with the packet type from the client
        switch (packetId_in) {
                
                // From client -- request a battle
            case C_BATTLE_INIT:
                
                // Check to see if there is another user available to fight
                // Send the client a s_battle_wait
                
                if (/*Another User is Available to Fight*/) {
                    
                    // Send back a s_battle_ack
                    
                } // End if
                
                // After a certain period of time, send back a s_battle_timeout if no other users are found
                
                packetId_out = S_BATTLE_ACK;
                break;
                
                // From client -- send a battle move
            case C_BATTLE_MOVE_MESSAGE:
                if (/* Move bad for some reason? */) {
                    packetId_out = S_BATTLE_MOVE_INVALID;
                }
                
                packetId_out = S_BATTLE_MOVE_MESSAGE;
                
                // Damage calculations, move message data
                
                if (/* Battle win/lose conditions */) {
                    // Set flag to follow up with battle_terminate
                }
                
                
                break;
                
                // From client -- switch to the PvE server
            case C_SWAP_SERVER:
                if (/* Not logged in */) {
                    // Drop packet
                    break;
                }
                
                if (/* In battle */) {
                    // Drop packet
                    break;
                }
                
                packetId_out = S_SWAP_SERVER_ACK;
                // Send client PvE server address
                
                break;
            default:
                // We don't know what this is! Drop it.
                break;
        }

        // Set up the error handlers**********************************
        if (errno == EPIPE)
        {
            perror("Connection has been broken. Ending connection to client.\n");
            exit(1);
        } // End EINTR handler
        else if (errno == ENETUNREACH)
        {
            perror("Network is unreachable. Ending connection to client.\n");
            exit(1);
        } // End ENETUNREACH handler
        else if (errno == EHOSTUNREACH)
        {
            perror("Host is unreachable. Ending connection to client.\n");
            exit(1);
        } // End EHOSTUNREACH handler
        // End setting up error handlers******************************
        
    } // End infinite for loop
    
    return;
} // End clientHandler

// **********************************************************
void sig_chld(int signo)
{
	pid_t pid;
	int stat;
	
	while( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
	    printf("child %d terminated\n\n", pid);
	return;
} // End sig_chld

// **********************************************************
void sig_term(int signo)
{
	perror("SIGTERM\n\n");
	close(listenfd);
	close(connfd);
	exit(1);
} // End sig_term

// **********************************************************
void sig_pipe(int signo)
{
    perror("SIGPIPE: Connection closed\n");
    errno = EPIPE;
} // End sig_pipe
