# Execution
There is a MakeFile, so the executable is compiled with the "make" command and executed with a command of the form 
#### ./server clients files_amounts requests lamda filenames 
N clients make L requests each from M files. One request is a part of a file, from a specific line to a specific line. 
#### e.g. ./server 20 2 7 0.05 example.txt notes.txt

# Main shared memory is used by clients to make requests to the server. Main shared memory contains the following variables: 
-	int file_num: the number of the requested file. 
-	int start_line: the number of the requested first line. 
-	int end_line: number of the requested last line. 
-	int temp_mem_used: this variable indicates whether the contents of the main memory correspond to a request to be serviced (1) or to a state of waiting for a request. This variable helps ensure the proper operation of the server's service loop, eliminating the possibility of a request being serviced more than once. 
-	int temp_shared_mem_key: the number of the key used by the client to construct temporary memory. The requested text will be written in the temporary memory.  Temporary shared memory's size is the size of the maximum line.
-	int finished: the number of client children that have been completed. The server service loop is executed until all clients are completed. When a client has fully serviced all requests, it increments this variable. 
-	int sem_id: I will explain the purpose of this variable later.


# Mutexes:
-	mutex_writer: protects all main memory variables except of finished, so that only one child process can access at a time.
-	mutex_finished: protects variable finished of main memory, , so that only one child process can access at a time.
-	mutex_diff: used to allow clients to notify the server that they made a new request (in main memory)
-	semaph_r, semaph_w (tables): used to synchronize caches. More precisely, each client creates a cache, makes its request in the main memory. Then the server serves it by writing the appropriate lines to the cache, and the client copies these lines from cache to its result file. Then, the client deletes this memory. This procedure is repeated for all requests. Therefore, at most one cache exists for a client at any one time.  So, I chose to have all the main memories created by the same client be served by the same semaphore.

    
# Introduction
Initially, erver creates all semaphores that was previously mentioned (initialized to 1 mutex_writer, mutex_finished, semaph_r and initialized to 0 mutex_diff and semaph_w), creates the main shared memory and forks to create the clients. Then, server performs a client service loop, which is described in detail below. A separate thread is used to serve each request. Finally, it performs the necessary cleanup.
 

# Main memory synchronisation - request submission - service loop
1.	The client selects (randomly) the file and file lines to request.
2.	The client constructs a one-line-long cache which will be used to pass the requested text between client and server.
3.	The client downs the mutex_writer semantic to request access to the main memory.
4.	The client modifies the file_num, start_line, end_line, temp_mem_used, temp_shared_mem_key, sem_id fields.
5.	The client ups mutex_diff semaphore to signal to the server that there is a new request to serve.
6.	Server dows mutex_diff semicarrier.
7.	Server retrieves from the main shared memory the unneeded data to service the request.
8.	Server attaches this process to the cache created by the client.
9.	The server identifies which semaphore to use from the semaph_r, semaph_w tables to manage the cache main memory via sem_id.
10.	With the information retrieved in 7,8,9 the server constructs a temporary anchor text to be sent to the thread that will satisfy the request.
11.	The server constructs the thread that will satisfy the request.
12.	The server changes temp_mem_used to 0, to indicate that it can accept a new request
13.	The server ups the mutex_writer semaphore, to allow the next client to send a request.
This loop is repeated for all requests.


# Request satisfaction - server thread
For i = 0; i < requested lines; i++
1.	The server downs the semaph_w semaphore requesting access to write to the cache.
2.	The server writes the i-th line to the cache.
3.	The server raises the semaph_r semaphore.
4.	The client downs the semaph_r semaphore and writes the line in main memory to its writefile ()
5.	The client ups the semaph_w semaphore, so that the server can send the next line to it.
End_for 
Then both participants detach from the cache and the client deletes it.

# Structure
The program consists of the source files parent_threads.cpp clients.cpp aid_functions.cpp and the header files shared_memory.h aid_functions.h. 
-	parent_threads. cpp: It contains a main function which is threaded to receive the arguments, a parent function which implements the multi-threaded server as described in "Introduction", "Main memory synchronization - request submission - service loop" and a threadFunction function which implements each individual thread of the server as described in "Request fulfillment server thread".
-	clients.cpp: Contains the code of each client procedure.
-	aid_functions.cpp: Contains auxiliary functions.
-	shared_memory.h: Contains shared memory's definition.


# Comments
-	Each client has its own writefile.
-	The length of each file is reflected in the LINES constant and the maximum line length by the MAX_LINE_SIZE constant.

