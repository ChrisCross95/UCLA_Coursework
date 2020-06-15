//////////////////////////
CS118 Project 1 Report
Name: Christopher Cross
UCLA ID: 10523826
//////////////////////////


○ The problems you ran into and how you solved the problems
○ List of any additional libraries used
○ Acknowledgement of any online tutorials or code example (except class
website) you have been using.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
High Level Design: 

1.) The sever goes through the standard sock() --> bind() --> listen() sequence
2.) The server enters a loop listening for incoming requests and calling
	accept() to establish a connection 
3.) If a connection is established, the server calls recv() to read the HTTP
	request,  checks to makes the output of recv() is not 0 or -1, then
	immediate outputs the HTTP to the terminal 
4.) The server next calls send_HTTP_Response() to get the requested data
5.) send_HTTP_Response():
	(a) Parse the HTTP request to get the filename
	(b) Call lookupFile(), which does a case-insensitive search of the 
		current directory using dirent.h library calls. lookupFile() 
		supports extensionless file requests and returns the file 
		descriptor of the 404 Error Page in the case of a miss. 
	(c) Open the file returned by lookupFile() in either 'r' or 'rb'
		mode. This depends on the return value of get_Extension().
		(i) get_Extension() returns either text/html or image/jpeg
			depending on the extension of the file. This is 
			name of the file >> in the current directory <<.
			In the case of no extension, the default return 
			is text/html. Current implementation is O(n) but
			could be reduced to O(1) using a map.
	(d) Get file size, allocate buffer of sufficient width, perform
		fread() operation, get time and date imformation
	(e) Call snprintf to format header and memcpy to add binary data
	(f) Call send() to pass segment to network layer
8.) Call close() to close connection.
9.) Return to step 2   


- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Issue 0: 

server.cpp:15:10: fatal error: 'experimental/filesystem' file not found
#include <experimental/filesystem>
         ^~~~~~~~~~~~~~~~~~~~~~~~~

Solution: A direct solution would require updating the g++ compiler 
and linking appropriate binary files with the implementation of the
filesystem library. A quicker workaround was found: use the following
libraries: 

	#include <unistd.h> // To get current directory and read and write
	#include <dirent.h> // To search current directory 
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
Issue 1: Server can not send images or gifs

The server handles requests for text/html, but cannot transmit binary data. 

The issue is that I am using allocating a buffer the size of the fize + 256
in get_HTTP_Reponse. In the case of large binary files, this causes a
segmentation fault on the call stack. 

We need a way to transmit smaller packets. Luckily this occurs after much of
the leg work has been done. Ofcourse, there is always plenty of space on the
heap. So instead of saving the data in local variables, which are on the stack,
we'll put the data somewhere on the heap with a dynamic memory allocation call. 
This has the added benefit of not requiring changes to the code structure.

After additional research, the issue is likely related to the Content-Type
field in the HTTP header. By changing the value to "application/x-binary",
I was able to send a binary file. However, the binary file sent has size 0
when I ran the command ls -l, indicating that it was not sent successfully.
I changed fopen(file_name, 'r') to fopen(file_name, 'rb') to read binary
data.  
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Issue 2: Server cannot send binary files

The server sends binary files of size 0. The issue was that the directory
search did not consider resources without extension, i.e. bigfile. An 
additional if-statement resolves this problem. The server now sends
binary files with correct size. 

However, the server does not send binary files with the correct data. 
The command diff bigfile trans_bigfile gives the following output:  

	Binary files bigfile and tbigfile differ

Thus, binary file transfers does not preserve the integrity of the binary 
data. This is also likely why the server cannot serve images of gifs. The
main hypothesis is that the conversion from a char array to a C-string 
causes data loss. The issue is that binary data may contain a '\0' byte, 
which signals the end of a string. So we need a way to append the body
to the header without converting the body to a string. 

By writing a for-loop to print out the binary data to the terminal, I see
that the conversion from a char array to a string is indeed the culprit. 

The solution was to do the file transfer within send_HTTP_Response() by
allocating memory on the heap, and using snprintf to format the header
and memcpy() to append the body without corrupting binary data. 

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Issue 3: Server.cpp won't compile on linux VM. 

The issue seems to be be that back() and pop_back() are string members
supported only in newer versions of the C++ compiler. I could have rewrote
the code, but instead choose to demo the server locally. 
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Additionally Libraries: 

#include <fstream>
#include <iostream>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdio.h>
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Additional Online Resources: 

https://pubs.opengroup.org/onlinepubs/009695399/functions/readdir.html
https://www.manongdao.com/q-159690.html
Stack Overflow and C++ Reference, liberally. 
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
