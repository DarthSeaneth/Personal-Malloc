Sean M. Patrick (smp429) & Fulton R. Wilcox (frw14)

CS214 Systems Programming Project I: My little malloc()

Implementation Description:

The memory array is uninitialized at first, with a specific byte pattern placed in the first 4 bytes which is "1, 2, 42, 69".
This byte pattern is used as a flag to determine if memory needs to be initialized.
Upon the first call to malloc(), we check if the first 4 bytes match that pattern, and if it does then we initialize all bytes in the array to 0
Then we allocate a short integer to represent the size of the subsequent memory chunk (metadata) which takes up 2 bytes of space, 
as opposed to other integer types that take up more space
During the very first malloc() call, a short integer will be allocated in the first 2 bytes of the array which will represent the size of the subsequent memory chunk, 
which will be 4094 bytes at first.
For every subsequent memory allocation, 2 bytes will be reserved to store the metadata in addition to however many bytes the client requests.
To determine if a chunk of memory is in use or not, we will represent the 2 byte (short) metadata as positive (free) or negative (in-use). 
This strategy utilizes less space than allocating an additional byte or more, for example, to represent whether the chunk is free or not. 
This way we minimize the amount of extra memory needed for metadata simply by changing the sign of the short integer that represents it.
This also allows us to iterate through each chunk of memory like a linked list because the metadata value essentially determines where the next chunk is located.

We have modularized our mymalloc library with various helper functions and thoroughly commented the code in order to minimize code repetition and to ensure 
a user-friendly, descriptive, and readable library that can be easily interpreted.

Helper functions:
void insertMetaData(void *memPtr, short chunkSize);
    -takes void pointer to metadata in memory array and short int representing chunk size (parameters), casts the pointer to a short pointer, 
    and then inserts the short int chunk size at the address of the 2 bytes
short getChunkSize(void *memPtr);
    -takes void pointer to metadata in memory array (parameter), casts to short int pointer, then reads the 2 bytes at the address as a short int, and returns it
bool isChunkFree(void *memPtr);
    -takes void pointer to metadata in memory array (parameter), reads data at address as short int, returns true if short int is positive (chunk is free)
    or false if short int is negative (chunk is in-use)
void initializeMemory();
    -initializes all bytes in the memory array to 0 during the very first call to malloc(), and inserts appropriate metadata in the first 2 bytes
void *getNextChunk(void *memPtr);
    -takes void pointer to a chunk's metadata in memory array (parameter), then copies that address to another pointer,
    then adds the current chunk size + sizeof(short) to the pointer because that is where the next chunk's metadata will be located, then returns the pointer
bool validPointer(void *memPtr);
    -takes void pointer to some address (parameter), then checks if the address is within the bounds of the addresses of each byte of the memory array,
    returns true if it is and false otherwise
bool completePointer(void *memPtr);
    -takes void pointer to some address (parameter), then checks if the pointer is the correct address which will be the first byte of the payload of a memory chunk
    by subtracting sizeof(short) from the pointer and checking if the new pointer points to the metadata of a memory chunk 
void memError(char* file, int line, int error);
    -prints descriptive error message determined by the the integer "error" (parameter), indicating what error occurred, and in which file on which line it occurred
void printChunkSizes();
    -iterates through each chunk of the memory array, printing each short integer representation of the metadata (chunk size). Was very useful for correctness testing.

mymalloc() implementation:
  First we check if the size parameter is 0 or less, and if it is then we print the appropriate error message and return NULL because it does
  not make sense to allocate 0 or less bytes.
  Then we check if the memory array needs to be initialized or not and if so, we initialize it.
  Then we iterate through each chunk of memory until we find a free chunk that is at least as large as the size requested.
  If no such chunk was found, we print the appropriate error message and return NULL.
  Then we check if the free chunk can be divided into 2 chunks, one chunk for the requested amount of memory, and another chunk after it with the remaining memory.
  If there is not enough space to split the chunks, then we insert the appropriate metadata as a negative short int to represent that it is being used,
  and then we return a pointer to the whole chunk which will contain some extra memory. We add sizeof(short) to the pointer before returning it to ensure
  that it correctly points to the payload of the chunk and not the metadata.
  If the chunk could be split, then we insert the appropriate metadata as a negative short int to indicate it is in use, then we create a new free chunk
  after the current chunk which will contain the remaining memory. Then we add sizeof(short) to the chunk pointer to ensure it points to the payload of the chunk.

myfree() implementation:
  First we check if the pointer passed to free() contains a valid address within the memory array and if it correctly points to a chunk's metadata after subtracting
  sizeof(short) from it.
  Then we check if the metadata of the chunk is a positive value because that would mean it is a free chunk.
  If any of that is true, we print the appropriate error message and exit the function.
  If not, then we make the metadata value of the appropriate chunk positive to indicate that it is free
  Then we iterate through each chunk of the memory array checking for adjacent free chunks.
  Whenever we find adjacent free chunks, we coalesce them by adding the value of the second chunk's metadata plus sizeof(short) (to account for metadata)
  to the first chunk's metadata. We do this until we have iterated through every chunk of memory, coalescing any adjacent free chunks that we find.

Test Plan:
  In order for our library to be correct, it must have the following properties:
      (1) Correctly allocates desired amount of bytes and returns pointer to the first byte of that requested payload
      (2) Prints appropriate error messages for cases of double free, invalid/bad pointers, not enough space, and invalid size requests
      (3) Correctly handles bytes reserved for metadata of each chunk of memory, ensuring that each chunk is in its correct place and that the client cannot
      access the metadata
      (4) Does not allow a client to access the backing memory array
      (5) Correctly determines if a chunk is free or not
      (6) Correctly determines if a pointer is valid or not (allocated by mymalloc() and points to correct address)
      (7) Correctly manipulates the metadata to indicate whether a chunk is free or not
      (8) Properly frees a chunk when requested, not manipulating any other chunks in the process
      (9) Properly coalesces adjacent free chunks when possible
      (10) If there is memory remaining after malloc() requests bytes but not enough to allocate another chunk, should return entire memory chunk
      containing some extra memory to client
 Specific methods to test for each property:
      (1) Allocate several, arbitrarily sized objects and check if each byte of each object can be properly written to, and then try using the data of those objects
      in another context.
      (2) Purposely attempt to free an already freed object, free an object with a pointer to the wrong address, free an object that was not allocated with malloc(),
      request to allocate an object that is too large to fit, and attempt to allocate a 0 or less byte object.
      (3) Allocate several, arbitrarily sized objects and write some data into each memory chunk. In between each malloc() call, iterate through the memory chunks
      and print each chunk size to see if they were allocated correctly. This will show whether the metadata remains as expected and whether writing to each object
      effected the metadata or not. (white-box testing)
      (4) Attempt to reference the memory array directly in some arbitrary black-box test program
      (5) Allocate several arbitrarily sized objects and then free some at random. Then use the function that checks if a chunk is free or not, passing in the pointer
      to that chunk. This will show if the function correctly determines which chunks were free or not. (white-box testing)
      (6) Allocate an arbitrarily sized object and then add 1 to the pointer. Then attempt to free the object with the new pointer. Declare and initialize some
      arbitrary variable and then call free() with the address of that variable.
      (7) Allocate an arbitrarily sized object and print the chunk size with the pointer to that object and call the free chunk checker function. Then free the object
      and print the information again. This will show whether a chunk's metadata is negative or not, and if it is considered free or not. (white-box testing)
      (8) Allocate several arbitrarily sized objects, write data to each object, and then free one object at random. Then check to see if the data within the other
      objects have changed or not, and check the metadata of each chunk.
      (9) Allocate several arbitrarily sized objects and then print the chunk sizes of each chunk. Then randomly choose 2 adjacent chunks to free and print the
      chunk sizes again. Repeat the process until all objects have been freed. This will show the state of memory and each chunk at each step and whether the chunks
      are properly coalescing or not. (white-box testing)
      (10) Allocate objects in such a way so that the final object allocated will result in 1-2 bytes remaining in memory. Then check to see if that object contains
      those extra bytes and attempt to write and use that data.

memgrind implementation: 
  Our memgrind implementation takes no arguments
  In addition to the 3 tasks detailed in the assignment description, we have designed 2 tasks to test performance that do the following:
    task4(): Allocates all of memory in 30 byte chunks, frees all of the objects, and then requests allocation of an object larger than 30 bytes
        This task tests how efficiently the memory chunks are freed and how the freed chunks coalesce. It also tests for correctness because the freed chunks
        need to coalesce in order for the final larger object allocation to succeed.
    task5(): Randomly chooses between allocating a variable sized object or freeing an object if possible, until 64 objects have been allocated. Then the 
    remaining objects are freed.
        This task tests performance similarly to all of the other tasks, but the randomness ensures that the sizes of each object are different which can
        help prove the correctness of our library because it will reveal if the sizes of the chunks and its data remain as requested and expected.
  Our memgrind implementation runs each task 50 times, calculating the time spent on each task and printing that information out, and then finally the average
  time spent on each task is printed.

test program implementation:
  We have included an additional test program called "test" that takes no arguments and will be compiled and made executable by the same Makefile
  The program allocates 8 equally sized, large objects (509 bytes) and then writes a unique byte pattern to each object
  Then the byte pattern of each object is printed to show whether writing to one object overwrote any of the other objects, and if the byte pattern
  is exactly as expected
  Each object needs to be 509 bytes because that is the maximum possible size in order to fit 8 objects. Each object reserves 2 more bytes than requested because
  of metadata, and therefore each object will occupy 511 bytes.
  Since there are only 4094 free bytes when the memory array is initialized, we cannot allocate all of memory with 8 objects without having some space remaining
  8 objects * (509 bytes + 2 bytes) = 4088 bytes
  Each object is filled with a byte pattern according to the order in which the object was allocated (first object will contain 1's, second object will have 2's. etc)

We were able to prove the correctness of each of the properties (detailed above) of our malloc() library.
The methods that were most helpful during testing were the methods (3), (5), (8), and (9) as detailed above.
More specifically, we used our task5() from memgrind in a white-box testing setting, utilizing methods (5) and (9), to debug issues with our implementation which
resulted in the discovery and correction of various mistakes and bugs.
Additonally, our extra test program was used in a white-box testing setting, utilizing methods (3) and (8), to debug our implementation as well.
We tested our implementation thoroughly with many other variations of our programs and other test cases, but these were the testing processes that helped the most.

Design Note:
Our malloc() library implementation requires that the memory array will not exceed 32769 bytes
This is because we are representing metadata using short integers, and the maximum value they can hold is 32767
During the first call to malloc(), 2 bytes (short int) of metadata are reserved, so memory will contain 2 less bytes than the total amount of memory
Since a short cannot exceed the value 32767, a memory array of size 32769 bytes will result in inserting the value 32767
within the very first chunk's metadata which will be 2 bytes. 2 bytes can represent numbers from 0 up to the value 32767.
