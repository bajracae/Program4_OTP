#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdbool.h>

char * fileToCharString(char * filename);
char * combinedFiles(char * buffer_p, char * buffer_k);
bool fileHasBadChar(char * file);
bool plainLessThanKey(char * plaintext, char * key);

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[256];
    
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
		
	// Exit the program if the file has a bad file
	// if((fileHasBadChar(argv[1]) == true) || (fileHasBadChar(argv[2]) == true)) {
    //     fprintf(stderr, "otp_enc error: input contains bad characters\n");
    //     exit(1);
    // }	
	// 
	// Check if the length to see if the chars are the same length
    // if(plainLessThanKey(argv[1], argv[2]) == false) {
    //     fprintf(stderr, "otp_enc error: input contains bad characters\n");
    //     exit(1);
    // }
	
	// // Send handshake message
	// char * handshake = "E";
	// charsWritten = send(socketFD, handshake, strlen(handshake), 0);
	// if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	// 
	// // Receive handshake confirmation
	// memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	// charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	// if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	// 
	// if(strcmp(buffer, handshake) != 0) {
	// 	fprintf(stderr,"The client and server do not match\n");
	// 	exit(2);
	// }

    // Read the file content into the buffer    
    char * buffer_p = fileToCharString(argv[1]);
    char * buffer_k = fileToCharString(argv[2]);
	char * buffer_final = combinedFiles(buffer_p, buffer_k);
	
	// Sending the length of the buffer_final
	int len = strlen(buffer_final);

	charsWritten = send(socketFD, &len, sizeof(int), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	
	// Sending the buffer_final itself
	printf("buffer_final: %s\n", buffer_final);
	charsWritten = send(socketFD, buffer_final, strlen(buffer_final), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer_final)) printf("CLIENT: WARNING: Not all data written to socket!\n");
	fflush(stdout);

	// Receive a confirmation from the server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	
	fprintf(stdout, "%s\n", buffer);

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

char * combinedFiles(char * buffer_p, char * buffer_k) {
	int size_p = strlen(buffer_p);
	int size_k = strlen(buffer_k);
	int total_size = (size_p + size_k) + 3;
	char * final_msg = (char *)malloc(sizeof(char) * total_size);
	
	memset(final_msg, '\0', sizeof(final_msg)); // Clear out the buffer again for reuse
	strcat(final_msg, "^");
	strcat(final_msg, buffer_p);
	strcat(final_msg, "!");
	strcat(final_msg, buffer_k);
	strcat(final_msg, "#");
	
	return final_msg;
}

// https://stackoverflow.com/questions/39949502/q-how-to-compare-char-from-text-file-in-c
bool fileHasBadChar(char * file) {
    FILE * f;
    f = fopen(file, "r");
    char letter[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    char c_file;
    
    do {
        c_file = fgetc(f);
        int i;
        for(i = 0; i < 27 - 1; i++) {
            if(c_file != letter[i]) {
                return true;
            }
        }
    } while(c_file != EOF);
    fclose(f);
    return false;
}

// https://www.geeksforgeeks.org/c-program-to-count-the-number-of-characters-in-a-file/
bool plainLessThanKey(char * plaintext, char * key) {
    FILE * p = fopen(plaintext, "r");
    FILE * k = fopen(key, "r");
    int p_count = 0;
    int k_count = 0;
    char p_char;
    char k_char;
    
    if (p == NULL) { 
        printf("Could not open file %s", plaintext); 
		fflush(stdout);

        return 0;
    }
	if(k == NULL) {
		printf("Could not open file %s", key); 
		fflush(stdout);

        return 0;
	}
    
    do {
        p_char = fgetc(p);   
        p_count++;     
    } while(p_char != EOF);
    
    do {
        k_char = fgetc(k);
        k_count++;
    } while(k_char != EOF);
    
    if(p_count <= k_count) {
        return true;
    }
	printf("Plaintext is bigger than key.\n"); 
	fflush(stdout);
    return false;
}

