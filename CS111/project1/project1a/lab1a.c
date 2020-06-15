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
#include <sys/wait.h>

/* Pipefd[0] refers to the read end of the pipe. Pipefd[1] refers
 to the write end of the pipe.*/
int pipe_to_chld[2]   = {-1, -1};
int pipe_to_par[2] = {-1, -1};
#define PARENT_READ    pipe_to_par[0]
#define CHILD_WRITE    pipe_to_par[1]
#define CHILD_READ     pipe_to_chld[0]
#define PARENT_WRITE   pipe_to_chld[1]

#define READ_SIZE        256
#define ctrl_C          0x03
#define ctrl_D          0x04  /* Escape Charachter */
#define cr_char         0x0D  /* Carraige Return */
#define lf_char         0x0A  /* Line Feed */

static int shell_flag = 0; /* Flag set by '--shell'. */
int timeout_msecs     = 0;

char *use_msg = "Correct usage: lab1a [--shell]\n";
char *prgm = "/bin/bash";
char *prgm_name;
pid_t cpid;

/* For storing terminal settings */
struct termios original_settings, full_duplex_settings;
struct pollfd pollfds[2];

void
set_term_attr(struct termios *termios_p) {
    if (termios_p == NULL) {
        fprintf(stderr,"Error: empty reference\n");
        exit(1);
    }
    termios_p->c_iflag = ISTRIP;
    termios_p->c_oflag  = 0;
    termios_p->c_lflag  = 0;
    return;
}

void
restore_tty(struct termios *original_settings) {
    if (original_settings == NULL) {
        fprintf(stderr,"Error: empty reference\n");
        exit(EXIT_FAILURE);
    }
    if (tcsetattr(STDIN_FILENO, TCSANOW, original_settings) < 0) {
        fprintf(stderr, "tcsetattr() error: %s\n", strerror(errno));
        return;
    }
    fprintf(stderr, "%c%c(ICANON set)\n", cr_char, lf_char);
    
    return;
}

void
shutdown() {
    
    if (shell_flag) {
        int child_status;
        if (waitpid(cpid, &child_status, 0) < 0) {
            fprintf(stderr, "Error waiting for the child process: %s", strerror(errno));
            exit(1);
        }
        int signal_code = child_status & 0x7f;
        int status_code = WEXITSTATUS(child_status);
        fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", signal_code, status_code);
    }
    restore_tty(&original_settings);
    return;
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
read_pipe(int pipefd) {
    
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
                    exit(1);
                    break;
                case lf_char:
                    dprintf(STDOUT_FILENO, "%c%c", cr_char, lf_char);
                    break;
                default:
                    dprintf(STDOUT_FILENO, "%c", read_buf[i]);
                    break;
            }
        }
    }
    
    return;
}


void
set_poll(struct pollfd *pollfds) {
#define keybd_pollfd pollfds[0]
    keybd_pollfd.fd      = STDIN_FILENO;
    keybd_pollfd.events  = POLLIN | POLLHUP | POLLERR;
    keybd_pollfd.revents = 0;
#define pipe_pollfd  pollfds[1]
    pipe_pollfd.fd      = PARENT_READ;
    pipe_pollfd.events  = POLLIN | POLLHUP | POLLERR;
    pipe_pollfd.revents = 0;
}

void
shellhalt_handler(int signal) {
    exit(0);
}

void
tty_config() {
    
    /* Get the current terminal modes, save them for
     restoration, and then make a copy */
    
    /* isatty - test whether a file descriptor refers to a terminal
     https://www.gnu.org/software/libc/manual/html_node/Noncanon-Example.html
     */
    if (isatty(STDIN_FILENO) == 0) {
        fprintf(stderr, "isatty() error: %s\n", strerror(errno));
        exit(1);
    }
    
    if (tcgetattr(STDIN_FILENO, &original_settings) < 0) {
        fprintf(stderr, "tcgetattr() error: %s\n", strerror(errno));
        exit(1);
    }
    
    if (tcgetattr(STDIN_FILENO, &full_duplex_settings) < 0) {
        fprintf(stderr, "tcgetattr() error: %s\n", strerror(errno));
        exit(1);
    }
    
    /* Load non-canonical settings */
    set_term_attr(&full_duplex_settings);
    
    /* Place terminal in non-canonical mode */
    if (tcsetattr(0, TCSANOW, &full_duplex_settings) < 0) {
        fprintf(stderr, "tcsetattr() Error: %s\n", strerror(errno));
        exit(1);
    } else {
        fprintf(stderr, "(ICANON unset)%c%c", cr_char, lf_char);
    }
    
    return;
}

void
get_opts(int argc, char** argv) {
    
    int returned_option;
    while (1)
    {
        static struct option long_options[] =
        {
            /* --shell argument to pass input/output
             between the terminal and a shell */
            {"shell",     required_argument,    NULL,  's'},
            /* Terminate the array with an element containing all zeros. */
            {0, 0, 0, 0}
        };
        
        int option_index = 0; /* Updated by getopt_long() */
        returned_option = getopt_long(argc, argv, "s:",
                                      long_options, &option_index);
        
        /* Detect the end of the options. */
        if (returned_option == -1) { break; }
        
        switch (returned_option) {
            case 's':
                shell_flag = 1;
                prgm = strdup((char*) optarg);
                break;
            case '?':
                fprintf(stderr, "%s\n", use_msg);
                exit(1);
                break;
            default:
                exit(1);
        }
    }
    return;
}

/* Shell option evoked: fork to create a new process, exec a shell (/bin/bash),
 whose standard input is a pipe from the terminal process, and whose standard
 output and standard error are (dups of) a pipe to the terminal process. */
void
start_shell() {
    
    if ((pipe(pipe_to_chld) == -1) || (pipe(pipe_to_par) == -1)) {
        fprintf(stderr, "Cannot create pipes: %s\n", strerror(errno));
        exit(1);
    }
    
    if ( (cpid = fork()) < 0) {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        exit(1);
    }
    
    /* TODO: read about execlp */
    /* Child Process */
    if (cpid == 0) {
        
        /* Redirects stdin, stdout, and stderr of child process */
        setstdstreams();
        
        char *myargs[2];
        myargs[0] = strdup(prgm);
        myargs[1] = NULL;
        if (execvp(prgm, myargs) == -1) {
            fprintf(stderr, "Execvp Error: %s\n", strerror(errno));
        }
        fprintf(stderr,"This shouldn’t print out.\n");
    } else /* Parent Process */{
        
        /* Close unused file descriptors */
        if (close(CHILD_READ) == -1) {
            fprintf(stderr, "close() error: %s\n", strerror(errno));
            exit(1);
        }
        if (close(CHILD_WRITE) == -1) {
            fprintf(stderr, "close() error: %s\n", strerror(errno));
            exit(1);
        }
        
        /* register signal handler */
        signal(SIGPIPE, shellhalt_handler);
        
    }
    
    return;
}

void
echo_and_forward() {
    
    
    set_poll(pollfds);
    char read_buf[READ_SIZE];
    int read_len  = 0;
    while (1) {
        
        int poll_result;
        poll_result = poll(pollfds, 2, timeout_msecs);
        if (poll_result == -1) {
            fprintf(stderr, "poll() error: %s\n", strerror(errno));
            exit(1);
        }
        
        if ( keybd_pollfd.revents & (POLLERR | POLLHUP) ) {
            exit(1);
        }
        /* Process input from keyboard first, if avaliable */
        if (keybd_pollfd.revents & POLLIN) {
            
            read_len = read(STDIN_FILENO, read_buf, READ_SIZE);
            
            if (read_len == -1) {
                fprintf(stderr, "stdin read error: %s\n", strerror(errno));
                exit(1);
            }
            
            /* Echo user input. Forward to shell. */
            for (int i = 0; i < read_len; i++) {
                
                switch (read_buf[i]) {
                        /* map received <cr> or <lf> into <cr><lf> */
                    case cr_char:
                    case lf_char:
                        /* <cr> or <lf> should echo as <cr><lf> and go to shell as <lf>. */
                        dprintf(STDOUT_FILENO, "%c%c", cr_char, lf_char);
                        /* Prints to file descriptor */
                        if (shell_flag) { dprintf(PARENT_WRITE, "%c", lf_char); }
                        break;
                        /* Upon receiving an EOF (^D) from the terminal,
                         close the pipe, send a SIGHUP to the shell */
                    case ctrl_D:
                        /* close the pipe to the shell */
                        if (close(PARENT_WRITE) == -1) {
                            fprintf(stderr, "close() error: %s\n", strerror(errno));
                            exit(1);
                        }
                        break;
                        /* Upon receiving an interrupt (^C) from the terminal, send a SIGINT to the shell */
                    case ctrl_C:
                        if (kill(cpid, SIGINT) < 0) {
                            fprintf(stderr, "kill() error: %s\n", strerror(errno));
                            exit(1);
                        }
                        break;
                    default:
                        dprintf(STDOUT_FILENO, "%c", read_buf[i]);
                        if (shell_flag) { dprintf(PARENT_WRITE, "%c", read_buf[i]); }
                        break;
                }
            }
        }
        
        if (shell_flag) {
            /* Process input from shell pipe, if avaliable */
            if (pipe_pollfd.revents & POLLIN) {
                read_pipe(PARENT_READ);
            }
            if ( pipe_pollfd.revents & (POLLERR | POLLHUP) ) {
                exit(1);
            }
        }
    }
    return;
}


int
main(int argc, char** argv) {
    
    get_opts(argc, argv);
    
    if (shell_flag == 1) { start_shell(); }
    
    tty_config();
    atexit(shutdown);
    
    echo_and_forward();
    
    exit(2);
}
