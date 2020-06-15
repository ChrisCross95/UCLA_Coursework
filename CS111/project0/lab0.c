#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

bool input_flag   = false;
bool output_flag  = false;
bool segfaul_flag = false;
bool catch_flag   = false;
#define BUF_SIZE 256

int input_fd, output_fd;
char * use_msg = "lab0 --input=<input> --output=<output> --segfault --catch";
void
segfault_handler(int signum) {
    fprintf(stderr, "Segfault Caught. Signal number: %d\n", signum);
    exit(4);
}

void
segfault() {
    char *cptr = NULL;
    *cptr      = 'x';
    return;
}

int
main(int argc, char** argv) {
    
    int returned_option;
    while (1)
    {
        static struct option long_options[] =
        {
            /* --shell argument to pass input/output
             between the terminal and a shell */
            {"input",       required_argument, 0,  'i'},
            {"output",      required_argument, 0,  'o'},
            {"segfault",    no_argument,       0,  's'},
            {"catch",       no_argument,       0,  'c'},
            /* Terminate the array with an element containing all zeros. */
            {0, 0, 0, 0}
        };
        
        int option_index = 0; /* Updated by getopt_long() */
        returned_option = getopt_long(argc, argv, "i:o:sc",
                                      long_options, &option_index);
        
        /* Detect the end of the options. */
        if (returned_option == -1) { break; }
        
        switch (returned_option) {
            case 'i':
                input_flag = true;
                input_fd = open(optarg, O_RDONLY);
                if (input_fd == -1) {
                    fprintf(stderr, "open() error: %s\n", strerror(errno));
                    exit(2);
                }
                if (dup2(input_fd, STDIN_FILENO) == -1) {
                    fprintf(stderr, "dup2() error: %s\n", strerror(errno));
                    exit(1);
                }
                break;
            case 'o':
                output_flag = true;
                output_fd = creat(optarg, 0644);
                if (output_fd == -1) {
                    fprintf(stderr, "creat() error: %s\n", strerror(errno));
                    exit(3);
                }
                if (dup2(output_fd, STDOUT_FILENO) == -1) {
                    fprintf(stderr, "dup2() error: %s\n", strerror(errno));
                    exit(1);
                }
                break;
            case 's':
                segfaul_flag = true;
                break;
            case 'c':
                catch_flag = true;
                signal(SIGSEGV, segfault_handler);
                break;
            case '?':
                fprintf(stderr, "%s\n", use_msg);
                exit(1);
                break;
            default:
                exit(1);
        }
    }
    
    if (segfaul_flag) { segfault(); }
    
    char read_buf[BUF_SIZE + 1];
    ssize_t read_len;
    while ((read_len = read(STDIN_FILENO, read_buf, BUF_SIZE)) > 0) {
        if (read_len == -1) {
            fprintf(stderr, "read() error: %s\n", strerror(errno));
            exit(1);
        }
        
        if (write(STDOUT_FILENO, read_buf, read_len) == -1) {
            fprintf(stderr, "write() error: %s\n", strerror(errno));
            exit(1);
        }
    }
    
    if (input_flag == true)   { close(input_fd); }
    if (output_flag == true ) { close(output_fd); }
    
    exit(0);
}
