/* TODO: Once you finish project1b specifications, data a database
   possible flags: --log, --encrypt, --tell (get pipe stats),
    the server should write to a CSV
 IMPORTANT: When both efficiency and privacy are desired,
 data is first compressed, then encrypted.
 */

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
#include <sys/socket.h> /* Core socket functions and data structures */
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <termios.h>    /* For manipulating terminal settings */
#include <sys/ioctl.h>  /* ioctl - control device */
#include <poll.h>       /* For monitoring I/0 readiness*/
#include <pthread.h>    /* On Linux, compile using cc -pthread. */
#include <stdbool.h>


#define serv_on            1
#define SCALING_FACTOR     2
#define READ_SIZE        256
#define ctrl_C          0x03
#define ctrl_D          0x04  /* Escape Charachter */
#define cr_char         0x0D  /* Carraige Return */
#define lf_char         0x0A  /* Line Feed */
#define child_limit     1000

/* Inter-Process Pipe Macros */
int pipeToChild[2]   = {-1, -1};
int pipeFromChild[2] = {-1, -1};
#define PARENT_READ    pipeFromChild[0]
#define CHILD_WRITE    pipeFromChild[1]
#define CHILD_READ     pipeToChild[0]
#define PARENT_WRITE   pipeToChild[1]

/* Networking Macros */
#define MY_PORT 8000 /* Avoid reserved ports */
#define MY_ADDR "192.168.1.12"
#define BACKLOG_LIMIT 100

int socktokern[2] = {-1, -1};
int kerntosock[2] = {-1, -1};
#define sock_to_kern_write socktokern[1]
#define sock_to_kern_read socktokern[0]
#define kern_to_sock_write socktokern[1]
#define kern_to_sock_read socktokern[0]

static int output_fd  = 1;
static int input_fd   = 0;
int buffer_size       = 1024 * sizeof(char);
int timeout_msecs     = 0;
pid_t cpid; /* Child process */
struct termios original_settings, full_duplex_settings; /* For storing terminal settings */

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
    
    /* Memory and file management :) */
    if (close(input_fd) == -1) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }
    if (close(output_fd) == -1) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }
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

/* One possible solution is to get the fd of the socket and send
 data directly to the client. However, since we must do character
 by character processing, this would result in N calls to send(),
 where N is the number of characters returned from the pipe. So,
 instead we do store and forward using the kernel_to_sock pipe as
 a buffer. */
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
                    /* Send copy of ctrl_D to kernel via the intra-process pipe*/
                    dprintf(kern_to_sock_write, "%c", read_buf[i]);
                    dprintf(STDOUT_FILENO, "%c", read_buf[i]); /* server stdout*/
                    restore_tty(&original_settings);
                    exit(1);
                    break;
                case lf_char:
                    dprintf(kern_to_sock_write, "%c%c", cr_char, lf_char);
                    dprintf(STDOUT_FILENO, "%c%c", cr_char, lf_char); /* server stdout*/
                    break;
                default:
                    dprintf(kern_to_sock_write, "%c", read_buf[i]);
                    dprintf(STDOUT_FILENO, "%c", read_buf[i]); /* server stdout*/
                    break;
            }
        }
    }
    
    return;
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


/* fork to create a new process, exec a shell (/bin/bash),
whose standard input is a pipe from the terminal process, and whose standard
output and standard error are (dups of) a pipe to the terminal process. */

pid_t
exec_prgm(char *prgm) {
    
    
    if ((pipe(pipeToChild) == -1) || (pipe(pipeFromChild) == -1)) {
        fprintf(stderr, "Cannot create pipes: %s\n", strerror(errno));
        exit(1);
    }
    
    if ( (cpid = fork()) < 0) {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        exit(1);
    }
    
    /* Child Process */
    if (cpid == 0) {
        
        /* Redirects stdin, stdout, and stderr of child process */
        setstdstreams();
        
        char *myargs[2];
        myargs[0] = prgm;    /* program: "bash" (shell) */
        myargs[1] = NULL;                   /* marks end of array */
        if (execvp(prgm, myargs) == -1) {
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
        
    }
    return cpid;
}


void
setpollfd(struct pollfd *pollfds) {
#define sock_pollfd pollfds[0]
    sock_pollfd.fd           = sock_to_kern_read;
    sock_pollfd.events       = POLLIN;
    sock_pollfd.revents      = timeout_msecs;
#define pipe_pollfd pollfds[1]
    pipe_pollfd.fd           = PARENT_READ;
    pipe_pollfd.events       = POLLIN;
    pipe_pollfd.revents      = timeout_msecs;
}

void
sign_handler(int signal) {
    restore_tty(&original_settings);
    exit(1);
}

/* TODO: further functional decomposition necessary
   TODO: server-side no longer needs to echo to stdout */
void
echo_and_forwrd(pid_t cpid) {
    /* Create buffer to hold data from standard input */
    
    char *read_buf = NULL;
    if ((read_buf = (char*) malloc(READ_SIZE)) == NULL) {
        fprintf(stderr, "Malloc Error.\n");
        exit(1);
    };
    
    static struct pollfd pollfds[2];
    setpollfd(pollfds);
    int poll_result;
    int read_len   = 0; /* Bytes returned by read */
    /* Loop broken by ^D or ^C (distally) */
    while (true) {
        
        poll_result = poll(pollfds, 2, timeout_msecs);
        
        /* Retrieve network packet buffered in kernel, if avaliable */
        if (sock_pollfd.revents == POLLIN) {
            read_len = read(sock_to_kern_read, read_buf, READ_SIZE);
            
            if (read_len == -1) {
                fprintf(stderr, "stdin read error: %s\n", strerror(errno));
                exit(1);
            }
            
            char *read_buf_index;
            read_buf_index = read_buf;
            
            /* Forward user input to shell, one-character-at-a-time. */
            for (int i = 0; i < read_len; i++) {
                
                switch (read_buf_index[i]) {
                        /* map received <cr> or <lf> into <cr><lf> */
                    case cr_char:
                    case lf_char:
                        /* <cr> or <lf> should echo as <cr><lf> and go to shell as <lf>. */
                        fprintf(stdout, "%c%c", cr_char, lf_char);
                        dprintf(PARENT_WRITE, "%c", lf_char);
                        break;
                    /* Upon receiving an EOF (^D) from the terminal,
                        close the pipe, send a SIGHUP to the shell */
                    case ctrl_D:
                        if (kill(cpid, SIGHUP) < 0) {
                            fprintf(stderr, "kill() error: %s\n", strerror(errno));
                            exit(1);
                        }
                        /* Close unused file descriptors */
                        if (close(PARENT_WRITE) == -1) {
                            fprintf(stderr, "close() error: %s\n", strerror(errno));
                            exit(1);
                        }
                        restore_tty(&original_settings);
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
            if ( pthread_create(&read_thrd, NULL, (void *) readpipe, (void *) PARENT_READ) < 0 ) {
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
    
    
}

void
start_duplex_tty() {
    /* Fork and execute input parm */
     pid_t cpid = exec_prgm("/bin/bash");
    
    /* Echo user input stdout, foward to shell, send/recieve signals.
     contains while(true) loop that terminates on specified user inputs*/
    echo_and_forwrd(cpid);
}

void
thrd_fork() {
    pthread_t read_thrd;
    if ( pthread_create(&read_thrd, NULL, (void *) start_duplex_tty, NULL) < 0 ) {
        fprintf(stderr, "pthread_create() error: %s\n", strerror(errno));
        exit(1);
    }
    return;
}

void
transmit_to_client(int sockfd) {
    
    static struct pollfd pollfds[1];
    #define kern_pollfd pollfds[0]
    kern_pollfd.fd      = kern_to_sock_read;
    kern_pollfd.events  = POLLIN;
    kern_pollfd.revents = timeout_msecs;
    
    char trans_buf[1024]; /* To hold data from kernel */
    int poll_result, read_len;
    while (true) {
        /* Check if shell has output for client */
        poll_result = poll(pollfds, 1, timeout_msecs);
        
        /* Retrieve network packet buffered in kernel, if avaliable */
        if (kern_pollfd.revents == POLLIN) {
            read_len = read(sock_to_kern_read, trans_buf, READ_SIZE);
            
            if (read_len == -1) {
                fprintf(stderr, "kern_read() error: %s\n", strerror(errno));
                exit(1);
            }
        
        int send_status;
        send_status = send(sockfd, trans_buf, read_len, 0);
            if (send_status == - 1) {
                fprintf(stderr, "send() error: %s\n", strerror(errno));
                exit(1);
            }
        }
    }
    fprintf(stderr, "Expected break from while(true) \n");
    exit(2);
}


int
main(int argc, char** argv) {
    
    int sock_listen, sock_connect; /* listen on sock_listen, new connection on new_connect */
    struct sockaddr_in my_addr; /* client-host address data-structure */
    struct sockaddr_in their_addr; /* server-host address data-structure */
    unsigned int sin_size = sizeof (their_addr);
    
    /* Assign values to the members of my_addr, which is a phony data structure used
     to associate an IP address and port number with a socket */
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(MY_PORT);  /* Host-TO-Network-Short coverserion */
    // TODO: Need to use a system call to get IP
    my_addr.sin_addr.s_addr = inet_addr(MY_ADDR);
    
    /* socket() creates a new TCP socket with nternet Protocol v4 addresses */
    if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    /* bind() is typically used on the server side, and associates a socket
     with a socket address structure, i.e. a specified local IP address
     and a port number. */
    if (bind(sock_listen,(struct sockaddr *) &my_addr, sizeof (my_addr)) == -1)
    {
        perror ("bind");
        exit (1);
    }
    
    /* The listen function enables the server socket (i.e., sock_listen),
     to accept connections*/
    if (listen(sock_listen, BACKLOG_LIMIT) == -1) {
        perror("listen");
        exit(1);
    }
    
    /* IMPORTANT: A socket that has been established as a server can accept connection
     requests from multiple clients. The server's original socket does not become part
     of the connection; instead, accept makes a new socket which participates in the
     connection. accept returns the descriptor for this socket. The server's original
     socket remains available for listening for further connection requests. The
     while-loop below is the main accept loop of the server. */
    while (serv_on) {
        /* NOTE: client_addr and addrlen are result arguments The program passes
         empty client_addr and addrlen into the function, and the kernel will fill
         in these arguments with client’s information. */
        if ((sock_connect = accept(sock_listen, (struct sockaddr*)
                                   &their_addr, &sin_size)) == -1) {
            fprintf(stderr, "%s\n", "accept");
            continue;
            
        }
        
        /* We've established a connect with a clinet at this point. We need to recv()
         their message and process it. */
        if (sock_connect) {
            
            char message_buf[1024]; // buffer for incoming message.
            int connection; // value to hold connection status
            
            /* recv() returns the number of bytes actually read into the buffer, or -1 on error */
            connection = recv(sock_connect, message_buf, sizeof(message_buf), 0);
            
            /* Immediately forward data to kernel so it can be piped to shell*/
            dprintf(sock_to_kern_write, "%s", message_buf);
            
            
            if (connection == -1) {
                fprintf(stderr, "%s\n", strerror(errno));
                exit(1);
            }
            /* recv() can return 0. This can mean only one thing: the remote side has
             closed the connection on you! A return value of 0 is recv()’s way of
             letting you know this has occurred. */
            else if (connection == 0) {
                fprintf(stderr, "%s\n", strerror(errno));
                exit(1);
            }
            
            /* Process the request. */
            else {

                /* Handles terminal settings */
                // tty_config();
                
                /* handles fork */
                thrd_fork();
                
                /* Create thread to send() shell output to client */
                pthread_t read_kern;
                if ( pthread_create(&read_kern, NULL, (void *) transmit_to_client, &sock_connect) < 0 ) {
                       fprintf(stderr, "pthread_create() error: %s\n", strerror(errno));
                       exit(EXIT_FAILURE);
                   }
                
                // Close the conection
                // close(sock_connect);
            }
        }
        
    }
    
    
    
    /* Indicate unexpected control locus */
    fprintf(stderr, "Unexpected control locus\n");
    return 2;
}
