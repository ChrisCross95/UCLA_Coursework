/* headers */
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h> /* Core socket functions and data structures */
#include <sys/wait.h>
#include <netinet/in.h> /* For standard IP addresses and TCP and UDP port numbers.*/
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <ctime>
#define MYPORT 6969 /* Avoid reserved ports */
#define BACKLOG 10 /* pending connections queue size */
#define Server_Is_Running 1
#define CURRENT_DIRECTORY "/Users/majestikmind/CS_Material/CS118/winter20-project1"
#define MAX_BUFFER_SIZE 100000


using namespace std;

void string_To_Lowercase(string *input_string);
void getFileName(string message, string *filename);
void send_HTTP_Response(string message, int sock_connect);
void string_to_char(char * charptr, string input_string);
string get_Extension(string resource);
string lookupFile(string filename);

int main(int argc, char *argv[])
{
    
    int sock_listen, sock_connect; /* listen on sock_listen, new connection on new_connect */
    struct sockaddr_in my_addr; /* my address */
    struct sockaddr_in their_addr; /* connector addr */
    unsigned int sin_size = sizeof (their_addr);
    
    /* Assign values to the members of my_addr, which is a phony data structure used
     to associate an IP address and port number with a socket */
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(MYPORT);  /* Host-TO-Network-Short coverserion */
    // TODO: Need to use a system call to get IP
    my_addr.sin_addr.s_addr = inet_addr("192.168.0.9");
    
    /* socket() creates a new TCP socket with nternet Protocol v4 addresses */
    if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    /* bind() is typically used on the server side, and associates a socket
     with a socket address structure, i.e. a specified local IP address
     and a port number. */
    if (bind (sock_listen,(struct sockaddr *) &my_addr, sizeof (my_addr)) == -1)
    {
        perror ("bind");
        exit (1);
    }
    
    /* The listen function enables the server socket (i.e., sock_listen),
     to accept connections*/
    if (listen(sock_listen, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    /* IMPORTANT: A socket that has been established as a server can accept connection
     requests from multiple clients. The server's original socket does not become part
     of the connection; instead, accept makes a new socket which participates in the
     connection. accept returns the descriptor for this socket. The server's original
     socket remains available for listening for further connection requests. The
     while-loop below is the main accept loop of the server. */
    while (Server_Is_Running) {
        /* NOTE: client_addr and addrlen are result arguments The program passes
         empty client_addr and addrlen into the function, and the kernel will fill
         in these arguments with client’s information. */
        if ((sock_connect = accept(sock_listen, (struct sockaddr*)
                                   &their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        
        /* We've established a connect with a clinet at this point. We need to recv()
         their message and process it. */
        if (sock_connect) {
            
            void *message_buf [1024]; // buffer for incoming message.
            int connection; // value to hold connection status
            
            /* recv() returns the number of bytes actually read into the buffer, or -1 on error */
            connection = recv(sock_connect, message_buf, sizeof(message_buf), 0);
            
            if (connection == -1) {
                perror("recv");
            }
            /* recv() can return 0. This can mean only one thing: the remote side has
             closed the connection on you! A return value of 0 is recv()’s way of
             letting you know this has occurred. */
            else if (connection == 0) {
                perror("remote side has closed the connection");
                exit(1);
            }
            
            /* Process the request. */
            else {
                
                // Print the HTTP request to the console.
                string message = (const char *) message_buf;
                cout << "\n" << message << endl;
                
                // Send a message back.
                send_HTTP_Response(message, sock_connect);
                
                // Close the conection
                close(sock_connect);
            }
        }
        
    }
    
}


/* Implement directory search */
// https://www.manongdao.com/q-159690.html
void send_HTTP_Response(string message, int sock_connect) {
    
    
    /* Use the HTTP request to get the filename of the requested
     file. May return the filename of the 404 Error page. */
    string temp_string;
    getFileName(message, &temp_string);
    
    /* Perform the directoy search temp_string now holds
     the value of the resource filename */
    temp_string = lookupFile(temp_string);
    
    string ext_temp;
    FILE *file = NULL;
    const char* file_name = temp_string.c_str();
    /* If the extension is "text/html", the file such be opened as
     a text file using the mode 'r'. */
    if ( (ext_temp = get_Extension(temp_string)) == "text/html" ) {
        file = fopen(file_name, "r");
        if (!file) {
            perror("file");
            exit(1);
        }
    /* Otherwise, the file is a binary file and should be opened
      in 'rb' mode for binary files */
    } else {
        file = fopen(file_name, "rb");
        if (!file) {
            perror("file");
            exit(1);
        }
    }
    
    // cast ext_temp to char array for use in sprintf
    const char* ext = ext_temp.c_str();
    
    // Get file length
    fseek(file, 0, SEEK_END);
    // Save file length
    int fileLen;
    fileLen = ftell(file);
    // Reset position indicator
    fseek(file, 0, SEEK_SET);
    
    
    
    /*
     Populate header fields with correct information
     struct stat sb;
     
     if (stat(file_name, &sb) == -1) {
           perror("stat");
           exit(EXIT_FAILURE);
       }
     
     %s", ctime(&sb.st_mtime)
     %s", sb.st_size
     
     */
    
    
    
    /* We can now allocate space for the header, which
     must be large enough to contain the header and the
     daata. The rather conservative assumption is that
    the HTTP header itself wil require no more than 265 bytes */
    char *reply_msg = new char [fileLen + 257];
    
    // Allocate memory
    char *data = new char [fileLen + 1];
    if (!data) {
        fprintf(stderr, "Memory error!");
        fclose(file);
        exit(1);
    }
    
    //Read file contents into buffer
    fread(data, fileLen, 1, file);
    if (ferror(file)) {
        perror("Error reading file\n");
    }
    
    if (fclose(file) == EOF) {
        perror("file");
    }
    
    // Get the time and date infomration
    char* Date = NULL;
    time_t _tm = time(NULL);
    struct tm * curtime = localtime ( &_tm );
    /* Note about asctime: The output string is followed by
     a new-line character ('\n') and terminated with a
     null-character.*/
    Date = asctime(curtime);
    
    int hlen;
    hlen = snprintf(reply_msg, 256,
            "HTTP/1.1 200 OK\n"
            "Date: yyy\n"
            "Server: Zyphr/1.0.0\n"
            "Last-Modified: xxx\n"
            "ETag: \"56d-9989200-1132c580\"\n"
            "Content-Type: %s\n"
            "Content-Length: %i\n"
            "Accept-Ranges: bytes\n"
            "Connection: close\n"
            "\n", ext, fileLen);
    
    memcpy(reply_msg + hlen, data, fileLen);
    
    /* Send response. Appearently send() does not need
     to be placed inside a loop */
    int send_status;
    send_status = send(sock_connect, reply_msg, (hlen + fileLen), 0);
    
    delete[] reply_msg;
    delete[] data;
    
    return;
}


/* see https://pubs.opengroup.org/onlinepubs/009695399/functions/readdir.html
 for a really great example of using the dirent.h library to perform a
 directory search */
string lookupFile(string filename) {
    
    DIR *dirp = NULL;
    struct dirent *dir_ptr = NULL;
    int ext_flag = 1; // Assume requested file has extension
    
    
    if ((dirp = opendir((const char *) CURRENT_DIRECTORY)) == NULL) {
        cerr << "couldn't open: " << CURRENT_DIRECTORY;
        exit(1);
    }
    // Case in-sensitive comparison
    string_To_Lowercase(&filename);
    
    // If no dot, assume file has no extension
    if (filename.find(".") == std::string::npos) {
        ext_flag ^= ext_flag;
    }
    
    // Search the directory useing dirent.h library functions
    do {
        errno = 0;
        if ((dir_ptr = readdir(dirp)) != NULL) {
            
            /* d_name is stored as char array, so
             convert it to string */
            string resource(dir_ptr->d_name);
            
            // Case in-sensitive comparison
            string_To_Lowercase(&resource);
            
            // Requested file has extension
            if (ext_flag) {
                if (resource == filename) {
                    return resource;
                }
            }
            
            // Support file request without extension
            if (!ext_flag) {
                if (resource[0] == '.') {
                    // We don't want any hits that start with a dot
                    continue;
                }
                
                /* Have to check that resource has dot. But the the dot
                 cannot be at the start of the recourse filename If the
                 resource has a dot in its name, remove everything to the
                 right of the dot and then remove the dot itself. */
                string resource_no_ext(resource);
                if (resource_no_ext.find(".") != std::string::npos) {
                    while (resource_no_ext.back() != '.') {
                        resource_no_ext.pop_back();
                    }
                    // One more time to get the dot
                    resource_no_ext.pop_back();
                    
                    /* Compare requested file with extension-striped
                     resource name */
                    if (resource_no_ext == filename) {
                        return resource;
                    }
                    /* If the resource does not have a dot at the beginning
                     and does not have an extension, just check against it
                     against the query. */
                } else {
                    if (resource == filename) {
                        return resource;
                    }
                }
                
                
            }
            
        }
        
    } while (dir_ptr != NULL);
    
    if (closedir(dirp) == -1) {
        perror("closedir");
    }

    // If you get to this point, the requested file was not in the directory
    if (errno != 0) {
        perror("error reading directory");
    }
    
    // If you get to this point, the requested file was not in the directory
    return "404 Error Page";

}


// TODO: Implement this using a map
/* This is basically a linear search over the seven
 supported extensions. */
string get_Extension(string resource) {
    
    /* If the query missed, we need to return an
     html error page */
    if (resource == "404 Error Page") {
        return "text/html";
    }
    
    if (resource.find(".html") != std::string::npos) {
        return "text/html";
    }
    else if (resource.find(".htm") != std::string::npos) {
        return "text/html";
    }
    else if (resource.find(".txt") != std::string::npos) {
        return "text/html";
    }
    else if (resource.find(".jpg") != std::string::npos) {
        return "image/jpg";
    }
    else if (resource.find(".jpeg") != std::string::npos) {
        return "image/jpeg";
    }
    else if (resource.find(".png") != std::string::npos) {
        return "image/png";
    }
    else if (resource.find(".gif") != std::string::npos) {
        return "image/gif";
    }
    /* if there is no extension then consider it as a binary file.
     and prompt immediate client-side download */
    else { return "application/x-binary"; }
    
}


void getFileName(string message, string *filename) {
    char* ptr = &message[0];
    // Example: GET /GIVEMEDOCUMENT.jpeg HTTP/1.1
    while ((*ptr) != '\n') {
        filename->push_back(*ptr);
        ptr++;
    }
    // GET /GIVEMEDOCUMENT.jpeg HTTP/1.1 --> GIVEMEDOCUMENT.jpeg HTTP/1.1
    filename->erase(0, filename->find('/') + 1);
    // GIVEMEDOCUMENT.jpeg HTTP/1.1 --> GIVEMEDOCUMENT.jpeg
    filename->erase(filename->find('/') - 5, filename->length() - 1);
    
    // HTTP converts spaces characters to %20, so we need to remove them
    int pos = 0;
    while ( (pos = filename->find("%20")) != std::string::npos) {
        filename->replace(pos, 3, " ");
    }
    
    return;
}


void string_To_Lowercase(string *input_string) {
    
    char* ptr = (char *) input_string;
    
    while (*ptr) {
        if (isalpha(*ptr) != 0) {
            *ptr = tolower(*ptr);
        }
        ptr++;
    }
    return;
}


