#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <termios.h>    /* For manipulating terminal settings */
#include <sys/ioctl.h>  /* ioctl - control device */
#include <poll.h>       /* For monitoring I/0 readiness*/
#include <pthread.h>    /* On Linux, compile using cc -pthread. */

/* Pipefd[0] refers to the read end of the pipe. Pipefd[1] refers
 to the write end of the pipe.*/
int pipeToChild[2]   = {-1, -1};
int pipeFromChild[2] = {-1, -1};
#define PARENT_READ    pipeFromChild[0]
#define CHILD_WRITE    pipeFromChild[1]
#define CHILD_READ     pipeToChild[0]
#define PARENT_WRITE   pipeToChild[1]

#define SCALING_FACTOR    2
#define READ_SIZE        256
#define ctrl_C          0x03
#define ctrl_D          0x04  /* Escape Charachter */
#define cr_char         0x0D  /* Carraige Return */
#define lf_char         0x0A  /* Line Feed */

static int output_fd  = 1;
static int input_fd   = 0;
static int shell_flag = 0; /* Flag set by '--shell'. */
int buffer_size       = 1024 * sizeof(char);
int timeout_msecs     = 0;

/* For storing terminal settings */
struct termios original_settings, full_duplex_settings;

void
check_buffer(char* usrin_buf, int buf_index, int buf_end) {
    /* Reallocate buffer space as necessary */
    if (buf_index == buf_end) {
        buf_end *= SCALING_FACTOR;
        if ( (usrin_buf = (char*) realloc(usrin_buf, buf_end)) == NULL) {
            fprintf(stderr, "Realloc Error: %s\n", strerror(errno));
            exit(1);
        } else {
            printf("Buffer scaled!\n");
            buf_end *= SCALING_FACTOR;
        }
    }
    return;
}

void
set_term_attr(struct termios *target_termios) {
    if (target_termios == NULL) {
        fprintf(stderr,"Error: empty reference\n");
        exit(EXIT_FAILURE);
    }
    target_termios->c_iflag = ISTRIP;
    target_termios->c_oflag  = 0;
    target_termios->c_lflag  = 0;
    return;
}

void
shutdown(struct termios *original_settings) {
    if (original_settings == NULL) {
        fprintf(stderr,"Error: empty reference\n");
        exit(EXIT_FAILURE);
    }
    if (tcsetattr(STDIN_FILENO, TCSANOW, original_settings) < 0) {
        fprintf(stderr, "tcsetattr() error: %s\n", strerror(errno));
        return;
    }
    fprintf(stderr, "%c%c(ICANON set)\n", cr_char, lf_char);
    
    /* Memory and file management :) */
    if (close(input_fd) == -1) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }
    if (close(output_fd) == -1) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }
}

void
setstdstreams() {
    
    /* Close unused file descriptors */
    if (close(PARENT_WRITE) == -1) {
        fprintf(stderr, "close() error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (close(PARENT_READ) == -1) {
        fprintf(stderr, "close() error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Calls to dup2() are atomic */
    /* Redirect stdin to CHILD_READ */
    if (dup2(CHILD_READ, STDIN_FILENO) == -1) {
        fprintf(stderr, "dup2() Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Redirect stdout to CHILD_WRTIE */
    if (dup2(CHILD_WRITE, STDOUT_FILENO) == -1) {
        fprintf(stderr, "dup2() Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Redirect stderr to CHILD_WRTIE */
    if (dup2(CHILD_WRITE, STDERR_FILENO) == -1) {
        fprintf(stderr, "dup2() Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void
readpipe(int pipefd) {
    
    /* Create buffer to hold data from standard input */
    char *read_buf = NULL;
    if ((read_buf = (char*) malloc(READ_SIZE)) == NULL) {
        fprintf(stderr, "Malloc Error.\n");
        exit(1);
    };
    
    /* Read user input. Loop broken by ^D (0x04) */
    int read_len = 0;
    read_len = read(pipefd, read_buf, READ_SIZE);
    if (read_len < 0) {
        fprintf(stderr, "Pipe read error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        for (int i = 0; i < read_len; i++) {
            switch (read_buf[i]) {
                /* Upon receiving EOF from the shell, restore
                 terminal modes and exit with return code 1 */
                case ctrl_D:
                    shutdown(&original_settings);
                    exit(1);
                    break;
                case lf_char:
                    fprintf(stderr, "%c%c", cr_char, lf_char);
                    break;
                default:
                    write(STDOUT_FILENO, &read_buf[i], 1);
                    break;
            }
        }
    }
    
    return;
}


void
setpollfd(struct pollfd *pollfds) {
#define keybd_pollfd pollfds[0]
    keybd_pollfd.fd     = STDIN_FILENO;
    keybd_pollfd.events    = POLLIN;
    keybd_pollfd.revents = timeout_msecs;
#define pipe_pollfd  pollfds[1]
    pipe_pollfd.fd      = PARENT_READ;
    pipe_pollfd.events     = POLLIN;
    pipe_pollfd.revents  = timeout_msecs;
}

void
shellhalt_handler(int signal) {
    shutdown(&original_settings);
    exit(1);
}


int
main(int argc, char** argv) {
    
    /* Get the current terminal modes, save them for
     restoration, and then make a copy */
    
    /* isatty - test whether a file descriptor refers to a terminal
     https://www.gnu.org/software/libc/manual/html_node/Noncanon-Example.html
     */
    if (isatty(STDIN_FILENO) == 0) {
        fprintf(stderr, "isatty() error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if (tcgetattr(STDIN_FILENO, &original_settings) < 0) {
        fprintf(stderr, "tcgetattr() error: %s\n", strerror(errno));
        return 1;
    }
    
    if (tcgetattr(STDIN_FILENO, &full_duplex_settings) < 0) {
        fprintf(stderr, "tcgetattr() error: %s\n", strerror(errno));
        return 1;
    }
    
    /* Load non-canonical settings */
    set_term_attr(&full_duplex_settings);
    
    /* Place terminal in non-canonical mode */
    if (tcsetattr(0, TCSANOW, &full_duplex_settings) < 0) {
        fprintf(stderr, "tcsetattr() Error: %s\n", strerror(errno));
        return 1;
    } else {
        fprintf(stderr, "(ICANON unset)%c%c", cr_char, lf_char);
    }
    
    int returned_option;
    while (1)
    {
        static struct option long_options[] =
        {
            /* --shell argument to pass input/output
             between the terminal and a shell */
            {"shell",  no_argument,  &shell_flag, 1},
            /* Terminate the array with an element containing all zeros. */
            {0, 0, 0, 0}
        };
        
        int option_index = 0; /* Updated by getopt_long() */
        returned_option = getopt_long(argc, argv, "",
                                      long_options, &option_index);
        
        /* Detect the end of the options. */
        if (returned_option == -1) { break; }
        
        switch (returned_option) {
            case 0:
                /* If option sets a flag, do nothing for now. */
                if (long_options[option_index].flag != 0)
                    break;
            case '?':
                /* getopt_long already printed an error message. */
                exit(1);
                break;
            default:
                exit(1);
        }
    }
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    
    /* Shell option evoked: fork to create a new process, exec a shell (/bin/bash),
     whose standard input is a pipe from the terminal process, and whose standard
     output and standard error are (dups of) a pipe to the terminal process. */
    pid_t cpid;
    if (shell_flag == 1) {
        
        if ((pipe(pipeToChild) == -1) || (pipe(pipeFromChild) == -1)) {
            fprintf(stderr, "Cannot create pipes: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        if ( (cpid = fork()) < 0) {
            fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        }
        
        /* TODO: Figure out why standard streams are not redirected*/
        /* TODO: read about execlp */
        /* Child Process */
        if (cpid == 0) {
            
            /* Redirects stdin, stdout, and stderr of child process */
            setstdstreams();
            
            char *myargs[2];
            myargs[0] = strdup("/bin/bash");    /* program: "bash" (shell) */
            myargs[1] = NULL;                   /* marks end of array */
            if (execvp("/bin/bash", myargs) == -1) {
                fprintf(stderr, "Execvp Error: %s\n", strerror(errno));
            }
            fprintf(stderr,"This shouldn’t print out.\n");
        } else /* Parent Process */{
            
            /* Close unused file descriptors */
            if (close(CHILD_READ) == -1) {
                fprintf(stderr, "close() error: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (close(CHILD_WRITE) == -1) {
                fprintf(stderr, "close() error: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            
            /* register signal handler */
            signal(SIGPIPE, shellhalt_handler);
            
        }
    }
    
    
    static struct pollfd pollfds[2];
    setpollfd(pollfds);
    
    /* Create buffer to hold data from standard input */
    char *read_buf = NULL;
    if ((read_buf = (char*) malloc(READ_SIZE)) == NULL) {
        fprintf(stderr, "Malloc Error.\n");
        exit(1);
    };
    
    /* Read user input. Loop broken by ^D (0x04) */
    int read_len   = 0; /* Bytes returned by read */
    int poll_result; /* for multithreading */
    while (1) {
        
        poll_result = poll(pollfds, 2, timeout_msecs);
        
        /* Process input from keyboard first, if avaliable */
        if (keybd_pollfd.revents == POLLIN) {
            read_len = read(input_fd, read_buf, READ_SIZE);
            
            if (read_len == -1) {
                fprintf(stderr, "stdin read error: %s\n", strerror(errno));
                exit(1);
            }
            
            char *read_buf_index;
            read_buf_index = read_buf;
            
            /* Echo user input. Forward to shell. */
            for (int i = 0; i < read_len; i++) {
                
                switch (read_buf_index[i]) {
                        /* map received <cr> or <lf> into <cr><lf> */
                    case cr_char:
                    case lf_char:
                        /* <cr> or <lf> should echo as <cr><lf> and go to shell as <lf>. */
                        fprintf(stdout, "%c%c", cr_char, lf_char);
                        /* Prints to file descriptor */
                        dprintf(PARENT_WRITE, "%c", lf_char);
                        break;
                    /* Upon receiving an EOF (^D) from the terminal,
                        close the pipe, send a SIGHUP to the shell */
                    case ctrl_D:
                        if (kill(cpid, SIGHUP) < 0) {
                            fprintf(stderr, "kill() error: %s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                        /* Close unused file descriptors */
                        if (close(PARENT_WRITE) == -1) {
                            fprintf(stderr, "close() error: %s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                        shutdown(&original_settings);
                        exit(0);
                        break;
                    /* Upon receiving an interrupt (^C) from the terminal, send a SIGINT to the shell */
                    case ctrl_C:
                        if (kill(cpid, SIGINT) < 0) {
                            fprintf(stderr, "kill() error: %s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                        break;
                    default:
                        write(STDOUT_FILENO, (void *) &read_buf_index[i], 1);
                        dprintf(PARENT_WRITE, "%c", read_buf_index[i]);
                        break;
                }
            }
        }
        
        /* Process input from shell pipe, if avaliable */
        if (pipe_pollfd.revents == POLLIN) {
            pthread_t read_thrd;
            if ( pthread_create(&read_thrd, NULL, readpipe, PARENT_READ) < 0) {
                fprintf(stderr, "pthread_create() error: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            } else {
            /* Wait for data in pipe to be read before getting user input */
            pthread_join(read_thrd, NULL);
            }
        }
        
    }
    /* upon receiving an interrupt (^C) from the terminal, send a SIGINT to the shell
     upon receiving an EOF (^D) from the terminal, close the pipe, send a SIGHUP to the shell, restore terminal modes, and exit with return code 0. */
    
    /* Indicate unexpected control locus */
    fprintf(stderr, "Unexpected control locus\n");
    return 2;
}
