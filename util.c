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

const char* get_raw_args(const char* argv[], char* name[], int size) {
    for (int i = 0; i < size; i++) {
        const char* raw = get_args(argv, name[i]);
        if(raw != NULL) return raw;
    }
    return NULL;
}

int get_int_args(const char* argv[], char* name[], int size) {
    int val = 0;
    const char* raw = get_raw_args(argv, name, size);
    if(raw != NULL) {
        sscanf(raw, "%d", &val);
    }
    return val;
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