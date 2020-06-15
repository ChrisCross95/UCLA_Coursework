#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>

int n_threads    = 1;
int n_iterations = 1;
long long counter = 0;
bool opt_yield = false;
bool sync_flag = true;
char sync_type  = 'n';

char *use_msg   = "lab2_add [--threads=] [--iterations=] [--yield] [--sync=]\n";
char *s_arg_msg = "Invalid --sync arg: must be 'm', 's', 'c'\n";

pthread_mutex_t mymutex;
int lock_s;


/*
 add-none       ... no yield, no synchronization
 add-m          ... no yield, mutex synchronization
 add-s          ... no yield, spin-lock synchronization
 add-c          ... no yield, compare-and-swap synchronization
 add-yield-none ... yield, no synchronization
 add-yield-m    ... yield, mutex synchronization
 add-yield-s    ... yield, spin-lock synchronization
 add-yield-c    ... yield, compare-and-swap synchronization
 */
void
get_tag(char* tag_ptr) {
    
    if (opt_yield) {
        switch (sync_type) {
            case 'n':
                strcpy(tag_ptr, "add-yield-none");
                break;
            case 'm':
                strcpy(tag_ptr, "add-yield-m");
                break;
            case 's':
                strcpy(tag_ptr, "add-yield-s");
                break;
            case 'c':
                strcpy(tag_ptr, "add-yield-c");
                break;
            default:
                fprintf(stderr, "Unexpected option parsing:\n%s",s_arg_msg);
                exit(2);
                break;
        }
    } else {
        switch (sync_type) {
            case 'n':
                strcpy(tag_ptr, "add-none");
                break;
            case 'm':
                strcpy(tag_ptr, "add-m");
                break;
            case 's':
                strcpy(tag_ptr, "add-s");
                break;
            case 'c':
                strcpy(tag_ptr, "add-c");
                break;
            default:
                fprintf(stderr, "Unexpected option parsing:\n%s",s_arg_msg);
                exit(2);
                break;
        }
    }
    return;
}


void
add(long long *pointer, long long value) {
    
    long long sum, oldVal;
    switch (sync_type) {
        case 'n': // no sync option, default option
            sum = (*pointer) + value;
            if (opt_yield) { sched_yield(); }
            *pointer = sum;
            break;
        case 'c': // use compare-and-swap
            do
            {
                oldVal = *pointer; // save counter's previous value
                sum = oldVal + value; // calculate sum based on previous value
                if (opt_yield) { sched_yield(); }
            } while ( __sync_val_compare_and_swap(pointer, oldVal, sum) != oldVal);
            // update only when counter is still equal to previous value
            break;
        case 'm': // use mutex
            pthread_mutex_lock(&mymutex);
            sum = (*pointer) + value;
            if (opt_yield) { sched_yield(); }
            *pointer = sum;              /* Critical Section */
            pthread_mutex_unlock(&mymutex);
            break;
        case 's': // spin-lock
            while (__sync_lock_test_and_set(&lock_s, 1)) {
                while (lock_s);
            }
            sum = (*pointer) + value;
            if (opt_yield) { sched_yield(); }
            *pointer = sum;             /* Critical Section */
            __sync_lock_release(&lock_s);
            break;
        default:
            break;
    }
    // fprintf(stderr, "counter value: %lld\n", counter);
}

/*
one protected by a pthread_mutex enabled by a new --sync=m option. When running this test, the output statistics should begin with "add-m" or "add-yield-m".
one protected by a spin-lock, enabled by a new --sync=s option. You will have to implement your own spin-lock operation. We suggest that you do this using the GCC atomic__sync_builtin functions __sync_lock_test_and_set and __sync_lock_release. When running this test, the output statistics should begin with "add-s" or "add-yield-s".
one that performs the add using the GCC atomic_sync_buildin function __sync_val_compare_and_swap to ensure atomic updates to the shared counter, enabled by a new --sync=c option. In this test case, because compare-and-swap changes the algorithm, the yield check should be put between the computation of the new sum and performing the compare-and-swap. When running this test, the output statistics should begin with "add-c" or "add-yield-c".
*/

void
add_thrd(int *n_iterations) {
    
    int n_iters = *n_iterations;
    
    // add 1 to the counter the specified number of times
    for (int i = 0; i < n_iters; i++) {
        add(&counter, 1);
    }
    
    // add i1 to the counter the specified number of times
    for (int i = 0; i < n_iters; i++) {
        add(&counter, -1);
    }
    
    return;
}

void
get_opts(int argc, char **argv) {
    int returned_option;
    char s_arg;
    while (1)
    {
        static struct option long_options[] =
        {
            {"threads",     required_argument, 0, 't'},
            {"iterations",  required_argument, 0, 'i'},
            {"yield",       no_argument,       0, 'y'},
            {"sync",        required_argument, 0, 's'},
            {0, 0, 0, 0}
        };
        
        int option_index = 0;
        returned_option = getopt_long(argc, argv, "t:i:ys:", long_options, &option_index);
        
        /* End of options */
        if (returned_option == -1) { break; }
        
        switch (returned_option) {
            case 't':
                n_threads = strtol(optarg, NULL, 0);;
                break;
            case 'i':
                n_iterations = strtol(optarg, NULL, 0);
                break;
            case 'y':
                opt_yield = true;
                break;
            case 's':
                s_arg = (char) optarg[0];
                if (strlen(optarg) != 1) {
                    fprintf(stderr, "%s", s_arg_msg);
                    exit(2);
                }
                switch (s_arg) {
                    case 'm':
                        sync_type = 'm';
                        pthread_mutex_init(&mymutex, NULL);
                        break;
                    case 's':
                        sync_type = 's';
                        break;
                    case 'c':
                        sync_type = 'c';
                        break;
                    default: // for unrecognized --sync arguments
                        fprintf(stderr, "%s", s_arg_msg);
                        exit(2);
                        break;
                }
                break;
            default: // for unrecognized options passed to main
                fprintf(stderr, "%s", use_msg);
                exit(1);
                break;
        }
    }
    fprintf(stderr,"n_threads: %d\n", n_threads);
    fprintf(stderr,"n_iterations: %d\n", n_iterations);
}

struct timespec tspec_start, tspec_end;

void
get_time(struct timespec *tspec) {
    /* notes the (high resolution) starting time for the run (using clock_gettime(3)) */
    if (clock_gettime(CLOCK_REALTIME, tspec) == -1) {
        fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
        exit(1);
    }
}

long long
get_runtime(struct timespec *ts_start, struct timespec *ts_end) {
    long long runtime = 0;
    // printf("ts_end->tv_sec: %lu\n", ts_end->tv_sec);
    // printf("ts_start->tv_sec: %lu\n", ts_start->tv_sec);
    // printf("ts_end->tv_nsec: %lu\n", ts_end->tv_nsec);
    // printf("ts_start->tv_nsec: %lu\n", ts_start->tv_nsec);
    runtime += ((ts_end->tv_sec) - (ts_start->tv_sec)) * 1000000000;
    // printf("After secs: %lli\n", runtime);
    runtime += ((ts_end->tv_nsec) - (ts_start->tv_nsec));
    // printf("After nsecs: %lli\n", runtime);
    // printf("Runtime in nanoseconds is: %lli\n", runtime);
    return runtime;
}

int
main(int argc, char** argv) {
    
    get_opts(argc, argv);
    
    get_time(&tspec_start); /* Get start time */
    
    fprintf(stderr, "counter initial value: %lld\n", counter);
    int ret_code;
    pthread_t thrd_ids[n_threads];
    for (int t = 0; t < n_threads; t++) {
        ret_code = pthread_create(&thrd_ids[t], NULL, add_thrd, (void *) &n_iterations);
        fprintf(stderr, "n_iterations: %d\n", *(&n_iterations));
        if (ret_code){
            fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", ret_code);
            exit(1);
        }
    }
    
    /// wait all threads by joining them
    for (int t = 0; t < n_threads; t++) {
        pthread_join(thrd_ids[t], NULL);
    }
    
    get_time(&tspec_end); /* Get end time */
    
    char tag_str[15];
    get_tag(tag_str);
    
    long long runtime = get_runtime(&tspec_start, &tspec_end);
    long long n_operations = n_threads * n_iterations * 2;
    long long avg_op_time = runtime / n_operations;
    printf("%s,%d,%d,%lli,%lli,%lli,%lli\n",
           tag_str,
           n_threads,
           n_iterations,
           n_operations,
           runtime,
           avg_op_time,
           counter
           );
    
    
    pthread_mutex_destroy(&mymutex);
    pthread_exit(NULL);
    exit(0);
}

/* Link with -lrt (only for glibc versions before 2.17). */
/* -pthreads */

/*
 Start with a basic add routine:
 
 void add(long long *pointer, long long value) {
 long long sum = *pointer + value;
 *pointer = sum;
 }
 Write a test driver program (called lab2_add) that:
 
 takes a parameter for the number of parallel threads (--threads=#, default 1).
 takes a parameter for the number of iterations (--iterations=#, default 1).
 initializes a (long long) counter to zero
 notes the (high resolution) starting time for the run (using clock_gettime(3))
 starts the specified number of threads, each of which will use the above add function to
 add 1 to the counter the specified number of times
 add -1 to the counter the specified number of times
 exit to re-join the parent thread
 wait for all threads to complete, and notes the (high resolution) ending time for the run
 prints to stdout a comma-separated-value (CSV) record including:
 the name of the test (add-none for the most basic usage)
 the number of threads (from --threads=)
 the number of iterations (from --iterations=)
 the total number of operations performed (threads x iterations x 2, the "x 2" factor because you add 1 first and then add -1)
 the total run time (in nanoseconds)
 the average time per operation (in nanoseconds).
 the total at the end of the run (0 if there were no conflicting updates)
 If bad command-line parameters are encountered or a system call fails, exit with a return code of one. If the run completes successfully, exit with a return code of zero. If any errors (other than a non-zero final count) are encountered, exit with a return code of two.
 */
