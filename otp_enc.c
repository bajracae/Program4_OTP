#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdbool.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

char * fileToCharString(char * filename);
char * getBigString(char * plaintext, char * key);
bool hasBadChar(char * file);
char * readEncryptedString(int socketFD, int charsRead, int plaintextLen, char * buffer);

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[1000000];
    
	if (argc < 3) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");
        
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MY CODE FROM HERE ON
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool isFirstRun = true;
    if(isFirstRun == true) {
        // Perform a handshake to make sure the two files are communicating with each other
        // Send a "X" from server
        char handshake[256];
        strcpy(handshake, "X");
        charsWritten = send(socketFD, handshake, strlen(handshake), 0); // Write to the server
        if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
        printf("handshake: %s\n", handshake);
        fflush(stdout);        
        
        // Receive a "X" from client
        memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
        charsRead = recv(socketFD, buffer, 255, 0);
        if (charsRead < 0) error("ERROR writing to socket");
        
        // Exit if the connections do not match
        if(strcmp(buffer, handshake) != 0) {
            fprintf(stderr, "The server could not connect with the client\n");
            exit(2);
        }
        isFirstRun = false;
    }
    
    // Create the "big" string that will be sent to the server
    char * plaintext = fileToCharString(argv[1]);
    char * key = fileToCharString(argv[2]);
    
    if(strlen(plaintext) > strlen(key)) {
        fprintf(stderr, "otp_enc error: plaintext is longer than the key\n");
        exit(1);
    }
    if((hasBadChar(plaintext) == true) || (hasBadChar(key) == true)) {
        fprintf(stderr, "otp_enc error: one of the files has a bad character\n");
        exit(1);
    }
    
    char * bigString = getBigString(plaintext, key);
    
    int strLength = strlen(bigString);
    
    // Send the string length to the server
    charsWritten = send(socketFD, &strLength, sizeof(int), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    
    // Send the "big" string to the server
	charsWritten = send(socketFD, bigString, strlen(bigString), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(bigString)) printf("CLIENT: WARNING: Not all data written to socket!\n");
	fflush(stdout);
    
    // Receive a confirmation
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    
    // Receive the encrypted string from the server
    // char encryptedText[strLength];
    char * encryptedText = readEncryptedString(socketFD, charsRead, strlen(plaintext), buffer);
    
    // Send confirmation
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
    charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    
    fprintf(stdout, "%s\n", encryptedText);
    
    close(socketFD); // Close the socket
    return 0;
}

//https://riptutorial.com/c/example/8274/get-lines-from-a-file-using-getline--
char * fileToCharString(char * filename) {
    FILE * file;
    char * buf = NULL;
    size_t buf_size = 0;
    int line_count = 0;
    ssize_t line_size;
    
    file = fopen(filename, "r");
    
    if(file == NULL) {
        fprintf(stderr, "File can not be open.");
        exit(1);
    }
    
    line_size = getline(&buf, &buf_size, file);
    
    while(line_size >= 0) {
        line_count++;
        line_size = getline(&buf, &buf_size, file);
    }
    
    fclose(file);
	buf[strlen(buf)-1] = 0;
    return buf;
}

char * getBigString(char * plaintext, char * key) {
    int pLen = strlen(plaintext);
	int kLen = strlen(key);
    int totalLen = ((pLen + kLen) + 5);
    // char bString[totalLen];
    char * bString = (char *)malloc(sizeof(char) * totalLen);
    
    memset(bString, '\0', sizeof(*bString)); // Clear out the buffer again for reuse
	strcat(bString, "^");
	strcat(bString, plaintext);
	strcat(bString, "!");
	strcat(bString, key);
	strcat(bString, "#");
	
	return bString;
}

bool hasBadChar(char * file) {
    char letter[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int i, j;
    for(i = 0; i < strlen(file); i++) {
        for(j = 0; j < strlen(letter); j++) {
            if(file[i] != letter[j]) {
                return true;
            }
        }
    }
    return false;
}

char * readEncryptedString(int socketFD, int charsRead, int plaintextLen, char * buffer) {
    // char eText[plaintextLen];
    char * eText = (char *)malloc(sizeof(char) * plaintextLen);

    int i;
    for(i = 0; i < plaintextLen; i++) {
        memset(buffer, '\0', sizeof(*buffer)); // Clear out the buffer again for reuse
        charsRead = recv(socketFD, buffer, 1000, 0); // Read data from the socket, leaving \0 at end
        if (charsRead < 0) error("CLIENT: ERROR reading from socket");
        strcat(eText, buffer);
    }
    
    return eText;
}