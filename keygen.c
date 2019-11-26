#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char * argv[]){
    int len = atoi(argv[1]);
    srand(time(NULL));
    
    if(argc == 1){
        fprintf(stderr, "%s\n", "Length not specified"); 
        return;
    }
    if(len == 0){
        fprintf(stderr, "%s\n", "Length cannot be 0"); 
        return;
    }
    
    char letter[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int randomNum = 0;
    int i;
    for(i = 0; i < len; i++){
        randomNum = rand() % 27;
        fprintf(stdout, "%c", letter[randomNum]);
    }
    printf("\n");
    
    return 0;
}