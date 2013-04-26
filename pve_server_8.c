///////////////////////////////////////////////////////////////////
// Student names: Alejandro Ramirez and Mark Smullen
// Course: COSC4653 - Advanced Networks
// Assignment: #8 - Two Unique Servers
// Source Code File Name: pvp_server_8.c
// Program's Purpose: Responds to connections from multiple clients
//	concurrently by creating a child process to handle each new TCP
//	connection and deals with user versus user interactions.
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
void registerSignalHandlers(void);
void sig_chld(int signo);
void sig_term(int signo);
void sig_pipe(int signo);

// Constants
#define	MAX_LISTEN_QUEUE_LENGTH 10
#define MAX_LINE_LENGTH 256

// Global Variables
int listenfd;
int connfd;
char buffer[INET_ADDRSTRLEN];

struct sockaddr_in serverAddress;

int main(int argc, char *argv[])
{
    // Declare the variables
    int length;
    int portNumber;
    pid_t pid;
    
    // Create a listening socket
    listenfd = listenOn();
    if (listenfd < 0) {
        fprintf(stderr, "Bad listen file descriptor! %d\n", listenfd);
        exit(1);
    }
    
    printf("\nMonster Master PvP Server Now Running...\n");
    printf("Ready to receive client requests!\n\n");
    
    registerSignalHandlers();
	
    for ( ; ; )
	{
		if ( (connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) < 0 ) {
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
        if (pid == -1) {
			// Error in the fork()
            fprintf(stderr, "Fork error\n");
            return 1;
		} // End error creating child process
        else if (pid == 0) {
            // This is a child -- close the listener socket
            close(listenfd);
            
            // Handle the client
            clientHandler(connfd);
            
            return 0;
		} // End child process code
        else {
		} // End parent code
		
        close(connfd); // I'm the parent, I don't need this client file descriptor
	} // End for
    
    close(listenfd); // Clean up, nice and neat
    
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
    servAddr.sin_port = htons(SERVER_PORT_PVE);
    
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
    // Declare variables
    struct sockaddr_in clientAddress;
    char ch_buffer[INET_ADDRSTRLEN];
    int length;
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
        if (nbrBytesRead == -1) {
            // Set up the error handlers**********************************
            switch (errno) {
                case ETIMEDOUT:
                    perror("Connection timeout. Ending client connection: ");
                    break;
                    
                case ECONNRESET:
                    perror("Connection reset. Ending client connection: ");
                    break;
                    
                default:
                    perror("Miscellaneous error. Ending client connection: ");
                    break;
            }
            // End setting up error handlers******************************
        }
        
        // Check the account validity
        
        sscanf(inputLine, "%d", (int *)&packetId_in);
        
        // Deal with the packet type from the client
        switch (packetId_in) {
            // From client -- log in and authenticate to server
            case C_AUTH_LOGIN:
                if (/* Logged in already? */) {
                    // Drop/ignore packet, read again
                    break;
                }
                
                if (/* Bad Username*/) {
                    packetId_out = S_AUTH_LOGIN_INVALID;
                    // Bad username packet data
                    break;
                }
                
                if (/* Bad Password */) {
                    packetId_out = S_AUTH_LOGIN_INVALID;
                    // Bad password packet data
                    break;
                }
                
                
                packetId_out = S_AUTH_LOGIN;
                // Login OK! packet data
                
                break;
                
               // From client -- request log out
            case C_AUTH_LOGOUT:
                packetId_out = S_AUTH_LOGOUT_ACK;
                if (/* Graceful? */) {
                    // Quick logout
                    break;
                }
                
                if (/* Can't log out? */) {
                    // Examples: Currently in battle.
                    // Logout situation invalid data
                    break;
                }
                
                // Logout confirmed! Clean up
                break;
                
            // From client -- make a new account
            case C_AUTH_ACCOUNT_CREATE:
                if (/* Username taken? */) {
                    packetId_out = S_AUTH_ACCOUNT_CREATE_INVALID;
                    // Username taken data
                    break;
                }
                
                if (/* Username or password too long */) {
                    packetId_out = S_AUTH_ACCOUNT_CREATE_INVALID;
                    // Field data too long
                    break;
                }
                
                packetId_out = S_AUTH_ACCOUNT_CREATE_ACK;
                // Account creation confirmation
                break;
                
            // From client -- request a battle
            case C_BATTLE_INIT:
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
                
            // From client -- ask for current monster stats
            case C_MONSTER_STATS_REQ:
                if (/* Not logged in */) {
                    // Drop packet
                    break;
                }
                
                if (/* In battle */) {
                    // Drop packet
                    break;
                }
                
                packetId_out = S_MONSTER_STATS_ACK;
                // Fill data for monster stats
                
                break;
                
            // From client -- switch with mob just battled against
            case C_SWAP_MONSTER:
                if (/* Not logged in */) {
                    // Drop packet
                    break;
                }
                
                if (/* Still in battle */) {
                    // Drop packet
                    break;
                }
                
                packetId_out = S_SWAP_MONSTER_ACK;
                // Change player's monster
                // Fill monster swap data
                
                break;
                
            // From client -- switch to the PvP server
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
                // Send client PvP server address
                
                break;
            default:
                // We don't know what this is! Drop it.
                break;
        }
        
        dprintf(connfd, "%d", packetId_out);
        
        if (/*No account registered*/0) {
            
            // Create a new account
            
            // Check against previous accounts
            
            if (/*New account was made*/0) {
                
                // Create monster
                
                // Create a text file for the new user
                
                // Write the new information to the file
                
            } // end New Account Made
            
        } // End No Account Registered
        
        // Open the user's file
        
        // Read from this user's file
        
        // Give player his options:
        /*
         1. Fight wild monsters
         2. Connect to the PvP server
         3. Check monster stats
         4. Quit Monster Master
         */
        
        if (/*Fight Wild Monsters c_battle_init*/0) {
            
            // Set up fight
            
            // Start turn-by-turn communication s_battle_ack
            
            if (/*User Wins*/0) {
                
                // Notify user, give reward, and end battle
                
                // Give option of exchanging user's current
                //   monster to the newly defeated one? [OPTIONAL]
                
            } else if (/*User Loses*/0){
                
                // Notify user and end battle
                
            }
            
        } // End Fighting Wild Monsters
        
        if (/*Connects to PvP Server c_server_swap*/0) {
            
            // Send a s_server_swap_ack with the PvP server information
            
        } // End Connect to PvP Server
        
        if (/*Check Monster Stats c_monster_stats_req*/0) {
            
            // Read from Client's file
            
            // Send the monster struct back in s_monster_stats_ack
            
        } // End Print Monster Stats
        
        if (/*User Quits c_auth_logout*/0) {
            
            // Send a s_auth_logout
            
            // Wait for a c_auth_logout_ack **UNLESS the client terminated**
            
            // Update the user's file
            
            // Close file
            
        } // End User Quits Client

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
void registerSignalHandlers(void) {
    struct sigaction sigChldAction;
    struct sigaction sigTermAction;
    struct sigaction sigPipeAction;
    
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
}

// **********************************************************
void sig_chld(int signo)
{
	pid_t pid;
	int stat;
	
	while( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
	    printf("Child process %d terminated\n\n", pid);
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
