#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void setUpServer(char * command, int numCommand, int portNumber);

int main(int argc, char *argv[]) {
    int numCommand = argc;
    char * command = argv[0];
    int listening_port = atoi(argv[1]);
    setUpServer(command, numCommand, listening_port);
    
}

void setUpServer(char * command, int numCommand, int portNumber) {
    int listenSocketFD, establishedConnectionFD, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;

    if (numCommand < 2) { 
        fprintf(stderr,"USAGE: %s port\n", command); 
        exit(1); 
    }
    
    // Set up the address struct for this process (the server)
    memset((char *)&serverAddress, '\0', sizeof(serverAddress));// Clear out the address struct
    serverAddress.sin_family = AF_INET;// Create a network-capable socket
    serverAddress.sin_port = htons(portNumber);// Store the port number
    serverAddress.sin_addr.s_addr = INADDR_ANY;// Any address is allowed for connection to this process
    
    // Set up the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);// Create the socket
    if (listenSocketFD < 0) error("ERROR opening socket");
    
    // Enable the socket to begin listening
    if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)// Connect socket to port
        error("ERROR on binding");
    listen(listenSocketFD, 5);// Flip the socket on - it can now receive up to 5 connections
    
    // set up the server
    // include a while loop that goes on forever
    
    // set up an infinite loop
        // accept, hang until one connecion comes in 
        // fork() 5 children
        
    pid_t spawnpid = -5; // Declare and initialize the variable 
    int count = 0;
    while(count < 5) {
        // Manage the child process
        // Check the number of child processes
        // Decrement the done child
        // Use waitpid here
        int pid = 0;
        int exitState = 0;
        int exitmes = 0;
        int sigmes = 0;
        while((pid = waitpid(-1, &exitState, WNOHANG)) > 0){ // While child processes exist
            if(WIFEXITED(exitState) != 0) { // If the child terminates normally
                exitmes = WEXITSTATUS(exitState);
                printf("background process %d is done. ", pid);
                fflush(stdout);
                printf("exit value %d\n", exitmes); // Print out the exit value
                fflush(stdout);
            }
            else {
                sigmes = WTERMSIG(exitState); // If the child terminates with a signal
                printf("background process %d was successfully killed. ", pid);
                fflush(stdout);
                printf("terminated by signal %d\n", sigmes); // Print out the terminating signal
                fflush(stdout);
            }
        }
        
        // Accept a connection, blocking if one is not available until one connects
        sizeOfClientInfo = sizeof(clientAddress);// Get the size of the address for the client that will connect
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);// Accept
        if (establishedConnectionFD < 0) error("ERROR on accept");
        
        spawnpid = fork(); // Forks happens here
        switch (spawnpid) {
            case -1:
                perror("Error with forking.");
                exit(1);
                break;
            case 0:
                // use a separate process to handle the rest of the transaction, which will occur on the newly accepted socket
                // make a child process to check to make sure it is communicating with otp_enc
                // While loop to read the whole data from client
                // child recieves from otp_enc plaintext and key from communcation socket
                // child will send back the ciphertext to otp_enc process that it is connected to via the same communication socket
                // use establishedConnectionFD to send and recieve data in the child process
                // close establishedConnectionFD
                
                close(establishedConnectionFD);
                break;
            default:
                // increment child process
                count++;
                break;
        }   
    }
    close(listenSocketFD);// Close the listening socketreturn 0;
}