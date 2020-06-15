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

int n_lists      = 1;
int n_threads    = 1;
int n_iterations = 1;
int key_len      = 20;
int n_elements;
SortedList_t *global_list;
SortedListElement_t *element_pool;
char sync_type  = 'n';
char y_args[4];
int opt_yield = 0x0;

char *use_msg       = "lab2_add [--threads=] [--iterations=] [--yield] [--sync=]\n";
char *s_arg_msg     = "Invalid --sync arg: must be 'm', 's',\n";
char *yield_use_msg = "Invalid yield option: yieldopts = {none, i,d,l,id,il,dl,idl}\n";

pthread_mutex_t mymutex;
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

void*
list_thrd(void *offset_p) {

    int offset = (int) (*((int*) offset_p));
    fprintf(stderr,"offset: %d\n", offset);
    
    
    
    // inserts them all into a (single shared-by-all-threads) list
    for (int i = (offset * n_iterations); i < (offset + 1) * n_iterations; i++) {
        
        if (sync_type == 'm') { pthread_mutex_lock(&mymutex); }
        else if (sync_type == 's') {
            while (__sync_lock_test_and_set(&lock_s, 1)) {
                while (lock_s);
            }
        }
        SortedList_insert(global_list, &element_pool[i]);
        
        if (sync_type == 'm') { pthread_mutex_unlock(&mymutex); }
        else if (sync_type == 's') { __sync_lock_release(&lock_s); }
    }
    
    
    // gets the list length
    int list_len;
    if (sync_type == 'm') { pthread_mutex_lock(&mymutex); }
    else if (sync_type == 's') {
        while (__sync_lock_test_and_set(&lock_s, 1)) {
            while (lock_s);
        }
    }
    list_len = SortedList_length(global_list);
    if (list_len == -1) { sig_corrupt(); }
    fprintf(stderr, "list_len: %d\n", list_len);
    if (sync_type == 'm') { pthread_mutex_unlock(&mymutex); }
    else if (sync_type == 's') { __sync_lock_release(&lock_s); }
    
    
    // looks up and deletes each of the keys it had previously inserted
    for (int i = (offset * n_iterations); i < (offset + 1) * n_iterations; i++) {
        if (sync_type == 'm') { pthread_mutex_lock(&mymutex); }
        else if (sync_type == 's') {
            while (__sync_lock_test_and_set(&lock_s, 1)) {
                while (lock_s);
            }
        }
        
        if ( SortedList_lookup(global_list, element_pool[i].key) == NULL ) {
            sig_corrupt();
        }
        if ( SortedList_delete(&element_pool[i]) == 1) {
            sig_corrupt();
        }
        if (sync_type == 'm') { pthread_mutex_unlock(&mymutex); }
        else if (sync_type == 's') { __sync_lock_release(&lock_s); }
    }
    
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
            {"threads",     required_argument, 0, 't'},
            {"iterations",  required_argument, 0, 'i'},
            {"yield",       required_argument, 0, 'y'},
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
    runtime += ((ts_end->tv_sec) - (ts_start->tv_sec)) * 1000000000;
    runtime += ((ts_end->tv_nsec) - (ts_start->tv_nsec));
    return runtime;
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
    
    get_opts(argc, argv);
    
    n_elements   = n_threads * n_iterations;
    element_pool = (SortedListElement_t *) malloc( n_elements * sizeof(SortedListElement_t) );
    global_list  = (SortedListElement_t *) malloc(sizeof(SortedList_t));
    if (!element_pool || !global_list) {
        fprintf(stderr, "malloc() error: %s\n", strerror(errno));
        exit(1);
    }
    
    global_list->next = global_list;
    global_list->prev = global_list;
    global_list->key  = NULL;
    
    
    for (int i = 0; i < n_elements; i++) {
        char* tmp_key       = (char*) malloc(key_len * sizeof(char));
        gen_random(tmp_key, key_len);
        element_pool[i].key = tmp_key;
        // fprintf(stderr, "Random Key: %s\n", element_pool[i].key);
    }
    
    int offset_arr[n_threads];
    for (int t = 0; t < n_threads; t++) {
        offset_arr[t] = t;
    }

    signal(SIGSEGV, segfault_handler);

    get_time(&tspec_start); /* Get start time */
    
    int ret_code;
    pthread_t thrd_ids[n_threads];
    for (int t = 0; t < n_threads; t++) {
        ret_code = pthread_create(&thrd_ids[t], NULL, list_thrd, (void *) &offset_arr[t]);
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
    
    // gets the list length
    int list_len;
    list_len = SortedList_length(global_list);
    if (list_len != 0) { sig_corrupt(); }
        
    fprintf(stderr, "list is consistent, list_len: %d\n", list_len);
    
    char tag_str[15];
    get_tag(tag_str);
    
    long long runtime = get_runtime(&tspec_start, &tspec_end);
    long long n_operations = n_threads * n_iterations * 3;
    long long avg_op_time = runtime / n_operations;
    printf("%s,%d,%d,%d,%lli,%lli,%lli\n",
           tag_str,
           n_threads,
           n_iterations,
           n_lists,
           n_operations,
           runtime,
           avg_op_time
           );
    
    
    for (int i = 0; i < n_elements; i++) {
        void *tmp_p = (void*) element_pool[i].key;
        free(tmp_p);
    }
    
    
    pthread_mutex_destroy(&mymutex);
    pthread_exit(NULL);
    exit(0);
}
