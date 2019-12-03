#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues
bool fileHasBadChar(char * file);

int main(int argc, char *argv[]) {
    char * plaintext = argv[1]; // name of a file that contains the plaintext you wish to encrypt
    char * key = argv[2]; // contains the encrypt key you wish to encrypt the text
    int port = argv[3]; // port to connect with otp_end_d
    if(fileHasBadChar(plaintext) == true || fileHasBadChar(key) == true) {
        fprintf(stderr, "otp_enc error: input contains bad characters\n");
        exit(1);
    }
    if(filesLengthSame(plaintext, key) == false) {
        fprintf(stderr, "otp_enc error: input contains bad characters\n");
        exit(1);
    }
}

void setUpClient() {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char buffer[256];

    // Check usage & arg
    if (argc < 3) { 
        fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
        exit(0); 
    } 

    // Set up the server address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[2]); // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into a special form of address
    if (serverHostInfo == NULL) { 
        fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
        exit(0); 
    }
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

    // Set up the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (socketFD < 0) 
        error("CLIENT: ERROR opening socket");

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
        error("CLIENT: ERROR connecting");

    // Get return message from server
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
    
    

}

// https://stackoverflow.com/questions/39949502/q-how-to-compare-char-from-text-file-in-c
bool fileHasBadChar(char * file) {
    FILE * f;
    f = fopen(file, "r");
    char letter[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    char c_file;
    bool flag;
    
    do {
        c_file = fgetc(f);
        int i;
        for(i = 0; i < 27; i++) {
            if(c_file != letter[i]) {
                return true;
            }
        }
    } while(c_file != EOF);
    fclose(f);
    return false;
}

// https://www.geeksforgeeks.org/c-program-to-count-the-number-of-characters-in-a-file/
bool filesLengthSame(char * plaintext, char * key) {
    FILE * p = fopen(plaintext, "r");
    FILE * k = fopen(key, "r");
    int p_count = 0;
    int k_count = 0;
    char p_char;
    char k_char;
    
    if (p == NULL || k == NULL) { 
        printf("Could not open file %s", filename); 
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
    
    if(p_count == k_count) {
        return true;
    }
    return false;
}

