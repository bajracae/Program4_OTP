
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

char * encryption(char * plaintext, char * key, int size);
void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int numChild = 0; // counter for the number of children
	
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, charsWritten;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	char handshake_rec[256];
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
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	// listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
	if(listen(listenSocketFD, 5) < 0) {
		error("ERROR on binding");
	}
	
	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress);// Get the size of the address for the client that will connect
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);// Accept
	if (establishedConnectionFD < 0) error("ERROR on accept");
	
    while(true) {

		if(numChild >= 5) {
			break;
		}

		pid_t spawnpid = -5;
		int j = 0;
		int size = 0;

        spawnpid = fork(); // Forks happens here
        switch (spawnpid) {
            case -1:
                perror("Error with forking.");
                exit(1);
                break;
            case 0:
				// if(numChild == 0) {
				// 	// Receive handshake message
				// 	memset(handshake_rec, '\0', sizeof(handshake_rec)); // Clear out the buffer again for reuse
				// 	charsRead = recv(establishedConnectionFD, handshake_rec, 255, 0);
				// 	if (charsRead < 0) error("ERROR writing to socket");
				// 	printf("handshake_rec: %s\n", handshake_rec);
				// 	fflush(stdout);
				// 
				// 	// Send handshake confirmation
				// 	char * handshake_sent = "E";
				// 	charsWritten = send(establishedConnectionFD, handshake_sent, strlen(handshake_sent), 0); // Write to the server
				// 	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
				// 	printf("handshake_sent: %s\n", handshake_sent);
				// 	fflush(stdout);
				// 
				// 	if(strcmp(handshake_rec, handshake_sent) != 0) {
				// 		fprintf(stderr, "The server could not connect with the client\n");
				// 		exit(2);
				// 	}
				// }
				// Receive the size of the buffer_final
				charsRead = recv(establishedConnectionFD, &size, sizeof(int), 0);
				if (charsRead < 0) error("ERROR writing to socket");
				
				printf("size: %d\n", size);
				fflush(stdout);
				
				char * enc = malloc(size + 1);
				
				memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
			
				// Read the buffer_final sent from client to enc
				do {
					charsRead = recv(establishedConnectionFD, buffer, 255, 0);
					// printf("charsRead: %d\n", charsRead);
					// fflush(stdout);
					strcat(enc, buffer);
					// printf("%s\n", buffer);
					// fflush(stdout);
				} while(!strstr(buffer, '#'));
				
				printf("IN D enc: %s\n", enc);
				fflush(stdout);
				
				char * plaintext = (char *)malloc(sizeof(char) * size);
				char * key = (char *)malloc(sizeof(char) * size);
				int i;
				int k = 0;
				int l = 0;

				for(i = 0; i < strlen(buffer); i++) {
					if(buffer[i] == '^') {
						k = i+1;
						do {
							
							strncat(plaintext, &buffer[k], 1);
							
							k++;
						} while(buffer[k] != '!');
					}
					if(buffer[i] == '!') {
						l = i+1;
						do {
							
							strncat(key, &buffer[l], 1);

							l++;
						} while(buffer[l] != '#');
					}
				}
				
				// printf("plaintext: %s\n", plaintext);
				// fflush(stdout);
				// 
				// printf("key: %s\n", key);
				// fflush(stdout);

				// Do encryption/decryption
				char * encrypted_message = encryption(plaintext, key, size);

				// Send the ciphertext to the otp_enc process with the same communcation socket	
				while(charsWritten < strlen(encrypted_message)) {
					// Send message to server
					charsWritten = send(listenSocketFD, encrypted_message, strlen(encrypted_message), 0); // Write to the server
					if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
					if (charsWritten < strlen(encrypted_message)) printf("CLIENT: WARNING: Not all data written to socket!\n");
					fflush(stdout);
				}
				
				close(establishedConnectionFD);
				exit(0);
                break;
            default:
				numChild++; // increment the number of children
                break;
        }
    }
}

char * encryption(char * plaintext, char * key, int size) {
    int encrypt = 0;
	char * message = (char*)malloc(sizeof(char) * size);
	char c;
    int i;
	for(i = 0; i < strlen(plaintext); i++) {

		if(plaintext[i] == ' ') {
			plaintext[i] = '[';
		}
		
		encrypt = ((plaintext[i] - 65) + (key[i] - 65)) % 27;

		encrypt += 65;
		
		if(encrypt == 91) {
			encrypt = 32;
		}

		c = (char)encrypt;
		
		message[i] = c;
	}
	message[i] = 0;
	// printf("MESSAGE: %s\n", message);
	// fflush(stdout);
	
	return message;
}

