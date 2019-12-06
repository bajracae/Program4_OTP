#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/wait.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int readSizeOfString(int establishedConnectionFD, int charsRead);
char * readBigString(int establishedConnectionFD, int charsRead, int lengthofString, char * buffer);
char * getPlaintext(int lengthofString, char * string);
char * getKey(int lengthofString, char * string);
char * encryption(int lengthofString, char * pText, char * kText);

int main(int argc, char *argv[]) 
{
    int numChild = 0; // counter for the number of children
    
    int listenSocketFD, establishedConnectionFD, portNumber, charsRead, charsWritten;
	socklen_t sizeOfClientInfo;
	char buffer[800000];
	struct sockaddr_in serverAddress, clientAddress;
    
    if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args
    
    // Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
    
    // Set up the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (listenSocketFD < 0) error("ERROR opening socket");
    
    // Enable the socket to begin listening
    if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {// Connect socket to port
        error("ERROR on binding");
    }
        
    // listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
    if(listen(listenSocketFD, 5) < 0) {
        error("ERROR on binding");
    }
    
    sizeOfClientInfo = sizeof(clientAddress);// Get the size of the address for the client that will connect
    
    // // Accept a connection, blocking if one is not available until one connects
    // establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);// Accept
    // if (establishedConnectionFD < 0) error("ERROR on accept");
    // 
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MY CODE FROM HERE ON
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int strLength = 0;
    bool isFirstRun = true;
    pid_t spawnpid = -5;
    
    while(true) {
        if(numChild >= 5) {
            break;
        }
        
        // Accept a connection, blocking if one is not available until one connects
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);// Accept
        if (establishedConnectionFD < 0) error("ERROR on accept");
    
        spawnpid = fork();
        switch(spawnpid) {            
            case -1:
                perror("Error with forking.");
                exit(1);
                break;
            case 0:
                if(isFirstRun == true) {
                    // Perform a handshake to make sure the two files are communicating with each other
                    // Send a "X" from server
                    char handshake[256];
                    memset(handshake, '\0', sizeof(handshake));
                    strcpy(handshake, "X");
                    // printf("length: %d\n", (int)strlen(handshake));
                    // fflush(stdout);
                    charsWritten = send(establishedConnectionFD, handshake, strlen(handshake), 0); // Write to the server
                    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
                    // printf("handshake: %s\n", handshake);
                    // fflush(stdout);
                    
                    // Receive a "X" from client
                	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
                	charsRead = recv(establishedConnectionFD, buffer, 255, 0);
                	if (charsRead < 0) error("ERROR writing to socket");
                    // printf("buffer in _d: %s\n", buffer);
                    // fflush(stdout);
                    
                    // Exit if the connections do not match
                    if(strcmp(buffer, handshake) != 0) {
                        // printf("buffer_d: %s\n", buffer);
                        // fflush(stdout);
                        // printf("handshake_d: %s\n", handshake);
                        // fflush(stdout);
                        fprintf(stderr, "The server could not connect with the client\n");
                        exit(2);
                    }
                    isFirstRun = false;
                }
                
                // Receive the length of the "big" string
                strLength = readSizeOfString(establishedConnectionFD, charsRead);
                printf("length of string in client: %d\n", strLength);
                fflush(stdout);

                // Receive string with plaintext, key, and delimiters in between
                // char bigString[strLength];
                char * bigString = (char *)malloc(sizeof(char) * strLength);
                bigString = readBigString(establishedConnectionFD, charsRead, strLength, buffer);
                printf("string in client: %s\n", bigString);
                fflush(stdout);
                
                // // Send confirmation to server
                // memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
                // // ERROR HERE-------------------------------------------
                // charsWritten = send(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); // Write to the server
                // // ERROR HERE-------------------------------------------
                // 
                // printf("charsWritten 1: %d\n", charsWritten);
                // fflush(stdout);
                // if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
                
                // Get plaintext from "big" string
                // char plaintext[strLength];
                char * plaintext = (char *)malloc(sizeof(char) * strLength);
                plaintext = getPlaintext(strLength, bigString);
                
                // Get key from "big" string
                // char key[strLength];
                char * key = (char *)malloc(sizeof(char) * strLength);
                key = getKey(strLength, bigString);
                
                // Do encryption
                // char encryptString[strLength];
                char * encryptString = (char *)malloc(sizeof(char) * strLength);
                encryptString = encryption(strLength, plaintext, key);
                
                // // Send encryption string to client
                printf("cihper: %s\n",encryptString );
                fflush(stdout);
                charsWritten = send(establishedConnectionFD, encryptString, strlen(encryptString), 0); // Write to the server
                if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
                printf("charsWritten 2: %d\n", charsWritten);
                fflush(stdout);
                if (charsWritten < strlen(encryptString)) printf("CLIENT: WARNING: Not all data written to socket!\n");
                fflush(stdout);
                
                // Receive confirmation
                memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
                charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
                printf("charsRead 1: %d\n", charsRead);
                fflush(stdout);
                if (charsRead < 0) error("ERROR writing to socket");
                
                // Close connection
                close(establishedConnectionFD); 
                exit(0);
                break;
            default:
                numChild++; // Increment the number of children
                if(numChild == 5) {
                    waitpid(-1, NULL, 0);
                    numChild--;
                    while(waitpid(-1, NULL, WNOHANG) > 0) numChild--;
                }
                break;
        }
    }
    return 0;
}

int readSizeOfString(int establishedConnectionFD, int charsRead) {
    int lengthofString = 0;
    
    charsRead = recv(establishedConnectionFD, &lengthofString, sizeof(int), 0);
    if (charsRead < 0) error("ERROR writing to socket");
    // if 0 then connection was cut
    printf("readSizeOfString charsRead: %d\n", charsRead);
    fflush(stdout);
    
    return lengthofString;
}

char * readBigString(int establishedConnectionFD, int charsRead, int lengthofString, char * buffer) {
    // char string[lengthofString];
    char * string = (char *)malloc(sizeof(char) * lengthofString);
    
    do {
        memset(buffer, '\0', sizeof(*buffer)); // Clear out the buffer again for reuse
        charsRead = recv(establishedConnectionFD, buffer, 1000, 0);
        printf("readBigString charsRead: %d\n", charsRead);
        fflush(stdout);
        if (charsRead < 0) error("ERROR writing to socket");
        strcat(string, buffer);
    } while(!strstr(buffer, "#"));
    return string;
}

char * getPlaintext(int lengthofString, char * string) {
    // char pText[lengthofString];
    char * pText = (char *)malloc(sizeof(char) * lengthofString);

    int j = 0;
    int i;
    for(i = 0; i < lengthofString; i++) {
        if(string[i] == '^') {
            j = i + 1;
            do {
                strncat(pText, &string[j], 1);
                j++;
            } while(string[j] != '!');
            break;
        }
    }
    
    return pText;
}

char * getKey(int lengthofString, char * string) {
    char * kText = (char *)malloc(sizeof(char) * lengthofString);
    int j = 0;
    int i;
    for(i = 0; i < lengthofString; i++) {
        // printf("c[i]: %c\n", string[i]);
        if(string[i] == '!') {
            j = i + 1;
            do {
                strncat(kText, &string[j], 1);
                j++;
            } while(string[j] != '#');
            break;
        }
    }
    
    return kText;
}

char * encryption(int lengthofString, char * pText, char * kText) {
    // char encryptText[lengthofString];
    char * encryptText = (char *)malloc(sizeof(char) * lengthofString);
    char c;
    int encrypt = 0;
    
    // printf("pText: %s\n", pText);
    // fflush(stdout);
    // printf("kText: %s\n", kText);
    // fflush(stdout);
    
    int i;
    for(i = 0; i < strlen(pText); i++) {
        if(pText[i] == ' ') {
            pText[i] = '[';
        }
        encrypt = ((pText[i] - 65) + (kText[i] - 65)) % 27;
        encrypt += 65;
        
        if(encrypt == 91) {
            encrypt = 32;
        }
        
        c = (char)encrypt;
        encryptText[i] = c;
    }
    encryptText[i] = 0;

    return encryptText;
}