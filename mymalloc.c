#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "mymalloc.h"

/*
 * Personal implementation of the standard library functions malloc() and free() with 
 * common error detection and reporting
 * Authors: Sean M. Patrick & Fulton R. Wilcox
 */

/*
 * Macro to determine size of memory in bytes (should not exceed 32769 bytes!)
 */
#define MEMSIZE 4096

/*
 * Memory storage array to mimic the heap
 * Specific byte pattern initialized after declaration as an initialization flag for malloc()
 */
static char memory[MEMSIZE] = {1, 2, 42, 69};

/*
 * Takes void pointer to metadata in memory
 * and inserts a short int representing the memory chunk size 
 * at the address of the 2 bytes
 */
void insertMetaData(void *memPtr, short chunkSize){
    *(short*)memPtr = chunkSize;
}

/*
 * Takes void pointer to metadata in memory
 * and reads data as a short int and returns it
 */
short getChunkSize(void *memPtr){
    return *(short*)memPtr;
}

/*
 * Takes void pointer to metadata in memory
 * and reads short int metadata
 * returns true if metadata was positive (free)
 * false if metadata was negative (in-use)
 */
bool isChunkFree(void *memPtr){
    if(getChunkSize(memPtr) > 0){
        return true;
    }
    return false;
}

/*
 * Initializes all bytes in memory to 0 during very first call to malloc()
 * and inserts appropriate metadata in first 2 bytes of memory
 */
void initializeMemory(){
    if(*memory == 1 && *(memory+1) == 2 && *(memory+2) == 42 && *(memory+3) == 69){ //using first 4 bytes to determine if the memory array has been initialized before using the memory
        memset(memory, 0, MEMSIZE);
        insertMetaData(memory, (short) (MEMSIZE - sizeof(short)));
    }
}

/*
 * Takes void pointer to some metadata location in memory
 * and calculates chunk size based on value of metadata
 * then calculates address of next chunk's metadata
 * by adding chunk size to pointer and sizeof(short) to account for metadata
 * then returns that pointer, or NULL if no more chunks exist
 */
void *getNextChunk(void *memPtr){
    short chunkSize = getChunkSize(memPtr);
    if(chunkSize == 0){
        return NULL;
    }
    if(chunkSize < 0) {
        chunkSize *= -1;
    }
    void *nextChunk = memPtr;
    nextChunk += (chunkSize + sizeof(short));
    if(((void *)memory + MEMSIZE) > nextChunk) {
        return nextChunk;
    }
    return NULL;
}

/*
 * Takes void pointer to some location in memory
 * returns true if pointer contains an address within the bounds of the memory array
 * false otherwise
 */
bool validPointer(void *memPtr){ //checks if a given pointer is within the bounds of the memory array
    if(memPtr >= ((void *)memory) && memPtr < ((void *)memory + MEMSIZE)){
        return true;
    }
    return false;
}

/*
 * Takes void pointer to some location in memory array
 * A correct pointer returned from malloc() will contain the address of the first
 * byte of the payload of the memory chunk
 * To obtain first byte of metadata of a correct pointer to a memory chunk, must
 * subtract sizeof(short) from the pointer
 * Then we iterate through each chunk of memory to check if the new pointer
 * contains the correct address of a chunk's metadata
 * returns true if it does, and false otherwise
 */
bool completePointer(void *memPtr){
    void *metaData = memory;
    while(metaData != NULL){
        if(memPtr - sizeof(short) == metaData){
            return true;
        }
        metaData = getNextChunk(metaData);
    }
    return false;
}

/*
 * Takes char pointer to file that called malloc() or free(), int representing
 * the line malloc() or free() was called, and int representing which error to display
 * Prints appropriate error message for common errors associated with malloc() and free()
 */
void memError(char* file, int line, int error) {
    //0 = not enough space, 1 = double-free, 2 = invalid pointer, 3 = bad pointer, 4 = invalid size request
    switch(error) {
        case(0): {printf("\nERROR: Not enough available space! Malloc() called in %s on line %d\n", file, line); return;}
        case(1): {printf("\nERROR: Double free! Free() called in %s on line %d\n", file, line); return;}
        case(2): {printf("\nERROR: Pointer not allocated by malloc()! Free() called in %s on line %d\n", file, line); return;}
        case(3): {printf("\nERROR: Bad pointer! Free() called in %s on line %d\n", file, line); return;}
        case(4): {printf("\nERROR: Invalid size request! Malloc() called in %s on line %d\n", file, line); return;}
    }
}

/*
 * Prints the short int metadata representing memory chunk size of each chunk of memory, separated by new lines
 * Will be negative if chunk is in-use and positive otherwise
 * Very useful for white-box correctness testing
 */
void printChunkSizes(){
    short *ptr = (short *)memory;
    while(ptr != NULL){
        printf(" |%d| ", *ptr);
        ptr = getNextChunk(ptr);
    }
    printf("\n");
}

/*
 * Takes size_t int representing amount of bytes to allocate, char pointer to
 * file that called malloc(), and int representing line that malloc() was called
 * First we check if the bytes requested were 0 or less and if so, we print
 * appropriate error message and return NULL
 * then we initialize the memory array if possible
 * then we iterate through each chunk of memory searching for free chunk
 * with at least enough space to allocate requested bytes
 * returns NULL if no such chunk was found
 * then we split up the free chunk if possible,
 * insert the appropriate metadata, and then
 * add sizeof(short) to the pointer and return that pointer so that it
 * correctly contains address of the first byte of the chunk's payload
 */
void *mymalloc(size_t size, char *file, int line){
    if(size <= 0){
        memError(file, line, 4);
        return NULL;
    }
    initializeMemory();
    void* chunkFinder = memory; //pointer to first chunk
    short currentChunkSize = 0; 
    while(chunkFinder != NULL){ //iterating through each memory chunk to find free chunk with enough space
        currentChunkSize = getChunkSize(chunkFinder);
        if(isChunkFree(chunkFinder) && (currentChunkSize >= size)){ //if current chunk is free and has atleast enough space
            break;
        }
        chunkFinder = getNextChunk(chunkFinder);
    }
    if(chunkFinder == NULL){
        memError(file, line, 0);
        return NULL;
    }
    short bytesRemaining = (currentChunkSize - sizeof(short) - size); 
    if(bytesRemaining <= 0){ //if there are only enough bytes left in memory to allocate metadata but not a new chunk
        insertMetaData(chunkFinder, -(currentChunkSize)); //negative chunk size to indicate it is in use
        return chunkFinder + sizeof(short);
    }
    //free chunk is not last chunk, therefore we need to allocate current chunk and create a new free chunk after it with the remaining amount of memory
    insertMetaData(chunkFinder, (short)-size);
    insertMetaData(getNextChunk(chunkFinder), bytesRemaining);
    return chunkFinder + sizeof(short); //need to return pointer to the payload of the chunk and not the metadata
}

/*
 * Takes void pointer to some memory address, char pointer to file that called free(),
 * and int representing line that free() was called
 * First we check if pointer passed is valid, and we print appropriate error message and exit function if so
 * Then we check if the chunk is already free, and print error message and exit function if so
 * Then we change the sign of the chunk's metadata to indicate that it is now free
 * Then we iterate through each chunk of memory, coalescing any adjacent free chunks that we find
 */
void myfree(void *ptr, char *file, int line) {
    if(!validPointer(ptr)){ //if pointer is not from malloc
        memError(file, line, 2);
        return;
    }
    if(!completePointer(ptr)){ //if pointer doesn't point to correct position
        memError(file, line, 3);
        return;
    }
    if(isChunkFree(ptr - sizeof(short))){ //if pointer is valid, need to subtract sizeof(short) to get the metadata and determine if free or not
        memError(file, line, 1);
        return;
    }
    short chunkSize = getChunkSize(ptr - sizeof(short));
    insertMetaData(ptr - sizeof(short), -(chunkSize));
    //need to check if adjacent free chunks can coalesce
    void *chunkFinder = memory;
    void *nextChunkFinder = getNextChunk(chunkFinder);
    while(chunkFinder != NULL && nextChunkFinder != NULL){ //iterating through chunks to coalesce adjacent free chunks
        if(isChunkFree(chunkFinder) && isChunkFree(nextChunkFinder)){ //if adjacent chunks are free
            insertMetaData(chunkFinder, getChunkSize(chunkFinder) + getChunkSize(nextChunkFinder) + sizeof(short)); //coalescing adjacent free chunks by combining first chunk's size with the second chunk's size plus the size of its metadata
            nextChunkFinder = getNextChunk(chunkFinder);
        }
        else{
            chunkFinder = nextChunkFinder;
            nextChunkFinder = getNextChunk(chunkFinder);
        }
    }
}