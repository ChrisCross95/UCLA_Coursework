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
#include <mcrypt.h>
#include "netdefs.h"
#include "zlib.h"
#include <assert.h>
#include <sys/wait.h>


#define serv_on             1
#define SCALING_FACTOR      2
#define READ_SIZE       16384  /* 8kB*/
#define ctrl_C           0x03
#define ctrl_D           0x04  /* Escape Charachter */
#define cr_char          0x0D  /* Carraige Return */
#define lf_char          0x0A  /* Line Feed */
#define child_limit      1000

/* Inter-Process Pipe Macros */
int pipeToChild[2]   = {-1, -1};
int pipeFromChild[2] = {-1, -1};
#define PARENT_READ    pipeFromChild[0]
#define CHILD_WRITE    pipeFromChild[1]
#define CHILD_READ     pipeToChild[0]
#define PARENT_WRITE   pipeToChild[1]

/* Networking Macros */
int sock_listen, sock_connect;
struct sockaddr_in my_addr; /* client-host address data-structure */
struct sockaddr_in their_addr; /* server-host address data-structure */
#define BACKLOG_LIMIT 100

int SERV_PORT; 

int socktokern[2] = {-1, -1};
int kerntosock[2] = {-1, -1};
#define sock_to_kern_write socktokern[1]
#define sock_to_kern_read  socktokern[0]
#define kern_to_sock_write kerntosock[1]
#define kern_to_sock_read  kerntosock[0]

int compress_flag     = 0;
int port_flag         = 0;
int shell_flag        = 0;
static int output_fd  = 1;
static int input_fd   = 0;
int buffer_size       = 1024 * sizeof(char);
int timeout_msecs     = 0;
pid_t cpid; /* Child process */
struct termios original_settings, full_duplex_settings; /* For storing terminal settings */

char *use_msg = "Correct usage: lab1b-server --port [--compress][--debug]\n";
char *pgrm_name;

void
segfault_handler(int sig) {
    printf("accessing null pointer\n");
    exit(1);
}

void
opt_handler(int argc, char** argv) {
    
    
    char returned_option;
    while (1)
    {
        static struct option long_options[] =
        {
            {"port",     required_argument,    &port_flag,      1},
            {"shell",    required_argument,    &shell_flag,     1},
            {"compress", no_argument,          &compress_flag,  1},
            /* Terminate the array with an element containing all zeros. */
            {0, 0, 0, 0}
        };
        
        int option_index = 0; /* Updated by getopt_long() with the index of the option */
        returned_option = getopt_long(argc, argv, "p:s:c",
                                      long_options, &option_index);
        
        /* Detect the end of the options. */
        if (returned_option == -1) { break; }
        
        switch(returned_option)
        {
            case 'p':
                port_flag = 1;
                /* a zero base is taken as 10 (decimal) */
                SERV_PORT = strtol(optarg, NULL, 0);
                fprintf(stderr, "Server Port specified: %d\n", SERV_PORT);
                break;
            case 's':
                shell_flag = 1;
                pgrm_name = optarg;
                break;
            case 'c':
                compress_flag =1;
                fprintf(stderr, "Compression Enabled.\n");
                break;
            default:    //case '?':
                fprintf(stderr, "%s\n", use_msg);
                exit(1);  // for unrecognized argument
        }
    }
    
    if (!port_flag) {
        fprintf(stderr, "--port=<port number> is a mandatory option\n");
        exit(1);
    }
    return; 
    
}



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

void
compress_wrapper(char *input_buf) {
    
    //    printf("compressing the following: %c%c%c", input_char, cr_char, lf_char);
    
    char *def_src_buf  = input_buf;
    char *def_dest_buf = (char *) malloc(READ_SIZE * sizeof(char));
    if( (def_src_buf == NULL) || (def_dest_buf == NULL) ){
        fprintf(stderr, "alloc error in compress_wrapper(): %s\n", strerror(errno));
        exit(1);
    }
    
    // zlib struct
    z_stream defstream;
    defstream.zalloc    = Z_NULL;
    defstream.zfree     = Z_NULL;
    defstream.opaque    = Z_NULL;
    // setup "a" as the input and "b" as the compressed output
    defstream.avail_in  = (uInt) strlen(def_src_buf) + 1; // size of input, string + terminator
    defstream.next_in   = (Bytef *) def_src_buf; // input char array
    defstream.avail_out = (uInt) READ_SIZE; // size of output
    defstream.next_out  = (Bytef *) def_dest_buf; // output char array
    
    // the actual compression work.
    deflateInit(&defstream, Z_BEST_COMPRESSION);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);
    
    // This is one way of getting the size of the output
    //    printf("Compressed size is: %lu\n", strlen(def_dest_buf));
    //    printf("%c%c", cr_char, lf_char);
    //    printf("Compressed string is: %s\n", def_dest_buf);
    //    printf("%c%c", cr_char, lf_char);
    
    int sent_len;
    sent_len = send(sock_connect, def_dest_buf, strlen(def_dest_buf), 0);
    
    fprintf(stderr, "Bytes sent: %d\n", sent_len);
    return;
}

void
transmit_to_client(int sockfd) {
    
    fprintf(stderr, "Sending to client\n");
    char *trans_buf = (char *) malloc(READ_SIZE * sizeof(char)); /* To hold data from kernel */
    int read_len;
    
    /* Retrieve network packet buffered in kernel, if avaliable */
    read_len = read(kern_to_sock_read, trans_buf, READ_SIZE);
    fprintf(stderr, "Bytes recieved from shell: %d\n", read_len);
    if (read_len == -1) {
        fprintf(stderr, "kern_read() error: %s\n", strerror(errno));
        exit(1);
    }
    
    /* compress_wrapper will transmit the compressed msg to client*/
    if (compress_flag) {
        compress_wrapper(trans_buf);
        return;
    }
    
    int send_status;
    send_status = send(sockfd, trans_buf, read_len, 0);
    fprintf(stderr, "Bytes sent to client: %d\n", send_status);
    if (send_status == - 1) {
        fprintf(stderr, "send() error: %s\n", strerror(errno));
        exit(1);
    }
    return;
}

void
readsock(char *message_buf) {
    int connection; // value to hold connection status
    
    /* recv() returns the number of bytes actually read into the buffer, or -1 on error */
    connection = recv(sock_connect, message_buf, sizeof(message_buf), 0);
    
    printf("Bytes received: %d\n", connection);
    /* Immediately forward data to kernel so it can be piped to shell*/
    dprintf(sock_to_kern_write, "%s", message_buf);
    fprintf(stderr, "%s\n", message_buf); /* for debugging */
    
    if (connection == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    /* recv() can return 0. This can mean only one thing: the remote side has
     closed the connection on you! A return value of 0 is recv()’s way of
     letting you know this has occurred. */
    if (connection == 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    return;
    
}

/* Report the exit status of shell child process. This function does not return.
 This is called if the server process exits for any reason*/
void
report_shell_status() {
    
    int child_status;
    if (waitpid(cpid, &child_status, 0) < 0) {
        fprintf(stderr, "Error waiting for the child process: %s", strerror(errno));
        exit(1);
    }
    int signal_code = child_status & 0x7f;
    int status_code = WEXITSTATUS(child_status);
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", signal_code, status_code);

    /* Kill the child process */
    kill(cpid, SIGKILL);
    
    close(sock_connect);
    
}

/* One possible solution is to get the fd of the socket and send
 data directly to the client. However, since we must do character
 by character processing, this would result in N calls to send(),
 where N is the number of characters returned from the pipe. So,
 instead we do store and forward using the kernel_to_sock pipe as
 a buffer. */
void
get_shell_output(int *pipefdp) {
    
    int pipefd = *pipefdp;
    
    /* Create buffer to hold data from shell */
    char *read_buf = NULL;
    if ((read_buf = (char*) malloc(READ_SIZE)) == NULL) {
        fprintf(stderr, "Malloc Error.\n");
        exit(1);
    };
    
    /* Read shell output. Loop broken by ^D (0x04) */
    int read_len = 0;
    read_len = read(pipefd, read_buf, READ_SIZE);
    if (read_len < 0) {
        fprintf(stderr, "Pipe read error: %s", strerror(errno));
        exit(1);
    }
    
    for (int i = 0; i < read_len; i++) {
        switch (read_buf[i]) {
                /* Upon receiving EOF from the shell, restore
                 terminal modes and exit with return code 1 */
            case ctrl_D:
                /* Send copy of ctrl_D to kernel via the intra-process pipe*/
                dprintf(kern_to_sock_write, "%c", read_buf[i]);
                dprintf(STDOUT_FILENO, "%c", read_buf[i]); /* server stdout*/
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
    
    
    /* We should send() to client here, after we get data from the shell
     send_to_client() */
    transmit_to_client(sock_connect);
    
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
    exit(1);
}

/* This method maps a compressed string to an uncompressed character.
 Because characters are trasmitted from the client one at a time,
 compression actually inflates the data. */
char
decompress_wrapper(char *input_buf) {
    printf("decompressing.\n");
    
    char *inf_src_buf  = input_buf;
    char *inf_dest_buf = (char *) malloc(20 * sizeof(char)); /* compression of single char inflates */
    if( (inf_src_buf == NULL) || (inf_dest_buf == NULL) ){
        fprintf(stderr, "alloc error in compress_wrapper(): %s\n", strerror(errno));
        exit(1);
    }
    
    // inflate b into c
    // zlib struct
    z_stream infstream;
    infstream.zalloc    = Z_NULL;
    infstream.zfree     = Z_NULL;
    infstream.opaque    = Z_NULL;
    // setup "b" as the input and "c" as the compressed output
    infstream.avail_in  = (uInt) sizeof(inf_src_buf); // size of input
    infstream.next_in   = (Bytef *) inf_src_buf; // input char array
    infstream.avail_out = (uInt) sizeof(inf_dest_buf); // size of output
    infstream.next_out  = (Bytef *) inf_dest_buf; // output char array
    
    // the actual DE-compression work.
    inflateInit(&infstream);
    inflate(&infstream, Z_FINISH);
    inflateEnd(&infstream);
    
    printf("Uncompressed size is: %lu\n", strlen(inf_dest_buf));
    printf("Uncompressed string is: %s\n", inf_dest_buf);
    
    return inf_dest_buf[0];
    
}



/* TODO: further functional decomposition (or encapsulation) necessary
 TODO: server-side no longer needs to echo to stdout */
void
echo_and_forwrd(pid_t cpid) {
    /* Create buffer to hold data from socket */
    char *read_buf = NULL;
    if ((read_buf = (char*) malloc(READ_SIZE)) == NULL) {
        fprintf(stderr, "Malloc Error.\n");
        exit(1);
    }
    
    if ((pipe(socktokern) == -1) || (pipe(kerntosock) == -1)) {
        fprintf(stderr, "Cannot create pipes: %s\n", strerror(errno));
        exit(1);
    }
    
    static struct pollfd pollfds[2];
    setpollfd(pollfds);
    // int poll_result;
    int read_len   = 0; /* Bytes returned by read */
    /* Loop broken by ^D or ^C (distally) */
    while (serv_on) {
        char message_buf[1024]; /* buffer for incoming client data */
        readsock(message_buf); /* Check socket */
        
        if ( poll(pollfds, 2, timeout_msecs) < 0) {
            fprintf(stderr, "poll() error: %s\n", strerror(errno));
            exit(1);
        };
        /* Retrieve network packet buffered in kernel, if avaliable */
        if (sock_pollfd.revents == POLLIN) {
            read_len = read(sock_to_kern_read, read_buf, READ_SIZE);
            if (read_len < 0) {
                close(PARENT_WRITE);
                
            }
            fprintf(stderr, "compressed data length: %d\n", read_len);
            fprintf(stderr, "compressed data: %s\n", read_buf);
        }
        if (read_len == -1) {
            fprintf(stderr, "stdin read error: %s\n", strerror(errno));
            exit(1);
        }
        
        
        char recv_char = read_buf[0];
        if (compress_flag) {
            recv_char = decompress_wrapper(read_buf);
        }
        
        /* Forward user input to shell, one-character-at-a-time. */
        switch (recv_char) {
                /* map received <cr> or <lf> into <cr><lf> */
            case cr_char:
            case lf_char:
                /* <cr> or <lf> should echo as <cr><lf> and go to shell as <lf>. */
                fprintf(stdout, "%c%c", cr_char, lf_char);
                dprintf(PARENT_WRITE, "%c", lf_char);
                printf("sent linefeed to shell\n");
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
                break;
                /* Upon receiving an interrupt (^C) from the terminal, send a SIGINT to the shell */
            case ctrl_C:
                if (kill(cpid, SIGINT) < 0) {
                    fprintf(stderr, "kill() error: %s\n", strerror(errno));
                    exit(1);
                }
                /* Close unused file descriptors */
                if (close(PARENT_WRITE) == -1) {
                    fprintf(stderr, "close() error: %s\n", strerror(errno));
                    exit(1);
                }
                break;
            default:
                fprintf(stdout, "%c", recv_char);
                dprintf(PARENT_WRITE, "%c", recv_char);
                printf("character sent to shell\n");
                break;
        }
        
        
        /* Process input from shell pipe, if avaliable */
        if (pipe_pollfd.revents & POLLIN) {
            printf("Shell data avalable.\n");
            pthread_t read_thrd;
            if ( pthread_create(&read_thrd, NULL, (void *) get_shell_output, &PARENT_READ) < 0 ) {
                fprintf(stderr, "pthread_create() error: %s\n", strerror(errno));
                exit(1);
            }
        }
        if (pipe_pollfd.revents & (POLLERR | POLLHUP)) {
            exit(1);
        }
        
    }
    /* Indicate unexpected control locus */
    fprintf(stderr, "Unexpected control locus\n");
    exit(2);
}

void
start_duplex_tty() {
    /* Fork and execute input parm */
    pid_t cpid = exec_prgm(pgrm_name);
    
    /* Echo user input stdout, foward to shell, send/recieve signals.
     contains while(true) loop that terminates on specified user inputs*/
    echo_and_forwrd(cpid);
}


int
main(int argc, char** argv) {

    fprintf(stderr, "Initiating Server...\n");
    
    /* Process options */
    opt_handler(argc, argv);
    fprintf(stderr, "Options processed...\n");
    
    atexit(report_shell_status);
    
    unsigned int sin_size = sizeof (their_addr);
    
    /* Assign values to the members of my_addr, which is a phony data structure used
     to associate an IP address and port number with a socket */
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(SERV_PORT);  /* Host-TO-Network-Short coverserion */
    // TODO: Need to use a system call to get IP
    my_addr.sin_addr.s_addr = inet_addr(SERV_ADDR);
    
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
    
    fprintf(stderr, "Server intiated. Now accepting connections on port %d\n", SERV_PORT);
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
            
            printf("Connection established on port: %d\n", SERV_PORT);
            printf("Client IP address: %s\n", inet_ntoa(their_addr.sin_addr));
            printf("Client port: %d\n", (int) ntohs(their_addr.sin_port));
            
            /* Create tread to recieve client input and forward to shell */
            pthread_t read_thrd;
            if ( pthread_create(&read_thrd, NULL, (void *) start_duplex_tty, NULL) < 0 ) {
                fprintf(stderr, "pthread_create() error: %s\n", strerror(errno));
                exit(1);
            }
            
        }
    }
    /* Indicate unexpected control locus */
    fprintf(stderr, "Unexpected control locus\n");
    exit(2);
}
