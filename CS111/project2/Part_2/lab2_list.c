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
#include "SortedList.h"
#include <gperftools/profiler.h>

int n_lists      = 1;
int n_threads    = 1;
int n_iterations = 1;
int key_len      = 20;
int n_elements;
SortedList_t **global_list;
SortedListElement_t *element_pool;
char sync_type  = 'n';
char y_args[4];
int opt_yield = 0x0;
unsigned long long total_lock_wait = 0;
unsigned long long *thrd_lock_wait;

char *sync_grain    = "coarse";
char *use_msg       = "lab2_add [--threads=] [--iterations=] [--yield] [--sync=]\n";
char *s_arg_msg     = "Invalid --sync arg: must be 'm', 's',\n";
char *yield_use_msg = "Invalid yield option: yieldopts = {none, i,d,l,id,il,dl,idl}\n";

pthread_mutex_t *mutex_arr;
pthread_mutex_t mymutex;
int *lock_arr_s;
int lock_s;

void
sig_corrupt() {
    fprintf(stderr, "Detected corrupted list.\n");
    fprintf(stderr, "Test stats: %d threads, %d iterations\n", n_threads, n_iterations);
    exit(2);
}

void
segfault_handler(int signum) {
    fprintf(stderr, "Segfault caused by corrupted list. Signal: %d\n", signum);
    sig_corrupt();
    exit(1);
}

void
get_tag(char* tag_ptr) {
    
    strcpy(tag_ptr,"list-");
    
    /** Get yieldopts */
    if (opt_yield) {
        tag_ptr = strncat(tag_ptr, y_args, 4);
    } else {
        tag_ptr = strcat(tag_ptr, "none");
    }
    tag_ptr = strcat(tag_ptr, "-");
    /** Get syncopts */
    if (sync_type == 'n') {
        tag_ptr = strcat(tag_ptr, "none");
    } else {
        tag_ptr = strncat(tag_ptr, &sync_type, 1);
    }
    
    fprintf(stderr, "test tag: %s\n", tag_ptr);
    return;
}

void
get_time(struct timespec *tspec) {
    /* notes the (high resolution) starting time for the run (using clock_gettime(3)) */
    if (clock_gettime(CLOCK_REALTIME, tspec) == -1) {
        fprintf(stderr, "clock_gettime() error: %s\n", strerror(errno));
        exit(1);
    }
}

unsigned long long
get_runtime(struct timespec *ts_start, struct timespec *ts_end) {
    unsigned long long runtime = 0;
    runtime += ((ts_end->tv_sec) - (ts_start->tv_sec)) * 1000000000;
    runtime += ((ts_end->tv_nsec) - (ts_start->tv_nsec));

    return runtime;
}

int
get_key_hash(const char *key) {
    
    
    /** Always return index of 0 for  coarse-grain syncrhonization */
    if ( ! strcmp(key, "coarse") ) {
        return 0;
    }
    
    int key_sum = 0;
    int key_hash_value;
    int len = strlen(key);
    for (int i = 0; i < len; i++) {
        key_sum += (int) key[i];
    }
    key_hash_value = (key_sum/n_lists) % n_lists;
    
    return key_hash_value;
}

void*
list_thrd(void *offset_p) {
    
    int offset = (int) (*((int*) offset_p));
    fprintf(stderr,"offset: %d\n", offset);
    struct timespec thrd_wait_start, thrd_wait_end;
    const char * elem_key;
    int sublist_idx;
    
    /* - - - - - - - - - - - INSERTIONS - - - - - - - - - - - - - */
    
    for (int i = (offset * n_iterations); i < (offset + 1) * n_iterations; i++) {
        
        // Hash key to get appropriate sublist
        elem_key = element_pool[i].key;
        sublist_idx = get_key_hash(elem_key);
        // fprintf(stderr,"hash value: %d\n", sublist_idx);
        
        // Start wait clock
        get_time(&thrd_wait_start);
        
        // Attempt to acquire lock
        if (sync_type == 'm') { pthread_mutex_lock(&mutex_arr[sublist_idx]); }
        else if (sync_type == 's') {
            while (__sync_lock_test_and_set(&lock_arr_s[sublist_idx], 1)) {
                while (lock_arr_s[sublist_idx]);
            }
        }
        
        // Calculate wait time
        get_time(&thrd_wait_end);
        thrd_lock_wait[offset] += get_runtime(&thrd_wait_start, &thrd_wait_end);
        
        // Perform Insertion
        SortedList_insert(global_list[sublist_idx], &element_pool[i]);
        
        // Release lock
        if (sync_type == 'm') { pthread_mutex_unlock(&mutex_arr[sublist_idx]); }
        else if (sync_type == 's') { __sync_lock_release(&lock_arr_s[sublist_idx]); }
        
        // fprintf(stderr,"Thread %d performed an insertion into list %d\n", offset, sublist_idx);
    }
    
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    
    // fprintf(stderr,"Insertions Complete\n");
    
    /* - - - - - - - - - - - - - - - - GET LENGTH - - - - - - - - - - - - - - - - */
    
    // Variable to store list length
    int list_len = 0;
    for (int i = 0; i < n_lists; i++) {

        // Start wait clock
        get_time(&thrd_wait_start);

        // Attempt to acquire lock
        if (sync_type == 'm') { pthread_mutex_lock(&mutex_arr[i]); }
        else if (sync_type == 's') {
            while (__sync_lock_test_and_set(&lock_arr_s[i], 1)) {
                while (lock_arr_s[i]);
            }
        }

        // Calculate wait time
        get_time(&thrd_wait_end);
        thrd_lock_wait[offset] += get_runtime(&thrd_wait_start, &thrd_wait_end);

        
        // Get sublist length
        int len_ret = SortedList_length(global_list[i]);
        // Reported any instance of corrupton
        if (len_ret == -1) {
            fprintf(stderr, "Corrupted list detected when getting list len in thread\n");
            sig_corrupt();
        }
        list_len += len_ret;
        
        // Release lock
        if (sync_type == 'm') { pthread_mutex_unlock(&mutex_arr[i]); }
        else if (sync_type == 's') { __sync_lock_release(&lock_arr_s[i]); }
    }
    
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    
    // fprintf(stderr,"Length Complete\n");
    
    /* - - - - - - - - - - - - - - - LOOK UP AND DELETE - - - - - - - - - - - - - - */
    
    // looks up and deletes each of the keys it had previously inserted
    for (int i = (offset * n_iterations); i < (offset + 1) * n_iterations; i++) {
        
        // Hash key to get appropriate sublist
        elem_key = element_pool[i].key;
        sublist_idx = get_key_hash(elem_key);
        
        // Start wait clock
        get_time(&thrd_wait_start);
        
        // Attempt to acquire lock
        if (sync_type == 'm') { pthread_mutex_lock(&mutex_arr[sublist_idx]); }
        else if (sync_type == 's') {
            while (__sync_lock_test_and_set(&lock_arr_s[sublist_idx], 1)) {
                while (lock_arr_s[sublist_idx]);
            }
        }
        
        // Calculate wait time
        get_time(&thrd_wait_end);
        thrd_lock_wait[offset] += get_runtime(&thrd_wait_start, &thrd_wait_end);
        
        // Perform look, Report any list corruption
        if ( SortedList_lookup(global_list[sublist_idx], element_pool[i].key) == NULL ) {
            fprintf(stderr, "Corrupted list detected when looking up in thread\n");
            sig_corrupt();
        }
        
        // Perform deletion, Report any list corruption
        if ( SortedList_delete(&element_pool[i]) == 1) {
            fprintf(stderr, "Corrupted list detected when deleting in thread\n");
            sig_corrupt();
        }
        
        // Release locks
        if (sync_type == 'm') { pthread_mutex_unlock(&mutex_arr[sublist_idx]); }
        else if (sync_type == 's') { __sync_lock_release(&lock_arr_s[sublist_idx]); }
    }
    
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    
    // fprintf(stderr,"Lookups/Deletions Complete\n");
    
    return NULL;
}

void
get_opts(int argc, char **argv) {
    int returned_option;
    char s_arg;
    while (1)
    {
        static struct option long_options[] =
        {
            {"lists",       required_argument, 0, 'l'},
            {"threads",     required_argument, 0, 't'},
            {"iterations",  required_argument, 0, 'i'},
            {"yield",       required_argument, 0, 'y'},
            {"sync",        required_argument, 0, 's'},
            {0, 0, 0, 0}
        };
        
        int option_index = 0;
        returned_option = getopt_long(argc, argv, "l:t:i:ys:", long_options, &option_index);
        
        /* End of options */
        if (returned_option == -1) { break; }
        
        switch (returned_option) {
            case 'l':
                sync_grain = "fine";
                n_lists = strtol(optarg, NULL, 0);
                break;
            case 't':
                n_threads = strtol(optarg, NULL, 0);
                break;
            case 'i':
                n_iterations = strtol(optarg, NULL, 0);
                break;
            case 'y':
                opt_yield = 1;
                strcpy(y_args, (char*) optarg);
                fprintf(stderr,"Yield options: %s\n", optarg);
                for (int i = 0; i < (int) strlen(optarg); i++) {
                         if (optarg[i] == 'i') { opt_yield |= INSERT_YIELD; }
                    else if (optarg[i] == 'd') { opt_yield |= DELETE_YIELD; }
                    else if (optarg[i] == 'l') { opt_yield |= LOOKUP_YIELD; }
                    else { fprintf(stderr,"%s", yield_use_msg); exit(1);    }
                }
                fprintf(stderr,"%x\n", opt_yield);
                break;
            case 's':
                s_arg = (char) optarg[0];
                if (strlen(optarg) != 1) {
                    fprintf(stderr, "%s", s_arg_msg);
                    exit(1);
                }
                switch (s_arg) {
                    case 'm':
                        sync_type = 'm';
                        pthread_mutex_init(&mymutex, NULL);
                        break;
                    case 's':
                        sync_type = 's';
                        break;
                    default: // for unrecognized --sync arguments
                        fprintf(stderr, "%s", s_arg_msg);
                        exit(1);
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

/** https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c */
void
gen_random(char *s, const int len) {
    static const char alphanum[] =
    "123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
    
    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    s[len] = '\0';
}

int
main(int argc, char** argv) {
    
    // Get CLO
    get_opts(argc, argv);
    
    
    // Allocate space for shared element poll
    n_elements   = n_threads * n_iterations;
    element_pool = (SortedListElement_t *) malloc( n_elements * sizeof(SortedListElement_t) );
    
    // Allocate space for global list
    global_list = (SortedList_t **) malloc(n_lists * sizeof(SortedList_t*));
    for (int i = 0; i < n_lists; i++) {
        global_list[i] = (SortedList_t *) malloc(sizeof(SortedList_t));
    }
    
    // Allocate space for and initalize locks and mutexes
    lock_arr_s = (int *) malloc(n_lists * sizeof(int));
    for (int i = 0; i < n_lists; i++) {
        lock_arr_s[i] = 0;
        fprintf(stderr, "Initial lock val: %d\n", lock_arr_s[i]);
    }
    
    mutex_arr = (pthread_mutex_t *) malloc(n_lists * sizeof(pthread_mutex_t));
    for (int i = 0; i < n_lists; i++) {
        pthread_mutex_init(&mutex_arr[i], NULL);
    }
    
    
    // Verify dynamic allocation
    if (!element_pool || !global_list || !lock_arr_s || !mutex_arr) {
        fprintf(stderr, "malloc() error: %s\n", strerror(errno));
        exit(1);
    }
    
    
    // Initialize each subblist head node .
    for (int i = 0; i < n_lists; i++) {
        global_list[i]->next = global_list[i];
        global_list[i]->prev = global_list[i];
        global_list[i]->key  = NULL;
    }
    
    // Allocate space for and get randomized key for each element
    srand(time(NULL));
    for (int i = 0; i < n_elements; i++) {
        char* tmp_key       = (char*) malloc(key_len * sizeof(char));
        gen_random(tmp_key, key_len);
        element_pool[i].key = tmp_key;
        // fprintf(stderr, "Random Key: %s\n", element_pool[i].key);
    }
    
    // Generate thread-safe offset value for each thread
    int offset_arr[n_threads];
    for (int t = 0; t < n_threads; t++) {
        offset_arr[t] = t;
    }
    
    // Allocate space for local counter for each thread
    thrd_lock_wait = (unsigned long long *) malloc(n_threads * sizeof(unsigned long long));
    for (int t = 0; t < n_threads; t++) {
        thrd_lock_wait[t] = 0;
        // fprintf(stderr, "Initial wait val: %llu\n", thrd_lock_wait[t]);
    }
    
    // Signal segfault handler
    signal(SIGSEGV, segfault_handler);
    
    // Get start time
    struct timespec tspec_start, tspec_end;
    get_time(&tspec_start);
    
    // Create threads
    int ret_code;
    pthread_t thrd_ids[n_threads];
    for (int t = 0; t < n_threads; t++) {
        ret_code = pthread_create(&thrd_ids[t], NULL, list_thrd, (void *) &offset_arr[t]);
        if (ret_code){
            fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", ret_code);
            exit(1);
        }
    }
    
    /// wait all threads by joining them
    for (int t = 0; t < n_threads; t++) {
        pthread_join(thrd_ids[t], NULL);
        /// get lock wait time of each thread when it joins
        // fprintf(stderr, "total wait time for thread %d: %llu\n", t, thrd_lock_wait[t]);
        total_lock_wait += thrd_lock_wait[t];
    }
    
    // Calculate runtime
    get_time(&tspec_end);
    
    
    
    // Variable to store list length
    int list_len = 0;
    for (int i = 0; i < n_lists; i++) {
        // Get sublist length
        list_len += SortedList_length(global_list[i]);
        if (list_len != 0) {
            fprintf(stderr, "Corrupted list detected in main()\n");
            sig_corrupt();
            
        }
        
    }
    fprintf(stderr, "list is consistent, list_len: %d\n", list_len);
    
    
    
    char tag_str[15];
    get_tag(tag_str);
    
    unsigned long long runtime = get_runtime(&tspec_start, &tspec_end);
    unsigned long long n_operations = n_threads * n_iterations * 3;
    unsigned long long avg_op_time = runtime / n_operations;
    unsigned long long n_lock_operations = (2 * n_elements) + n_threads;
    unsigned long long avg_lock_wait_time = total_lock_wait / n_lock_operations;
    printf("%s,%d,%d,%d,%lli,%lli,%lli,%llu\n",
           tag_str,
           n_threads,
           n_iterations,
           n_lists,
           n_operations,
           runtime,
           avg_op_time,
           avg_lock_wait_time
           );
    
    // fprintf(stderr,"- - - - - - - - - - - - -\ntotal wait: %llu n_lock_ops: %llu\n", total_lock_wait, n_lock_operations);
    
    for (int i = 0; i < n_elements; i++) {
        void *tmp_p = (void*) element_pool[i].key;
        free(tmp_p);
    }
    
    pthread_mutex_destroy(&mymutex);
    pthread_exit(NULL);
    exit(0);
}
