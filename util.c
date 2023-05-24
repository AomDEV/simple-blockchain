#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

const char* get_args(const char* argv[], char* name) {
    int count = 0; 
    while(argv[++count] != NULL) {
        if(strcmp(argv[count], name) == 0) {
            return argv[count+1];
        }
    }
    return NULL;
}

int get_int_args(const char* argv[], char* name[]) {
    int port = 0;
    for (int i = 0; i < 2; i++) {
        const char* raw_port = get_args(argv, name[i]);
        if(raw_port != NULL) {
            sscanf(raw_port, "%d", &port);
            break;
        }
    }
    return port;
}

const char * int2str(int n) {
    char * result;
    if (n >= 0)
        result = malloc(floor(log10(n)) + 2);
    else
        result = malloc(floor(log10(n)) + 3);
    sprintf(result, "%d", n);
    return result;
}