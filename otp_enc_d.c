
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

char * encryption(char * plaintext, char * key);
void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int numChild = 0; // counter for the number of children
	
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, charsWritten;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	printf("enc_d.c %d\n", portNumber);
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
	//sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
	//establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
	//if (establishedConnectionFD < 0) error("ERROR on accept");

	// // Get the message from the client and display it
	// memset(buffer, '\0', 256);
	// charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket
	// if (charsRead < 0) error("ERROR reading from socket");
	// printf("SERVER: I received this from the client: \"%s\"\n", buffer);
    // 
	// // Send a Success message back to the client
	// charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
	// if (charsRead < 0) error("ERROR writing to socket");
	// close(establishedConnectionFD); // Close the existing socket which is connected to the client
	// close(listenSocketFD); // Close the listening socket
	// return 0; 
	sizeOfClientInfo = sizeof(clientAddress);// Get the size of the address for the client that will connect

    while(true) {
		printf("1111\n");
		fflush(stdout);

		if(numChild >= 5) {
			break;
		}
		printf("2222\n");
		fflush(stdout);

		// Accept a connection, blocking if one is not available until one connects
        // Hangs automatically
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);// Accept
        if (establishedConnectionFD < 0) error("ERROR on accept");
        printf("333\n");
		fflush(stdout);
		pid_t spawnpid = -5;
		int j = 0;
		int size = 0;
		int len = 0;
		printf("hereaeijan\n");
		fflush(stdout);

        spawnpid = fork(); // Forks happens here
        switch (spawnpid) {
            case -1:
                perror("Error with forking.");
                exit(1);
                break;
            case 0:
                // Check if the otp_enc_d is communicating with opt_enc
				
				// Loop to read the whole data from client
				charsRead = recv(establishedConnectionFD, &size, sizeof(int), 0);
				
				// size = atoi(buffer);
				printf("size: %d\n", size);
				fflush(stdout);
				
				printf("charsRead1 %d\n", charsRead);
				fflush(stdout);

				charsRead = send(establishedConnectionFD, "IN DAEMON I am the server, and I got your message", 39, 0); // Send success back
				printf("charsRead2 %d\n", charsRead);
				fflush(stdout);


				if (charsRead < 0) error("ERROR writing to socket");
				
				printf("i AM here\n");
				fflush(stdout);

				char *enc = malloc(size + 1);
			
				do {
					printf("Also here\n");
					fflush(stdout);
					charsRead = recv(establishedConnectionFD, buffer, 255, 0);
					strcat(enc, buffer);
					printf("%s\n", buffer);
					fflush(stdout);
					j++;
				} while(!strstr(buffer,"#"));
				
				printf("Now I am here too\n");
				fflush(stdout);

				char * plaintext = (char *)malloc(sizeof(char) * size);
				char * key = (char *)malloc(sizeof(char) * size);
				int i;
				int k = 0;
				int l = 0;
				printf("THIS\n");
				fflush(stdout);



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
				
				printf("plaintext: %s\n", plaintext);
				fflush(stdout);

				printf("key: %s\n", key);
				fflush(stdout);

				printf("HELLOW\n");
				fflush(stdout);

				// Do encryption/decryption
				char * encrypted_message = encryption(plaintext, key);
				printf("HELLOW\n");
				fflush(stdout);

				// Send the ciphertext to the otp_enc process with the same communcation socket	
				while(charsWritten < strlen(encrypted_message)) {
					// Send message to server
					charsWritten = send(listenSocketFD, encrypted_message, strlen(encrypted_message), 0); // Write to the server
					if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
					if (charsWritten < strlen(encrypted_message)) printf("CLIENT: WARNING: Not all data written to socket!\n");
					fflush(stdout);
				}
				
                break;
            default:
				numChild++; // increment the number of children
                break;
        }
    }
}

char * encryption(char * plaintext, char * key) {
	printf("ecript1: d\n");
	fflush(stdout);

    int encrypt = 0;
	char * message = NULL;
    int i;
	for(i = 0; i < strlen(plaintext); i++) {
		printf("ecript2: d\n");
		fflush(stdout);

		encrypt = ((plaintext[i] - 65) + (key[i] - 65)) % 27;
		printf("ecript3: d\n");
				fflush(stdout);
		printf("encrypt: %d\n", encrypt);
		fflush(stdout);
		
		encrypt += 65;
		
		printf("encrypt: %d\n", encrypt);
		fflush(stdout);

		sprintf(message[i], "%d", encrypt)
		// message[i] = encrypt + '0';
		printf("MESSAGE: %s\n", message);
		fflush(stdout);
	}
	printf("ecript4: d\n");
	fflush(stdout);

	return message;
}

