///////////////////////////////////////////////////////////////////
// Student names: Alejandro Ramirez and Mark Smullen
// Course: COSC4653 - Advanced Networks
// Assignment: #8 - Two Unique Servers
// Source Code File Name: client_8.c
// Program's Purpose: Serves as the user client to connect to the
//   Monster Master servers
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
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>		// for Unix domain sockets
#include	"monster_master.h"

// Constants
#define	MAX_LINE_LENGTH	256

int socketfd;

// Function prototypes
void sig_term(int signo);
int connectTo(char *ipAddress, int portNumber);

int main(int argc, char *argv[])
{
    char inputLine[MAX_LINE_LENGTH];
    char option;
    char buffer[MAX_LINE_LENGTH];
    char *message;
    char sector_id[MAX_MESSAGE_TEXT];
    int nbrOfBytesRead;
    struct sigaction sigTermAction;
    struct sigaction sigPipeAction;
    char ipAddress[MAX_IP_ADDRESS_LENGTH];
    int portNumber = SERVER_PORT_PVE;
    bool isPvE;
    bool connectionDesired = true;
    
    // Test the number of arguments on the command line
    if (argc != 2)
	{
        fprintf(stderr, "Usage: a.out <server IP address>\n");
        exit(1);
	} // End if
    
    // Grab the server IP address
    strncpy(ipAddress, argv[1], MAX_IP_ADDRESS_LENGTH);
    
    // Register signals
    signal(SIGPIPE, SIG_IGN);
    sigTermAction.sa_handler = sig_term;
    sigemptyset(&sigTermAction.sa_mask);
    sigTermAction.sa_flags = 0;
    if(sigaction(SIGTERM, &sigTermAction, NULL) == -1)
	{
        perror("SIGTERM action\n");
	} // End if
    
    // Get the socket file descriptor of the connected socket
    socketfd = connectTo();
    
    // 1. Begin communication by sending in a username and password
    
    // 2. Sends back a login confirmation
    
    if (/*s_auto_login_invalid*/) {
        
        // 2a. Wrong password, but registered user
        if (/*bad_passwd*/) {
            perror("Your username/password did not match. Please try
                   "again.\n\n");
            exit(0);
        } // End if
        
        // 2b. Create a new account
        
        // 2c. Create account failed (probably due to this username already being taken)
        if (/*s_auth_account_create_invalid*/) {
            perror("Account creation failed. Please choose another username.");
            exit(0);
        } // End if
    } // End if
    
    
    // Infinite loop until connection is terminated
    while (connectionDesired) {
        
        if (isPvE) {
            
            
            // 3. Output the four PvE options for the client:
            //        1. Train In The Wilds!
            //        2. Show My Monster Stats!
            //        3. Migrate to PvP Server!
            //        4. Quit Monster Mash!
            
            // 4. Send the correct packet to the server based on the choice above
            
            switch (/*Selection*/) {
                case /*c_battle_init*/:
                    // 4a. Begin a battle [function?]
                    break;
                case /*c_monster_stats_req*/:
                    // 4b. Output my monster stats [function?]
                    
                    // Print out monster's stats in the following format:
                    /*
                     Monster-
                     Name: name
                     Level: level
                     Exp: experience
                     Str: value
                     Int: "
                     Def: "
                     Wis: "
                     Agi: "
                     HP: "
                     MP: "
                     
                     Moves-
                     move_name1              move_name2
                     Target: self/enemy      Target: self/enemy
                     Damage: number          Damage: number
                     Cost: cost              Cost: cost
                     Type: move_type         Type: move_type
                     
                     move_name3              move_name4
                     Target: self/enemy      Target: self/enemy
                     Damage: number          Damage: number
                     Cost: cost              Cost: cost
                     Type: move_type         Type: move_type
                     */
                    break;
                case /*c_server_swap*/:
                    // 4c. Swap to the PvP server [function?]
                    break;
                case /*c_auth_logout*/:
                    // 4d. Begin the logout process [function?]
                    //      **Don't forget the c_auth_logout_ack**
                    connectionDesired = false;
                    break;
                default:
                    // *. There was an error
                    break;
            } // End switch
            
        } // End PvE
        else {
            
            // 5. Output the four PvP options for the client:
            //        1. Fight a Stranger!
            //        2. Migrate to PvE Server!
            //        3. Quit Monster Mash!
            
            // 6. Send the correct packet to the server based on the choice above
            
            switch (/*Selection*/) {
                case /*c_battle_init*/:
                    // 6a. Begin a battle [function?]
                    break;
                case /*c_server_swap*/:
                    // 4c. Swap to the PvE server [function?]
                    break;
                case /*c_auth_logout*/:
                    // 4d. First, swap to the PvE server...
                    //      Begin the logout process [function?]
                    //      **Don't forget the c_auth_logout_ack**
                    connectionDesired = false;
                    break;
                default:
                    // *. There was an error
                    break;
            } // End switch
            
        } // End PvP
        
    } // End while
    
    // Close the socket
    close(socketfd);
    
    return 0;
} // End main

// ################################################################
int connectTo(char *ipAddress, int portNumber) {
    
    struct sockaddr_in serverAddress;
    int status;
    PacketType packetId_in;
    PacketType packetId_out;
    
    // Connect to server
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
        fprintf(stderr, "Socket error\n");
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port   = htons(portNumber);
    inet_pton(AF_INET, ipAddress, &serverAddress.sin_addr);
    status = connect(socketfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    
    // If the client program cannot establish a TCP connection
    if (status != 0)
	{
        perror("The database server is not available at this time.\n");
        exit(1);
	} // End if
    
    return socketfd;
    
} // End listenOn

// **********************************************************
void sig_term(int signo)
{
    // *. Send a c_auth_logout_ with special flag saying, "HELP ME!!!"
    close(socketfd);
    printf("\n\nSIGTERM: Connection closed\n");
    exit(1);
} // End sig_term
