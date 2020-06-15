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

z_stream inbound;
z_stream outbound;

int compress_flag     = 0;
int port_flag         = 0;
int buffer_size       = 1024 * sizeof(char);
int timeout_msecs     = 0;
pid_t cpid; /* Child process */
struct termios original_settings, full_duplex_settings; /* For storing terminal settings */

char *use_msg = "Correct usage: lab1b-server --port [--compress][--shell]\n";
char *prgm = "/bin/bash";
char *prgm_name;

void zip_init() {
    /* Decompressor */
    inbound.zalloc = NULL;
    inbound.zfree = NULL;
    inbound.opaque = NULL;
    if (inflateInit(&inbound) < 0){
        fprintf(stderr, "Failed to initialize compression\n");
        exit(1);
    }
    /* Compressor */
    outbound.zalloc = NULL;
    outbound.zfree = NULL;
    outbound.opaque = NULL;
    if (deflateInit(&outbound, Z_DEFAULT_COMPRESSION) < 0){
        fprintf(stderr, "Failed to initialize compression\n");
        exit(1);
    }
    
    return;
}


void
opt_handler(int argc, char** argv) {
    
    char returned_option;
    while (1)
    {
        static struct option long_options[] =
        {
            {"port",      required_argument,    NULL,  'p'},
            {"shell",     required_argument,    NULL,  's'},
            {"compress",  no_argument,          NULL,  'c'},
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
                prgm_name = (char*) malloc(strlen((char*) optarg));
                memcpy(prgm_name, optarg, strlen((char*) optarg));
                fprintf(stderr, "Pass prgm_name: %s\n", prgm_name);
                fprintf(stderr, "prgm_name len: %lu\n", strlen((char*) optarg));
                prgm = (char *) prgm_name;
                fprintf(stderr, "Shell option evoked\n");
                break; 
            case 'c':
                compress_flag = 1;
                zip_init();
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
get_and_forward_shell_output(int pipefd) {
    
    printf("Getting and forwarding shell output \n");
    
    /* Create buffer to hold data from shell */
    char read_buf[READ_SIZE];
    char zip_buf[READ_SIZE];
    
    /* Read shell output */
    int read_len = 0;
    read_len = read(pipefd, read_buf, READ_SIZE);
    if (read_len < 0) {
        fprintf(stderr, "Pipe read error: %s", strerror(errno));
        exit(1);
    }
    
    /* Display on server for debugging purposes */
    for (int i = 0; i < read_len; i++) {
        switch (read_buf[i]) {
                /* Upon receiving EOF from the shell, restore
                 terminal modes and exit with return code 1 */
            case ctrl_D:
                /* Send copy of ctrl_D to kernel via the intra-process pipe*/
                dprintf(kern_to_sock_write, "%c", read_buf[i]);
                dprintf(STDOUT_FILENO, "%c", read_buf[i]); /* server stdout*/
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
    
    /* Transmit (and maybe compress) to client locally */
    
    if (compress_flag) {
        int zip_size;
        outbound.avail_in  = (uInt) read_len;
        outbound.next_in   = (Bytef *) read_buf;
        outbound.avail_out = READ_SIZE;
        outbound.next_out  = (Bytef *) zip_buf;
        do {
            int def_ret = deflate(&outbound, Z_SYNC_FLUSH);
            if (def_ret == Z_STREAM_ERROR) {
                fprintf(stderr, "deflate() error: %s", outbound.msg);
                exit(1);
            }
        } while (outbound.avail_in > 0);
        zip_size = READ_SIZE - outbound.avail_out;
        memcpy(read_buf, zip_buf, zip_size);
        read_len = zip_size;
    }
    
    int send_status;
    send_status = send(sock_connect, read_buf, read_len, 0);
    fprintf(stderr, "Bytes sent to client: %d\n", send_status);
    if (send_status == - 1) {
        fprintf(stderr, "send() error: %s\n", strerror(errno));
        exit(1);
    }
    return;
}

/* Report the exit status of shell child process. This function does not return.
 This is called if the server process exits for any reason*/
void
shutdown_server() {
    
    int child_status;
    if (waitpid(cpid, &child_status, 0) < 0) {
        fprintf(stderr, "Error waiting for the child process: %s", strerror(errno));
        exit(1);
    }
    int signal_code = child_status & 0x7f;
    int status_code = WEXITSTATUS(child_status);
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", signal_code, status_code);
    
    if (compress_flag) {
        deflateEnd(&outbound);
        inflateEnd(&inbound);
    }
    
    /* Kill the child process */
    kill(cpid, SIGKILL);
    
    close(sock_connect);
    
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
        exit(1);
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
    sock_pollfd.fd           = sock_connect;
    sock_pollfd.events       = POLLIN | POLLHUP | POLLERR;
    sock_pollfd.revents      = timeout_msecs;
#define pipe_pollfd pollfds[1]
    pipe_pollfd.fd           = PARENT_READ;
    pipe_pollfd.events       = POLLIN | POLLHUP | POLLERR;
    pipe_pollfd.revents      = timeout_msecs;
}


void
echo_and_forwrd(pid_t cpid) {
    /* Create buffer to hold data from socket */
    char read_buf[READ_SIZE];
    char unzip_buf[READ_SIZE];
    
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
        
        if ( poll(pollfds, 2, timeout_msecs) < 0) {
            fprintf(stderr, "poll() error: %s\n", strerror(errno));
            exit(1);
        };
        
        /* Retrieve network packet if avaliable. Process and foward to shell */
        if (sock_pollfd.revents & POLLIN) {
            read_len = read(sock_connect, read_buf, READ_SIZE);
            if (read_len < 0) {
                fprintf(stderr, "recv() error: %s\n", strerror(errno));
                exit(1);
            }
            printf("Message length: %d\n", read_len);
            fprintf(stderr, "%s\n", read_buf);
            
            
            /* We need to decompress the incoming message before
             forwarding to shell to */
            int unzip_size;
            if (compress_flag) {
                inbound.avail_in  = (uInt) read_len;
                inbound.next_in   = (Bytef *) read_buf;
                inbound.avail_out = READ_SIZE;
                inbound.next_out  = (Bytef *) unzip_buf; //pass by pointer
                do {
                    int inf_ret = inflate(&inbound, Z_SYNC_FLUSH);
                    if(inf_ret == Z_STREAM_ERROR){
                        fprintf(stderr, "inflate error(): %s", inbound.msg);
                        exit(1);
                    }
                } while (inbound.avail_in > 0);
                unzip_size = READ_SIZE - inbound.avail_out;
                memcpy(read_buf, unzip_buf, unzip_size);
                read_len  = unzip_size;
            }
            
            /* Forward user input to shell, one-character-at-a-time. */
            for (int i = 0; i < read_len; i++) {
                char recv_char = read_buf[i];
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
                        /* Close write end of pipe to shell */
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
                        fprintf(stdout, "Received char: %c\n", recv_char);
                        dprintf(PARENT_WRITE, "%c", recv_char);
                        printf("character sent to shell\n");
                        break;
                }
            }
        }
        
        /* Process input from shell pipe, if avaliable */
        if (pipe_pollfd.revents & POLLIN) {
            printf("Shell data avalable.\n");
            get_and_forward_shell_output(PARENT_READ);
        }
        
        if (pipe_pollfd.revents & (POLLERR | POLLHUP)) {
            exit(1);
        }
        
    }
    /* Indicate unexpected control locus */
    fprintf(stderr, "Unexpected control locus\n");
    exit(1);
}



int
main(int argc, char** argv) {
    
    fprintf(stderr, "Initiating Server...\n");
    
    /* Process options */
    opt_handler(argc, argv);
    fprintf(stderr, "Options processed...\n");
    
    atexit(shutdown_server);
    
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
            /* Fork and execute input parm */
            pid_t cpid = exec_prgm(prgm);
            /* Process user input from socket and shell output
             from pipe. Contains while(true) loop */
            echo_and_forwrd(cpid);
            
        }
    }
    /* Indicate unexpected control locus */
    fprintf(stderr, "Unexpected control locus\n");
    exit(2);
}
