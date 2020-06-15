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
#include <netdb.h>

int timeout_msecs     = 0;
static int output_fd  = 1;
static int input_fd   = 0;

#define SERV_PORT 8000
#define SERV_ADDR "192.168.1.12"
#define PAYLOAD_SIZE 1
/* socket file_descriptor */
int sockfd;


#define SCALING_FACTOR     2
#define READ_SIZE        256
#define ctrl_C          0x03
#define ctrl_D          0x04  /* Escape Charachter */
#define cr_char         0x0D  /* Carraige Return */
#define lf_char         0x0A  /* Line Feed */

/* These buffers stored compressed and/or encrypted data
 There */
char *sent_buf;
char *recv_buf;
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

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void
setpollfd(struct pollfd *pollfds) {
#define keybd_pollfd pollfds[0]
    keybd_pollfd.fd        = STDIN_FILENO;
    keybd_pollfd.events    = POLLIN;
    keybd_pollfd.revents   = timeout_msecs;
#define sock_pollfd  pollfds[1]
    sock_pollfd.fd         = sockfd;
    sock_pollfd.events     = POLLIN;
    sock_pollfd.revents    = timeout_msecs;
}

struct termios original_settings, full_duplex_settings; /* For storing terminal settings */

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
readsock(int pipefd) {
    
    /* Create buffer to hold data from standard input */
    char *read_buf = NULL;
    if ((read_buf = (char*) malloc(READ_SIZE)) == NULL) {
        fprintf(stderr, "Malloc Error.\n");
        exit(1);
    };
    
    /* Read input from server. Loop broken by ^D (0x04) */
    int read_len = 0;
    read_len = read(sockfd, read_buf, READ_SIZE);
    if (read_len < 0) {
        fprintf(stderr, "Pipe read error: %s", strerror(errno));
        exit(1);
    } else {
        for (int i = 0; i < read_len; i++) {
            switch (read_buf[i]) {
                /* Upon receiving EOF from the shell, restore
                 terminal modes and exit with return code 1 */
                case ctrl_D:
                    restore_tty(&original_settings);
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

/*
The steps involved in establishing a socket on the client side are as follows:

Create a socket with the socket() system call
Connect the socket to the address of the server using the connect() system call
Send and receive data. There are a number of ways to do this, but the simplest is to use the read() and write() system calls.
 */

int
main(int argc, char** argv) {
    
    /* Handles terminal settings */
    tty_config();
    
    /* Create socket file_descriptor */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    
    /* Fill in Address data-structure */
    struct sockaddr_in *serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(SERV_PORT);
    serv_addr->sin_addr.s_addr = inet_addr(SERV_ADDR);
    
    /* Connects sockfd to address */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
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
        if (poll_result == -1) {
            fprintf(stderr, "%s\n", strerror(errno));
            exit(1);
        }
        
        /* Process input from keyboard first, if avaliable */
        if (keybd_pollfd.revents == POLLIN) {
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
                        dprintf(STDIN_FILENO, "%c%c", cr_char, lf_char);
                        /* Send data over network */
                        char *lf_ptr = NULL;
                        *lf_ptr = lf_char;
                        send(sockfd, lf_ptr, PAYLOAD_SIZE, 0);
                        break;
                    /* Note: If a ^D or ^D is recieved, it is forwarded to server*/
                    default:
                        /* Technically, dprintf(), send(), and write() could be used
                         interchangeably. To distinguish use cases: dprintf() is
                         used only to transfer data that is guaranteed to be on the same
                         host; send() is used only to transfer data through sockets */
                        dprintf(STDIN_FILENO, "%c", read_buf[i]);
                        send(sockfd, &read_buf[i], PAYLOAD_SIZE, 0);
                        break;
                }
            }
        }
        
        /* Process input from socket, if avaliable */
        if (sock_pollfd.revents == POLLIN) {
            pthread_t get_sock;
            if ( pthread_create(&get_sock, NULL, readsock, &sockfd) < 0) {
                fprintf(stderr, "pthread_create() error: %s\n", strerror(errno));
                exit(1);
            } else {
            /* Wait for data in pipe to be read before getting user input */
            pthread_join(get_sock, NULL);
            }
        }
        
    }
    return 0;
}

/*
The client program:
The client program will open a connection to a server (port specified with the mandatory --port= command line parameter) rather than sending it directly to a shell. The client should then send input from the keyboard to the socket (while echoing to the display), and input from the socket to the display.
Maintain the same non-canonical (character at a time, no echo) terminal behavior you used in Project 1A.
Include, in the client, a --log=filename option, which maintains a record of data sent over the socket (see below).
X If a ^D is entered on the terminal, simply pass it through to the server like any other character.
X if a ^C is entered on the terminal, simply pass it through to the server like any other character.
In the second part of this project we will add a --compress switch to the client.
If the client encounters an unrecognized argument it should print an informative usage message (on stderr) and exit with a return code of 1. If any system call fails, it should print an informative error message (on stderr) and exit with a return code of 1.
Before exiting, restore normal terminal modes.
*/


