#include "mymalloc.h"
#include "dotmalloc.h"
#include "colorCodes.h"

#include "../../TestBib/testbib.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SOLUTION_START_ADDRESS ((void*)0xC0CAC01AADD511FE)

#define IS_NO_FREE
#define IS_NO_MERGE
#define IS_FINAL

//! global variables for ignoring free's
static int use_free  = 1;
int        use_merge = 1;

//! global variables for each mode, which should be exceeded in order for a 0 program termination
static int no_free_pass = 12 + 1; // Coding Guidelines
static int no_merge_pass = 15;
static int final_pass = 15;


static int test_count = 1;

int points_received  = 0;

enum CONSTANTS {
    MAX_OFFSETS = 32,
    OFFSET_MASK = 0xffffff
};

intptr_t offsets[MAX_OFFSETS];
unsigned int offset_counter= 0;

enum POINTS {
    TASK_1 = 12,
    TASK_2 = 11,
    TASK_3 = 6
};

enum MODE {
    NO_FREE,
    NO_MERGE,
    FINAL
};

enum TESTS_TOTAL {
    TOTAL_TESTS_NO_FREE  = 33,
    TOTAL_TESTS_NO_MERGE = 55,
    TOTAL_TESTS_FINAL    = 56
};

static enum MODE mode = NO_FREE;

static int tests_missing  = -1; //needs initialization at runtime
static int maximum_points = -1; //needs initialization at runtime
static int tests_failed   = 0;
static int tests_passed   = 0;


//-------------------------------------------------------------------------------------------------------------------//

static int __attribute__((destructor)) count_points(void) 
{
    tests_missing = tests_missing - tests_failed - tests_passed;
    if(tests_missing <0)
    {
        tests_missing = 0;
    }
    assert(maximum_points >0);
    points_received = maximum_points - tests_failed - tests_missing;
    if(points_received <0)
    {
        points_received = 0;
    }

    printf("\n");
    printf(COLOR_POINTS"============================="COLOR_RESET"\n");
    printf(COLOR_POINTS"Tests passed:  %d"COLOR_RESET"\n", tests_passed);
    printf(COLOR_POINTS"Tests failed:  %d"COLOR_RESET"\n", tests_failed);
    printf(COLOR_POINTS"Tests missing: %d"COLOR_RESET"\n", tests_missing);
    printf(COLOR_POINTS"-----------------------------"COLOR_RESET"\n");
    printf(COLOR_POINTS"==> You get: %d/%d Points"COLOR_RESET"\n",points_received, maximum_points);
    printf(COLOR_POINTS"============================="COLOR_RESET"\n");

    exercise_t *ex = addExercise("Zusammenfassung");
    addSubExercise(ex, "Memory Map", "Memory Map Vergleich", points_received, maximum_points, maximum_points);
    //Coding Guidelines
    int coding_guidelines = checkStyle("../../TestBib/checkstyle.sh", "../mymalloc.c", NULL);
    addSubExercise(ex, "Coding Guidelines", "Halte die Coding Guidelines ein", coding_guidelines, 1, 3);

    int pointsToPass = 0;
    if(mode == NO_FREE)
    {
        pointsToPass = no_free_pass;
    }
    else if(mode == NO_MERGE)
    {
        pointsToPass = no_merge_pass;
    }
    else
    {
        pointsToPass = final_pass;
    }
    exercise_score_t *score = passExercises(pointsToPass);
    
    exit(score->returnvalue);
}

//-------------------------------------------------------------------------------------------------------------------//

/** 
 * @brief add_offset computes a file offset from the given address and adds it to offsets array
 * @param address that is returned by malloc
 */
static void add_offset(void* address)
{
    assert(offset_counter < MAX_OFFSETS && "address wont fit into array");
    
    //ignore null-pointer
    if(address == NULL)
    {
        return;
    }
    
    //we only need the lower 3 byte for the offset
    intptr_t offset = (intptr_t) address & (intptr_t) OFFSET_MASK;
    
    //get the offset of the memory block
    offset = offset - sizeof(memory_block_t);
    
    //do not add an address if it already exists
    for(unsigned i=0; i<offset_counter; i++)
    {
        if((offsets[i] == offset) || (offset % sizeof(memory_block_t) != 0))
        {
            return;
        }
    }
    
    offsets[offset_counter] = offset;
    offset_counter++;
    return;
}

//-------------------------------------------------------------------------------------------------------------------//

/** 
 * @brief cleanup_opened_files_and_unmap closes open files and unmaps memory
 */
static void cleanup_opened_files_and_unmap(void)
{
    // Check whether root is a valid destination or not
    if(root)
    {
        printf(COLOR_MESSAGE"cleaning up ..."COLOR_RESET"\n\r");

        // duplicating file descriptor and size information to prevent use after free
        int    file = root->file;
        size_t size = root->size;

        // check if unmapping was sucessfully, if not give an error message
        if(munmap(root, size) == -1)
        {
            perror("munmap");
        }
        // close opened file
        close(file);

        // assign NULL to root, to prevent double unmaps and what so ever
        root = NULL;
    }
    return;
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief end_program performs cleanup operations
 * @note: this function is automatically called on process termination (enforced by _attribute__((destructor)) )
 * @note: cleanup_opened_files_and_unmap is defined in mymalloc.c
 */
void end_program(void) __attribute__((destructor));
void end_program(void)
{
    cleanup_opened_files_and_unmap();
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief signal_handler is called automatically when a signal (SIGSEGV) is send by the OS to this process
 * @param signal_number number of the signal
 * @note: cleanup_opened_files_and_unmap is defined in mymalloc.c
 */
void signal_handler(int signal_number)
{
    fprintf(stderr, "\nReceived Signal: %i\n", signal_number);
    cleanup_opened_files_and_unmap();
    exit(signal_number);
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief initialize_signal_handler registers the signal handler
 * @note this function is automatically called on startup (enforced by __attribute__((constructor)) )
 */
void initialize_signal_handler(void) __attribute__((constructor));
void initialize_signal_handler(void)
{
    printf(COLOR_MESSAGE"Registering signal handler"COLOR_RESET"\n\r");

    signal(SIGINT, signal_handler);
    signal(3, signal_handler);
    signal(SIGILL, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGFPE, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGTERM, signal_handler);

    printf(COLOR_MESSAGE"Registering signal handler sucessfully"COLOR_RESET"\n\r");
    return;
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief print_new_test_description prints predefined message to distinguish all test cases
 * @param string message that should be printed
 */
static inline void print_new_test_description(const char* string)
{
    printf(COLOR_MESSAGE"\t\t--------------------------------------------------"COLOR_RESET"\n");
    printf(COLOR_MESSAGE"\t\t"COLOR_COMMAND"%s"COLOR_RESET"\n", string);
    printf(COLOR_MESSAGE"\t\t--------------------------------------------------"COLOR_RESET"\n\n");
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief execute_malloc is a wrapper for malloc that allocates memory and sets the contents to a predefined value.
 * @param size: size of memory in bytes that should be allocated
 * @param init_value: value to write into the allocated memory
 * @param index: this will be printed as part of a debug message
 * @return pointer to allocated memory
 */
static inline void* execute_malloc(size_t size, int init_value, int index, void* expected_return)
{
    void* allocated_memory = NULL;
    // print what is shall be performed, perform allocation and print its result
    printf(COLOR_MESSAGE"[%02d]Executing: allocated_memory[%i]=malloc(%4zu)..."COLOR_RESET, test_count++, index, size);
    allocated_memory = (unsigned char*)my_malloc(size);
    
    add_offset(expected_return);
    
    // check whether memory allocation was sucessfully or not
    if(allocated_memory == expected_return)
    {
        if(allocated_memory == NULL && errno != ENOMEM)
        {
             printf(COLOR_MESSAGE"\t... allocated to: "COLOR_ERROR"%10p (ERROR: expected %s)"COLOR_RESET"\n\r", 
                    allocated_memory, strerror(ENOMEM));
             tests_failed++;
        }
        else
        {
            printf(COLOR_MESSAGE"\t... allocated to: "COLOR_SUCCESS"%10p (SUCCESS)"COLOR_RESET"\n\r", allocated_memory);
            tests_passed++;
        }
        // Set memory to a predefined value if memory was allocated...
        if(allocated_memory)
        {
            memset(allocated_memory, init_value, size);
        }
        return allocated_memory;
    }
    else
    {
        printf(COLOR_MESSAGE"\t... allocated to: "COLOR_ERROR"%10p (ERROR: expected %p)"COLOR_RESET"\n\r", 
               allocated_memory, expected_return);
        tests_failed++;
        return allocated_memory;
    }
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief execute_free is a wrapper around free with some debug output
 * @note Used inside test functions
 * @param location pointer to memory that should be freed
 */
static inline void execute_free(void* location)
{
    if(!use_free)
    {
        printf(COLOR_WARNING"[WARNING] skipping free(%p);"COLOR_RESET"\n\r", location);
        return;
    }
    else
    {
        printf(COLOR_MESSAGE"[%02d]Executing: free(%10p);..."COLOR_RESET"\t", test_count++, location);
        my_free(location);
        printf(COLOR_SUCCESS"...SUCCESS"COLOR_RESET"\n\r");
        tests_passed++;
        
        //add_offset(location);
        
        return;
    }
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief file_creation_test tests if the file is created correctly
 */
static inline void file_creation_test(void)
{
    
    print_new_test_description("testing creation of the memory map file");

    //preparation: delete memory map file if it exists
    if( access( FILE_NAME, F_OK ) != -1 ) {
        if(remove(FILE_NAME) == -1)
        {
            perror(COLOR_ERROR"failed to remove memory map file"COLOR_RESET);
            exit(EXIT_FAILURE);
        }
        
    } 
    
    
    struct stat status_buffer;
    int file_descriptor = open_file();
    
    if(file_descriptor >= 0) 
    {
        //!test if file exists
        printf(COLOR_MESSAGE"[%2d]Testing File Existence:"COLOR_RESET"\n", test_count++);
        if( access(FILE_NAME, F_OK ) != -1 ) 
        {
            printf(COLOR_SUCCESS"\t[SUCCESS] memory map file exists"COLOR_RESET"\n");
            tests_passed++;
        } 
        else 
        {
            printf(COLOR_ERROR"\t[ERROR] memory map file does not exist => Abort all other tests"COLOR_RESET"\n");
            exit(EXIT_FAILURE);
            tests_failed++;
        }
        
        //!test permissions
        printf(COLOR_MESSAGE"[%2d]Testing File Permissions:"COLOR_RESET"\n", test_count++);
        if(stat(FILE_NAME, &status_buffer) < 0)
        {
            perror("file_creation_test reading file status");
            printf(COLOR_ERROR"[ERROR] Cannot read status of memory map file => aborting all other tests"COLOR_RESET"\n");
            exit(EXIT_FAILURE);
            tests_failed++;
        }
        mode_t mode = status_buffer.st_mode;
        if( ((mode & (S_IWUSR | S_IRUSR)) == (S_IWUSR | S_IRUSR)) &&
            ((mode & (S_IXUSR | S_IXGRP | S_IXOTH | S_IWGRP | S_IWOTH | S_IRGRP | S_IROTH)) == 0) )
        {
            printf(COLOR_SUCCESS"[SUCCESS] memory map file permissions are correct"COLOR_RESET"\n");
            tests_passed++;
        }
        else
        {
             printf(COLOR_ERROR"[ERROR] memory map file permissions are wrong (%o; expected: %o)"COLOR_RESET"\n", 
                    mode & ACCESSPERMS, (S_IWUSR | S_IRUSR));
             tests_failed++;
        }
        
        //!test size
        printf(COLOR_MESSAGE"[%2d]Testing File Size:"COLOR_RESET"\n", test_count++);
        if(status_buffer.st_size == (long)get_page_size())
        {
            printf(COLOR_SUCCESS"[SUCCESS] memory map file size is correct"COLOR_RESET"\n");
            tests_passed++;
        }
        else
        {
             printf(COLOR_ERROR"[ERROR] memory map file size is wrong (%zu expected: %zu)"COLOR_RESET"\n", 
                    status_buffer.st_size, get_page_size());
             tests_failed++;
        }

        //and close file again
        if(close(file_descriptor) < 0)
        {
            perror("closing file in file_creation_test");
            exit(EXIT_FAILURE);
        }
        printf(COLOR_MESSAGE"\t\t ... done"COLOR_RESET"\n\n\r");
    }
    else
    {
        printf(COLOR_ERROR"[ERROR] open_file does not return a valid file descriptor"COLOR_RESET"\n");
        exit(EXIT_FAILURE);
    }
    
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief simple_allocation_and_free_test Tests malloc and free implementation with a 3 input parameters.
 */
static inline void simple_allocation_and_free_test(void)
{
    // predefined allocation sizes, initialization values and array for storing allocated memory adresses
    size_t allocation_sizes[]  = {32, 64, 128};
    int    memory_init_array[] = {0x11, 0x22, 0x33};
    void*  allocated_memory[]  = {NULL, NULL, NULL};
    void*  expected_results[]   = {(void*)0x10000030, (void*)0x10000078, (void*)0x100000d8};
    
    // print message:
    print_new_test_description("testing simple memory allocation");

    // execute mallocs
    for(size_t i = 0; i < sizeof(allocated_memory) / sizeof(allocated_memory[0]); ++i)
    {
        allocated_memory[i] = execute_malloc(allocation_sizes[i], memory_init_array[i], i, expected_results[i]);
    }
    printf("\n\r");
    // execute free
    for(size_t i = 0; i < sizeof(allocation_sizes) / sizeof(allocated_memory[0]); ++i)
    {
        execute_free(allocated_memory[i]);
    }

    printf(COLOR_MESSAGE"\t\t ... done"COLOR_RESET"\n\n\r");
    return;
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief simple_reallocation_and_free_test: mostly like simple_allocation_and_free_test
 *        but memory allocation and freeing happens in different orders.
 * @note: should be called after simple_allocation_and_free_test
 */
static inline void simple_reallocation_and_free_test(void)
{
    // predefined allocation sizes, initialization values and array for storing allocated memory adresses
    size_t allocation_sizes[]  = {128, 64, 256};
    int    memory_init_array[] = {0x44, 0x55, 0x66};
    void*  allocated_memory[]  = {NULL, NULL, NULL};
    void*  no_free_results[]   = {(void*)0x10000180, (void*)0x10000228, (void*)0x10000288};
    void*  no_merge_results[]  = {(void*)0x100000d8, (void*)0x10000078, (void*)0x10000180};
    void*  final_results[]     = {(void*)0x10000030, (void*)0x100000d8, (void*)0x10000138};
    void** expected_results;

    if(mode == NO_FREE)
    {
       expected_results = no_free_results;
    }
    else if(mode == NO_MERGE)
    {
        expected_results = no_merge_results;
    }
    else
    {
        expected_results = final_results;
    }

    // print message:
    print_new_test_description("testing simple memory reallocation");

    // execute mallocs
    for(size_t i = 0; i < sizeof(allocated_memory) / sizeof(allocated_memory[0]); ++i)
    {
        allocated_memory[i] = execute_malloc(allocation_sizes[i], memory_init_array[i], i, expected_results[i]);
    }

    execute_free(allocated_memory[0]);
    execute_free(allocated_memory[2]);
    execute_free(allocated_memory[1]);


    printf(COLOR_MESSAGE"\t\t ... done"COLOR_RESET"\n\n\r");
    return;
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief mixed_allocation_and_free_test tests allocation and freeing in a mixed order
 */
static inline void mixed_allocation_and_free_test(void)
{
    size_t allocation_sizes[]  = {3, 99, 500, 300, 400, 1};
    int    memory_init_array[] = {0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};
    void*  allocated_memory[]  = {NULL, NULL, NULL, NULL, NULL, NULL};
    void*  no_free_results[]   = {(void*)0x100003a8, (void*)0x100003d8, (void*)0x10000468, 
                                  (void*)0x10000678, (void*)0x100007c8, (void*)0x100009d8,
                                  (void*)0x10000b88};
    void*  no_merge_results[]  = {(void*)0x10000030, (void*)0x100000d8, (void*)0x100002a0, 
                                  (void*)0x100002a0, (void*)0x100004b0, (void*)0x100006c0,
                                  (void*)0x10000078};
    void*  final_results[]     = {(void*)0x10000030, (void*)0x10000060, (void*)0x100000f0, 
                                  (void*)0x10000060, (void*)0x100001b0, (void*)0x100003c0,
                                  (void*)0x10000060};
    void** expected_results;
    if(mode == NO_FREE)
    {
       expected_results = no_free_results;
    }
    else if(mode == NO_MERGE)
    {
        expected_results = no_merge_results;
    }
    else
    {
        expected_results = final_results;
    }
    
    // print message:
    print_new_test_description("mixing simple memory free's and allocations in series");

// execute test
#define MEMORY_ALLOCATION(X,Y) allocated_memory[X] = execute_malloc(allocation_sizes[X], memory_init_array[X], X, Y);
    MEMORY_ALLOCATION(0,expected_results[0]);
    MEMORY_ALLOCATION(1,expected_results[1]);
    MEMORY_ALLOCATION(2,expected_results[2]);
    execute_free(allocated_memory[2]);
    execute_free(allocated_memory[1]);
    MEMORY_ALLOCATION(3,expected_results[3]);
    MEMORY_ALLOCATION(2,expected_results[4]);
    execute_free(allocated_memory[3]);
    MEMORY_ALLOCATION(4,expected_results[5]);
    MEMORY_ALLOCATION(5,expected_results[6]);

    execute_free(allocated_memory[0]);
    execute_free(allocated_memory[5]);
    execute_free(allocated_memory[4]);
    execute_free(allocated_memory[2]);
#undef MEMORY_ALLOCATION
    printf(COLOR_MESSAGE"\t\t ... done"COLOR_RESET"\n\n\r");
    return;
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief failure_handling_test checks if error handling works properly
 */
static inline void failure_handling_test(void)
{
    void*  allocated_memory   = NULL;
    size_t sizes[]            = {4096 - sizeof(memory_block_t) - sizeof(chunk_t) + 1, 
                               4000,
                               79*sizeof(memory_block_t)};
    void*  no_free_results[]  = {NULL, NULL,              NULL};
    void*  no_merge_results[] = {NULL, NULL,              (void*)0x10000870};
    void*  final_results[]    = {NULL, (void*)0x10000030, NULL};
    void** expected_results;
    if(mode == NO_FREE)
    {
       expected_results = no_free_results;
    }
    else if (mode == NO_MERGE)
    {
        expected_results = no_merge_results;
    }
    else
    {
        expected_results = final_results;
    }
    
    
    print_new_test_description("testing error handling in malloc and free");

    for(int i=0; i<3 && allocated_memory==NULL; i++)
    {
        allocated_memory = execute_malloc(sizes[i],i,0x00,expected_results[i]);
    }

    if(use_free)
    {
        if(allocated_memory == NULL)
        {
            fprintf(stderr, "\n\n"COLOR_ERROR"ERROR: Memory has not been allocated"COLOR_RESET"\n\n");
            tests_failed++;
            return;
        }
        printf(COLOR_MESSAGE"following free should fail:\t"COLOR_RESET);
        execute_free((char*)allocated_memory + 3);
        printf(COLOR_MESSAGE"this one should work:\t\t"COLOR_RESET);
        execute_free(allocated_memory);
        printf(COLOR_MESSAGE"checking double free:\t\t"COLOR_RESET);
        execute_free(allocated_memory); // should fail...
        printf(COLOR_MESSAGE"check freeing nullpointer:\t"COLOR_RESET);
        execute_free(NULL);
        printf("\t\t"COLOR_MESSAGE" ... done"COLOR_RESET"\n\n\r");
    }
    else
    {
        printf(""COLOR_WARNING"[warning] skipping error handling for \"free()\""COLOR_RESET"\n\n\r");
    }
    return;
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief advanced_reallocation_test tests merging of free memory
 */
static inline void advanced_reallocation_test(void)
{
    size_t allocation_sizes[]               = {128, 64, 256};
    int    memory_init_array[]              = {0xDD, 0xEE, 0xFF};
    void*  allocated_memory[]               = {NULL, NULL, NULL};
    void*  reallocated_memory[]             = {NULL, NULL, NULL};
    void*  expected_allocated_results[]     = {(void*)0x10000030, (void*)0x100000d8, (void*)0x10000138};
    void*  expected_reallocated_results[]   = {(void*)0x10000030, (void*)0x100000d8, (void*)0x10000138};
    
    
    
    // print message:
    print_new_test_description("testing merging of allocated memory");

    if(mode != FINAL)
    {
        printf(COLOR_WARNING"[WARNING] skipping this test"COLOR_RESET"\n\r");
        return;
    }
    // execute mallocs
    for(size_t i = 0; i < sizeof(allocated_memory) / sizeof(allocated_memory[0]); ++i)
    {
        allocated_memory[i] = execute_malloc(allocation_sizes[i], memory_init_array[i], i, 
                                             expected_allocated_results[i]);
    }
    // release memory and merge space
    for(size_t i = 0; i < sizeof(allocated_memory) / sizeof(allocated_memory[0]); ++i)
    {
        execute_free(allocated_memory[i]);
    }
    // reallocate memory
    for(size_t i = 0; i < sizeof(allocated_memory) / sizeof(allocated_memory[0]); ++i)
    {
        reallocated_memory[i] = execute_malloc(allocation_sizes[i], memory_init_array[i], i, 
                                               expected_reallocated_results[i]);
    }
    // release memory again
    for(size_t i = 0; i < sizeof(allocated_memory) / sizeof(allocated_memory[0]); ++i)
    {
        execute_free(reallocated_memory[i]);
    }

    printf("\t\t"COLOR_MESSAGE" ... done"COLOR_RESET"\n\n\r");
    return;
}

//-------------------------------------------------------------------------------------------------------------------//

void meta_data_test(void)
{
    print_new_test_description("testing memory management structures");
    add_offset(root);
    
    //iterate over free list
    for(memory_block_t* free_space = root->free_space_list; 
        free_space != NULL; 
        free_space = free_space->next)
    {
        add_offset(free_space);
    }
    
    char * solution_memory_dump;
    
    if(mode == NO_FREE) 
    {
        solution_memory_dump = "memory_map_no_free.bin";
    }
    else if (mode == NO_MERGE)
    {
        solution_memory_dump = "memory_map_no_merge.bin";
    }
    else
    {
        solution_memory_dump = "memory_map_final.bin";
    }
    
    int solution_file = open(solution_memory_dump, O_RDONLY);
    //Error handling
    if(solution_file <0)
    {
        perror(COLOR_ERROR"ERROR: failed opening solution file"COLOR_RESET);
        fprintf(stderr, COLOR_ERROR"Aborting all remaining tests"COLOR_RESET"\n");
        exit(EXIT_FAILURE);
    }
    
    void* solution = mmap(SOLUTION_START_ADDRESS, get_page_size(), PROT_READ, MAP_FILE | MAP_PRIVATE, solution_file, 0);
    //Error handling
    if(solution == NULL)
    {
        perror(COLOR_ERROR"ERROR: failed mapping solution file"COLOR_RESET);
        fprintf(stderr, COLOR_ERROR"Aborting all remaining tests"COLOR_RESET"\n");
        exit(EXIT_FAILURE);
    }
    //this case shouldn't happen but who knows :-)
    else if (solution != SOLUTION_START_ADDRESS)
    {
        printf(COLOR_WARNING"WARNING: solution was mapped at %p that is not expected"COLOR_RESET"\n",
               solution);
    }
    
    //Compare memory
    size_t errors = 0;
    for(unsigned i=0; i<offset_counter; i++)
    {
        printf(COLOR_MESSAGE"[%02d]testing memory @ %06lx ...\t"COLOR_RESET,test_count++, offsets[i]);
        
        intptr_t managed_memory  = (intptr_t)START_ADDRESS+offsets[i];
        intptr_t solution_memory = (intptr_t)solution+offsets[i];
        
        int result = memcmp((void*) managed_memory, 
                            (void*) solution_memory, 
                            sizeof(memory_block_t));
        
        //print result
        if(result == 0) 
        {
            printf(COLOR_SUCCESS"\t...SUCCESS"COLOR_RESET"\n");
            tests_passed++;
        }
        else
        {
            errors++;
            printf(COLOR_ERROR"\t...ERROR"COLOR_RESET"\n");
            tests_failed++;
        }
    }
    
    //print message if errors were found
    if(errors >0)
    {
        printf(COLOR_ERROR"please use run_compare.bash to compare your memory footprint with the solution"COLOR_RESET"\n");
    }
    
    
    printf("\t\t"COLOR_MESSAGE" ... done"COLOR_RESET"\n\n\r");
    return;
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief handle_input_error prints a usage message for this program
 */
static void handle_input_error()
{
    printf(COLOR_ERROR"[ERROR] wrong input"COLOR_RESET"\n");
    printf(COLOR_WARNING"USAGE: ./myShell [ -no-free | -no-merge | -final ]"COLOR_RESET"\n");
    exit(EXIT_FAILURE);
}

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief print_points explains how this exercise is evaluated
 * @param suffix name of the test-configuration
 */
static void print_points(char* suffix)
{
    printf(COLOR_POINTS"You can achieve up to %d points if every test is successful in this mode.\n", maximum_points);
    printf("For every test that fails or is missing 1 point is subtracted from this maximum.\n");
    printf("Furthermore, you will lose 1 point for each call to a library function without proper error handling.\n\n");
    printf("Please note, we will also inspect the code, cheating the tests will not yield any points!\n"COLOR_RESET"\n");
    
    printf(COLOR_POINTS"In the case of errors, please compare your memory_map with memory_map_%s"COLOR_RESET"\n", suffix);
}    

//-------------------------------------------------------------------------------------------------------------------//

/**
 * @brief main function tests all memory allocation and merging functionalities of malloc
 * @param argc: number of arguments
 * @param argv: null terminated array of strings containing command line arguments.
 * @return exit status
 */
int main(int argc, char* argv[])
{
    initExercises(argc, argv);
    
    // parsing command line input...
    if(argc==2 || (argc==3 && strstr(argv[2], "--env")))
    {
        if(strstr(argv[1], "-no-free"))
        {
            use_free  = 0;
            use_merge = 0;
            mode = NO_FREE;
        }
        else if(strstr(argv[1], "-no-merge"))
        {
            use_free  = 1;
            use_merge = 0;
            mode = NO_MERGE;
        }
        else if(strstr(argv[1], "-final"))
        {
            use_free  = 1;
            use_merge = 1;
            mode = FINAL;
        }
        else
        {
           handle_input_error(); 
        }
    }
    else
    {
        handle_input_error();
    }
    
    if(mode == NO_FREE)
    {
        maximum_points = TASK_1;
        tests_missing  = TOTAL_TESTS_NO_FREE;
        print_points("no_free");
    }
    else if(mode == NO_MERGE)
    {
        maximum_points = TASK_1 + TASK_2;
        tests_missing  = TOTAL_TESTS_NO_MERGE;
        print_points("no_merge");
    }
    else
    {
        maximum_points = TASK_1+TASK_2+TASK_3;
        tests_missing  = TOTAL_TESTS_FINAL;
        print_points("final");
    }
    
    
    
    file_creation_test();

    simple_allocation_and_free_test();
    simple_reallocation_and_free_test();
    mixed_allocation_and_free_test();
    failure_handling_test();
    advanced_reallocation_test();
    meta_data_test();
    
    checkpoint("end-of-test");

    printf(COLOR_MESSAGE"everything done"COLOR_RESET"\n\n\r");
    return 0;
}
