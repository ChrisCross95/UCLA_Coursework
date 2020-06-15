#define _GNU_SOURCE

#include <string.h>
#include <signal.h>
#include <poll.h>       /* For monitoring I/0 readiness*/
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
#include <pthread.h>    /* On Linux, compile using cc -pthread. */
#include <stdbool.h>
#include <netdb.h>
#include <mcrypt.h>
#include "netdefs.h"
#include "zlib.h"
#include <assert.h>

int timeout_msecs     = 0;
int log_flag          = 0;
int port_flag         = 0;
int compress_flag     = 0;
static int output_fd  = 1;
static int input_fd   = 0;

#define PAYLOAD_SIZE 1
/* socket file_descriptor */
int sockfd;
int SERV_PORT;

z_stream inbound;
z_stream outbound;

#define SCALING_FACTOR      2
#define READ_SIZE       16384  /* 8kB */
#define ctrl_C           0x03
#define ctrl_D           0x04  /* Escape */
#define cr_char          0x0D  /* Carraige Return */
#define lf_char          0x0A  /* Line Feed */
#define sp_char          0x20  /* Space */

char* use_msg = "Correct usage: lab1b-server --port [--log][--compress]\n";


/* These buffers stored compressed and/or encrypted data
 There Note that structs cannot be initalized in C. */
struct buffer {
    int capacity;
    int load;
    int avaliable;
    char *data;
};

struct buffer *sent_buf;
struct buffer *recv_buf;
int logfd;

/* Reallocate buffer space as necessary */
void
grow_buffer(struct buffer *buf) {
    buf->capacity *= SCALING_FACTOR;
    buf->data = (char *) realloc(buf->data, buf->capacity);
    if (buf->data == NULL) {
        fprintf(stderr, "Realloc Error: %s\n", strerror(errno));
        exit(1);
    } else {
        buf->avaliable = (buf->capacity) - (buf->load);
        printf("Buffer scaled!\n");
    }
    return;
}

void
error(char *msg)
{
    perror(msg);
    exit(0);
}

void
setpollfd(struct pollfd *pollfds) {
#define keybd_pollfd pollfds[0]
    keybd_pollfd.fd        = STDIN_FILENO;
    keybd_pollfd.events    = POLLIN | POLLHUP | POLLERR;
    keybd_pollfd.revents   = 0;
#define sock_pollfd  pollfds[1]
    sock_pollfd.fd         = sockfd;
    sock_pollfd.events     = POLLIN | POLLHUP | POLLERR;
    sock_pollfd.revents    = 0;
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
get_server_data() {
    
    /* Create buffer to hold data from server */
    char read_buf[READ_SIZE];
    char unzip_buf[READ_SIZE];
    
    /* Read input from server. Loop broken by ^D (0x04) */
    int read_len = 0;
    read_len = read(sockfd, read_buf, READ_SIZE);
    if ( (read_len < 0) || (read_len == 0) ) {
        fprintf(stderr, "Server has closed connection: %s\n", strerror(errno));
        exit(0);
    }
    fprintf(stderr, "Bytes received: %d%c%c", read_len, lf_char, cr_char);
    
    /* Log recieved data (prior to any decompression) */
    if (log_flag) {
        while (read_len > (recv_buf->avaliable)) {
            grow_buffer(recv_buf);
        }
        memcpy( (recv_buf->data) + (recv_buf->load), read_buf, read_len);
        recv_buf->load += read_len;
    }
    
    /* Exchange compressed data for uncompressed msg, display, and return */
    if (compress_flag) {
        int unzip_size;
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
        /* display data from server to user */
        for (int i = 0; i < unzip_size; i++) {
            switch (unzip_buf[i]) {
                    /* Upon receiving EOF from the shell, restore
                     terminal modes and exit with return code 1 */
                case ctrl_D:
                    fprintf(stderr, "Connection closed\n");
                    exit(0);
                    break;
                case lf_char:
                    fprintf(stderr, "%c%c", cr_char, lf_char);
                    break;
                default:
                    write(STDOUT_FILENO, &unzip_buf[i], 1);
                    break;
            }
        }
        return;
    }
    
    
    /* Display data from server to user */
    for (int i = 0; i < read_len; i++) {
        switch (read_buf[i]) {
                /* Upon receiving EOF from the shell, restore
                 terminal modes and exit with return code 1 */
            case ctrl_D:
                fprintf(stderr, "Connection closed\n");
                exit(0);
                break;
            case lf_char:
                fprintf(stderr, "%c%c", cr_char, lf_char);
                break;
            default:
                write(STDOUT_FILENO, &read_buf[i], 1);
                break;
        }
    }
    
    
    return;
}

/* returns a pointer to a dynamically allocated buffer structure. */
struct buffer*
init_buf() {
    struct buffer *bufptr;
    bufptr = (struct buffer *) malloc(sizeof(struct buffer));
    if (bufptr  == NULL) {
        fprintf(stderr, "Buffer allocation error: %s\n", strerror(errno));
        exit(1);
    }
    bufptr->data      = (char *) malloc(READ_SIZE);
    bufptr->capacity  = READ_SIZE;
    bufptr->load      = 0;
    bufptr->avaliable = (bufptr->capacity) - (bufptr->load);
    fprintf(stderr, "buffer space allocated\n");
    return bufptr;
}

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
    
    int returned_option;
    while (1)
    {
        static struct option long_options[] =
        {
            /* --shell argument to pass input/output
             between the terminal and a shell */
            {"log",      required_argument,    &log_flag,       1},
            {"port",     required_argument,    &port_flag,      1},
            {"compress", no_argument,          &compress_flag,  1},
            /* Terminate the array with an element containing all zeros. */
            {0, 0, 0, 0}
        };
        
        int option_index = 0; /* Updated by getopt_long() with the index of the option */
        returned_option = getopt_long(argc, argv, "l:p:c",
                                      long_options, &option_index);
        
        /* Detect the end of the options. */
        if (returned_option == -1) { break; }
        
        const char *option_name = (char *) malloc(strlen(long_options[option_index].name));
        if (option_name == NULL) {
            fprintf(stderr, "Option handling error: %s\n", strerror(errno));
            exit(1);
        } else {
            option_name = long_options[option_index].name;
        }
        
        if ( strcmp(option_name, "log") == 0 ) {
            fprintf(stderr, "Logging Enabled.\n");
            /* optarg is set by getopt_long with a ptr to the argument,
             in this case, the name of a file */
            logfd = open(optarg, O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
            if (logfd < 0) {
                fprintf(stderr, "%s\n", strerror(errno));
                exit(1);
            }
            /* Allocate space for buffers to hold logged data */
            sent_buf = init_buf();
            recv_buf = init_buf();
            continue;
        }
        /* Compression using zlib library */
        else if ( strcmp(option_name, "compress") == 0 ) {
            zip_init();
            fprintf(stderr, "Compression Enabled.\n");
            continue;
        }
        
        else if ( strcmp(option_name, "port") == 0 ) {
            /* a zero base is taken as 10 (decimal) */
            SERV_PORT = strtol(optarg, NULL, 0);
            fprintf(stderr, "Server Port specified: %d\n", SERV_PORT);
            continue;
        }
        
        else {
            fprintf(stderr, "%s", use_msg);
            exit(1);
        }
        
    }
    
    if (!port_flag) {
        fprintf(stderr, "--port=<port number> is a mandatory option\n");
        exit(1);
    }
    return;
}

void
shutdown_client() {
    
    restore_tty(&original_settings);
    
    printf("log_flag: %d\n", log_flag);
    if (log_flag) {
        
        fprintf(stderr, "writing to log file");
        
        for (int i = 0; i < recv_buf->load; i++) {
            if ( (recv_buf->data[i] == lf_char) || (recv_buf->data[i] == cr_char) ) {
                recv_buf->data[i] = sp_char;
            }
        }
        
        dprintf(logfd, "SENT %d bytes: %s\n", sent_buf->load, sent_buf->data);
        dprintf(logfd, "RECEIVED %d bytes: %s\n", recv_buf->load, recv_buf->data);
    } else {
        printf("log_flag state: %d", log_flag);
    }
    
    if (compress_flag) {
        deflateEnd(&outbound);
        inflateEnd(&inbound);
    }
    
    close(sockfd);
    fprintf(stderr, "exiting as expected");
}

void
connect_to_server() {
    /* Create socket file_descriptor */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    
    /* Fill in Address data-structure */
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERV_ADDR);
    
    /* Connects sockfd to address */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    } else {
        printf("Connection established on port: %d\n", SERV_PORT);
        printf("Server IP address: %s\n", inet_ntoa(serv_addr.sin_addr));
        printf("Server port: %d\n", (int) ntohs(serv_addr.sin_port));
    }
    
    return;
}


int
main(int argc, char** argv) {
    
    system("clear");
    
    opt_handler(argc, argv);
    
    atexit(shutdown_client);
    
    connect_to_server();
    
    /* Handles terminal settings */
    tty_config();
    
    /* Create buffer to hold data from standard input */
    char read_buf[READ_SIZE];
    /* Create buffer to hold compressed data */
    char zip_buf[READ_SIZE];
    
    static struct pollfd pollfds[2];
    setpollfd(pollfds);
    int read_len   = 0; /* Bytes returned by read */
    int poll_result; /* for multithreading */
    /* Read user input. Loop broken on client-side by ^D (0x04) */
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
                
                char keybd_char = read_buf[i];
                
                switch (keybd_char) {
                        /* <cr> or <lf> should echo as <cr><lf> and go to shell as <lf>. */
                    case cr_char:
                    case lf_char:
                        dprintf(STDOUT_FILENO, "%c%c", cr_char, lf_char);
                        /* compressed transmission */
                        if (compress_flag) {
                            int zip_size;
                            outbound.avail_in  = (uInt) read_len;
                            outbound.next_in   = (Bytef *) &keybd_char;
                            outbound.avail_out = READ_SIZE;
                            outbound.next_out  = (Bytef *) zip_buf;
                            do {
                                int def_ret = deflate(&outbound, Z_SYNC_FLUSH);
                                if(def_ret == Z_STREAM_ERROR){
                                    fprintf(stderr, "deflate() error: %s", outbound.msg);
                                    exit(1);
                                }
                            } while (outbound.avail_in > 0);
                            
                            zip_size = READ_SIZE - outbound.avail_out;
                            
                            if (log_flag) {
                                while ( (sent_buf->avaliable) < zip_size) {
                                    grow_buffer(sent_buf);
                                }
                                memcpy( (sent_buf->data) + (sent_buf->load), zip_buf, zip_size);
                                sent_buf->load += zip_size;
                            }
                            
                            send(sockfd, &zip_buf, zip_size, 0);
                            break;
                            
                        } else /* raw transmission */ {
                            
                            if (log_flag) {
                                if ( (sent_buf->avaliable) == 0) {
                                    grow_buffer(sent_buf);
                                }
                                (sent_buf->data[sent_buf->load]) = sp_char;
                                (sent_buf->load)++;
                            }
                            char *lf_ptr = (char *) malloc(sizeof(char));
                            *lf_ptr = lf_char;
                            send(sockfd, lf_ptr, PAYLOAD_SIZE, 0);
                            break;
                        }
                        /* Note: If a ^D or ^D is recieved, it is forwarded to server*/
                    default:
                        dprintf(STDOUT_FILENO, "%c", keybd_char);
                        
                        /* compressed transmission */
                        if (compress_flag) {
                            int zip_size;
                            outbound.avail_in  = (uInt) read_len;
                            outbound.next_in   = (Bytef *) &keybd_char;
                            outbound.avail_out = READ_SIZE;
                            outbound.next_out  = (Bytef *) zip_buf;
                            do {
                                int def_ret = deflate(&outbound, Z_SYNC_FLUSH);
                                if(def_ret == Z_STREAM_ERROR){
                                    fprintf(stderr, "deflate() error: %s", outbound.msg);
                                    exit(1);
                                }
                            } while (outbound.avail_in > 0);
                            
                            zip_size = READ_SIZE - outbound.avail_out;
                            
                            if (log_flag) {
                                while ( (sent_buf->avaliable) < zip_size) {
                                    grow_buffer(sent_buf);
                                }
                                memcpy( (sent_buf->data) + (sent_buf->load), zip_buf, zip_size);
                                sent_buf->load += zip_size;
                            }
                            
                            send(sockfd, &zip_buf, zip_size, 0);
                            break;
                            
                        } else  /* raw transmission */ {
                            
                            if (log_flag) {
                                if ( (sent_buf->avaliable) == 0) {
                                    grow_buffer(sent_buf);
                                }
                                (sent_buf->data[sent_buf->load]) = keybd_char;
                                (sent_buf->load)++;
                            }
                            
                            send(sockfd, &keybd_char, PAYLOAD_SIZE, 0);
                            break;
                        }
                }
            }
        }
        
        if (sock_pollfd.revents & (POLLERR | POLLHUP)) {
            exit(0);
        }
        
        /* Process input from socket, if avaliable */
        if (sock_pollfd.revents & POLLIN) {
            get_server_data();
        }
        
    }
    exit(0);
}

