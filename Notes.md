# Overview
 Create five programs that encrypt and decrypt information using a one-time pad-like system.
 
#Specifications
 Only work on bash prompt in class server
 
 Plaintext: information that you wish to encrypt and protect (readable code)
 Ciphertext: plaintext after it has been encrypted by your program (non-readable code)
 Key: random sequence of characters that will be used to convert Plaintext to Ciphertext
 
 Program will encrypt and decrypt plaintext to ciphertext, using a key
 Use modulo 27 operations: your 27 characters are teh 26 capital letters and space character ().
 
 Do this by creating 5 small programs in C
 - Two will function like daemons, will be accessed using network sockets
 - Two will use the daemons to perform work
 - Last is a standalone utility
 
 Program use network calls (send(), recv(), socket(), bind(), listen(), & accept())
 Sends and receive sequences of bytes for the purposes of encryption and decryption by the appropriate daemons
 
 Specifications of the five programs:
 - otp_enc_d: 
    - program will run in the background as a daemon
    - output error if it can't be run due to a network error
    - program will listen to a port/socket assigned when it is first ran
    - When connection is made, opt_enc_d will call accept() to generate teh socket used for actual communcation, then use separate process to handle the rest of the transaction, occur on the new accepted socket
    - child process first check to make sure it is communicating with otp_enc
    - child receives from opt_enc plaintext and key via communication socket
    - child will write back the cipher text to the otp_enc process that it is connected to via the same communcation socket
    - support 5 concurrent socket connections running at the same time
 - otp_enc
    - connects to otp_enc_d
    - asks it to perform a one-time pad style encryption
    - does not do the encryption
      - opt_enc_d does
    - when opt_enc receives the ciphertext from otp_enc_d, it should output stdout
    - if receives key or plaintext files with any bad characters in them, or key file is shorter than plaintext, then it should terminate, send appropriate error text to stderr and set the exit value to 1
    - should not be able to connect to otp_dec_d
    - if rejection happens, otp_enc should report the reject to stderr, then terminate itself
    - otp_enc cannot connect to the otp_enc_d server, report the error to stderr with the attempted port and set exit value to 2
    - upon successful running and terminating, otp_enc should set the text value to 0
    - all error text must be output to stderr
 - otp_dec_d:
    - performs like otp_enc_d
    - will decrypt ciphertext given using passed in ciphertext and key
    - return plaintext to otp_dec
 - otp_dec: 
    - connect to otp_dec_d
    - ask it to decrypt ciphertext using a passed-in ciphertext and key
    - or perform exact like otp_enc and must be runnable in three ways
    - otp_dec should not be able to connect to otp_enc_d
      - program must reject each other as described in otp_enc
 - keygen: 
    - creates a key file of specified length
    - characters in the file generated will be any of the 27 allowed characters, generated using the UNIX randomization methods
    - Don't create spaces every five characters, as has been historically done
    - Note that you specifically do not have to do any fancy random number generation
    - rand() is fine
    - last character keygen outputs should be a newline. All error text must be output to stderr


Braxton notes:
- #define A 
  #if A:
    print A
  #else:
    print notA
  #endif
  
- encryption and decryption files are the same
 - except one "+" then modulizes and the other "-" then modulizes
Two clients have the same source (same file but with if statement... look at image from the image) and the two daemons have the same source (same file... look at image from the image), same file
