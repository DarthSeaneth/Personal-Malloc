#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mymalloc.h"

void printBytePattern(char *ptr, int id){
    printf("Object %d byte pattern:\n", id);
    for(int i = 0; i < 509; i ++){
        printf(" %d ", ptr[i]);
    }
    printf("\n");
}

int main(int argc, char **argv) {
    /*
     * 3. Write a program that allocates several large objects. Once allocation is complete, it fills each
     * object with a distinct byte pattern (e.g., the first object is filled with 1, the second with 2,
     * etc.). Finally, it checks whether each object still contains the written pattern. (That is, writing
     * to one object did not overwrite any other.)
     */

    /*
     * memory will have 4094 free bytes at first
     * each call to malloc will reserve desired bytes plus 2 bytes for metadata. 
     * we can request allocation for 8 large objects of equal size = 509 bytes, and each object will reserve 511 bytes
     * 511 * 8 = 4088 -> there will be 6 bytes left over in memory after 8 objects are allocated
     */
    
    char *pointers[8];
    int i;
    for(i = 0; i < 8; i ++){
        pointers[i] = malloc(509);
    }
    for(i = 0; i < 8; i ++){
        memset(pointers[i], i+1, 509);
    }
    for(i = 0; i < 8; i ++){
        printBytePattern(pointers[i], i+1);
    }
    for(i = 0; i < 8; i ++){
        free(pointers[i]);
    }

    return EXIT_SUCCESS;
}