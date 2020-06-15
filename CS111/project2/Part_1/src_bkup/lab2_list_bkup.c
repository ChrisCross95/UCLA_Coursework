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
#include "SortedList_h"

int n_lists      = 1;
int n_threads    = 1;
int n_iterations = 1;
int n_elements   = n_threads * n_iterations;
int key_len      = 20;
SortedList_t *global_list;
SortedListElement_t element_pool[n_elements];
bool opt_yield = false;
bool sync_flag = true;
char sync_type  = 'n';
char y_args[4];

char *use_msg   = "lab2_add [--threads=] [--iterations=] [--yield] [--sync=]\n";
char *s_arg_msg = "Invalid --sync arg: must be 'm', 's',\n";

pthread_mutex_t mymutex;
int lock_s;


/* The name of the test, which is of the form: list-yieldopts-syncopts: where
yieldopts = {none, i,d,l,id,il,dl,idl}
syncopts = {none,s,m}
*/

void
get_tag(char* tag_ptr) {
    
    strcpy(tag_ptr,"list-");
    
    /** Get yieldopts */
    if (opt_yield == true) {
         tag_ptr = strncat(tag_ptr, y_args, 4);
    } else {
        tag_ptr = strncat(tag_ptr, "none", 4);
    }
    tag_ptr = strncat(tag_ptr, "-", 1);
    /** Get syncopts */
    if (sync_flag == true) {
        tag_ptr = strncat(tag_ptr, (char *) sync_type, 1);
    } else {
        tag_ptr = strncat(tag_ptr, "none", 4);
    }
    
    fprintf("test tag: %s\n", tag_ptr);
    return;
}

void
list_thrd(int *offset) {
    
    int offset = *offset;
    
    // inserts them all into a (single shared-by-all-threads) list
    for (int i = 0; i < n_iterations; i++) {
        SortedList_insert(global_list, &element_pool[offset + i]);
    }
    
    // gets the list length
    int list_len;
    list_len = SortedList_length(global_list);
    fprintf(stderr, "list_len: %d\n", list_len);
    
    looks up and deletes each of the keys it had previously inserted
    for (int i = 0; i < n_iterations; i++) {
        SortedList_insert(global_list, element_pool[offset + i].key);
        SortedList_delete(&element_pool[offset + i])
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
                opt_yield = true;
                strcpy(y_args, (char*) optarg);
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

/** https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c */
void
gen_random(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}


int
main(int argc, char** argv) {
    
    get_opts(argc, argv);
    
    global_list->next = global_list;
    global_list->prev = global_list;
    global_list->key  = NULL;
    
    
    SortedListElement_t element_pool[n_elements];
    for (int i = 0; i < n_elements; i++) {
        
        element_pool[i].key = (char*) malloc(key_len);
        gen_random(element_pool[i].key, key_len);
        fprintf(stderr, "Random Key: %s\n", element_pool[i].key)
        
    }
    
    
    get_time(&tspec_start); /* Get start time */
    

    int ret_code;
    pthread_t thrd_ids[n_threads];
    for (int t = 0; t < n_threads; t++) {
        int offset = t;
        ret_code = pthread_create(&thrd_ids[t], NULL, list_thrd, (void *) &offset);
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
    if (list_len != 0) {
        fprintf(stderr, "list is inconsistent, list_len is: %d, should be 0\n", list_len);
        exit(2);
    } else {
        fprintf(stderr, "list is consistent, list_len: %d\n", list_len);
    }
    
    char tag_str[15];
    get_tag(tag_str);
    
    long long runtime = get_runtime(&tspec_start, &tspec_end);
    long long n_operations = n_threads * n_iterations * 3;
    long long avg_op_time = runtime / n_operations;
    printf("%s,%d,%d,%d,%lli,%lli,%lli,%lli\n",
           tag_str,
           n_threads,
           n_lists,
           n_iterations,
           n_operations,
           runtime,
           avg_op_time,
           counter
           );
    
    
    for (int i = 0; i < n_elements; i++) {
        free(element_pool[i].key);
    }
    
    
    pthread_mutex_destroy(&mymutex);
    pthread_exit(NULL);
    exit(0);
}
