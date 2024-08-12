# 42cursus-webserv

## Overview

This project involves developing a robust HTTP server in C++98, designed to handle basic web traffic and serve static content. Our server will conform to HTTP/1.1 standards and utilize non-blocking I/O operations for efficient resource management.

## Table of Contents

- [Project Description](#project-description)
- [Features](#features)
- [Technical Requirements](#technical-requirements)
- [Team Members and Roles](#team-members-and-roles)
- [Task Assignments](#task-assignments)
- [Team Composition and Initial Assignments](#team-composition-and-initial-assignments)
- 

- [Topics](#topics)

## Features

- **HTTP Protocol Support**: Implements essential features of the HTTP/1.1 protocol, including methods such as **GET**, **POST**, and **DELETE**.
- **Concurrency Handling**: Utilizes non-blocking sockets along with I/O multiplexing techniques (`select`, `poll`, `epoll`, and `kqueue`) to manage multiple client connections efficiently.
- **Static Content Serving**: Capable of serving HTML, CSS, and JavaScript files, enabling the hosting of static websites.
- **Error Handling**: Provides meaningful error responses and default error pages, ensuring the server robustly handles incorrect or unexpected client requests.
- **Customizable Configuration**: Includes a flexible configuration system inspired by NGINX, allowing adjustments for ports, server names, and routing behaviors without altering the server code.

## Technical Requirements

- **Programming Language**: C++98
- **Development Tools**: Git, Make, GCC, Valgrind
- **Testing Tools**: Stress testing with `siege` or `ab` (Apache Bench)

## Team Members and Roles

- [Hassan Sobane](https://github.com/hsobane) (hsobane): Network Infrastructure and Server Setup
- [Bilal Eddinaoui](https://github.com/edbilal) (beddinao): HTTP Protocol Handling
- [Zakaria Elhajoui](https://github.com/zelhajou) (zelhajou): Configuration Management and Logging

## Task Assignments

- **hsobane - Network Infrastructure and Server Setup**
	- Socket Creation and Management
	- Connection Handling
	- I/O Multiplexing
	- Error Handling and Resilience

- **beddinao - HTTP Protocol Handling**
	- Request Parsing
	- Response Generation
	- Support HTTP Methods
	- Static File Serving
	- File Upload Handling

- **zelhajou - Configuration Management and Logging**
	- Configuration File Parsing
	- Apply Configuration Settings
	- Logging System

## Team Composition and Initial Assignments

### 1. Network Infrastructure and Server Setup

The network infrastructure and server setup are crucial components of the project, as they form the foundation for handling incoming connections and managing client requests.

<details>
<summary><b>Requirements from Subject</b></summary>

- Must not crash under any circumstances.
- Must handle configuration files.
- Must be non-blocking and use one `poll()` (or equivalent) for all I/O operations.
- Ensure resilience and continuous availability under stress tests.
</details>

<details>
<summary><b>Tasks for Network Infrastructure and Server Setup</b></summary>

<details>
<summary><b>Task 1: Setup Basic Server Framework</b></summary>

**Objective**: Establish a basic server framework that can handle TCP connections.

**Steps:**

1. **Creating Main Entry Point:**
    - Developing `main.cpp`, which includes the main function where the server starts executing.
    - Setting up basic command line parsing to read arguments like configuration file paths or port numbers.

2. **Initializing Server Class:**
    - Designing a `Server` class that encapsulates all server-related functionalities.
    - Implementing constructors and destructors, considering resource management for network connections.
</details>

<details>
<summary><b>Task 2: Implementing Socket Programming</b></summary>

**Objective**: Develop the socket programming foundation that will allow the server to accept and manage client connections.

**Steps:**

1. **Socket Creation:**
    - Using the `socket()` system call to create a socket that listens for incoming connections.
    - Ensuring that the socket is set to the Internet domain (AF_INET) and uses TCP (SOCK_STREAM).

2. **Binding Socket:**
    - Binding the socket to an IP address and port. Typically, servers bind to `INADDR_ANY` to accept connections on any interface and a specific port from the server configuration.
    - Handling potential errors in binding, such as "Address already in use," with appropriate error messages and retries if necessary.

3. **Listening on Socket:**
    - Setting the socket to listen for incoming connections with `listen()`.
    - Defining the backlog queue length, which determines how many pending connections can queue up.
</details>

<details>
<summary><b>Task 3: Accepting and Managing Connections</b></summary>

**Objective**: Efficiently accept incoming client connections and manage these using either blocking or non-blocking sockets.

**Steps:**

1. **Accepting Connections:**
    - Continuously checking for incoming connections using `accept()`, which will block until a new connection is made unless non-blocking sockets are used.
    - For each accepted connection, possibly spawning a new thread or delegating to a worker depending on the concurrency model chosen (e.g., multi-threading, event-driven).

2. **Non-Blocking I/O:**
    - Setting sockets to non-blocking mode using `fcntl()` to prevent the server from being stalled by I/O operations.
    - This is essential for implementing an event-driven model that uses `select()`, `poll()`, or `epoll()` to handle multiple connections efficiently.
</details>

<details>
<summary><b>Task 4: Using Multiplexing for Handling Connections</b></summary>

**Objective**: Implement I/O multiplexing to handle multiple connections simultaneously without using multiple threads for each connection.

**Steps:**

1. **Select Implementation:**
    - Implementing `select()` as the starting point for multiplexing, which monitors multiple file descriptors to see if any of them are ready for reading or writing.
    - Setting up `fd_set` for read and write descriptors and using `select()` to determine which sockets can perform non-blocking read or write operations.

2. **Advanced Multiplexing (Optional):**
    - If performance under `select()` is limiting, considering using more scalable methods like `epoll()` on Linux or `kqueue()` on BSD systems, which handle large numbers of simultaneous connections more efficiently.
    - Implementing these systems in a modular way so that the underlying multiplexing mechanism can be switched based on the deployment environment or configuration settings.
</details>

<details>
<summary><b>Task 5: Testing and Validation</b></summary>

**Objective**: Ensure the network infrastructure is robust and can handle various network conditions.

**Steps:**

1. **Unit Testing:**
    - Writing unit tests for each component (socket creation, binding, listening, accepting).
    - Testing error handling and edge cases, such as what happens if the server runs out of available ports or the maximum number of connections is reached.

2. **Integration Testing:**
    - Testing the server with simulated client connections.
    - Using tools like `telnet` or `nc` (Netcat) to connect to the server and send requests.

3. **Stress Testing:**
    - Using a tool like `siege` or `ab` (Apache Bench) to stress test the server with a large number of concurrent connections.
</details>

</details>

---

### 2. HTTP Protocol Handling

The HTTP protocol handling component is responsible for parsing incoming HTTP requests, generating appropriate responses, and serving static content to clients.


<details>

<summary><b>Requirements from Subject</b></summary>

- Must support GET, POST, and DELETE methods.
- Must serve static websites.
- Must have default error pages.
- Must handle file uploads.

</details>

<details>

<summary><b>Tasks for HTTP Protocol Handling</b></summary>

<details>

<summary><b>Task 1: Design and Implement HTTP Request Parsing </b></summary>

**Objective**: Develop a system to correctly parse incoming HTTP requests and extract vital information such as the method, URI, version, headers, and body.

**Steps**:

1. **Understanding HTTP Request Structure**:
	- Familiarize with the structure of an HTTP request, which consists of a request line, headers, and an optional body.
	- The request line includes the method (GET, POST, etc.), URI, and HTTP version.

2. **Implement Request Parser**:
	- Create a class `HTTPRequest` that contains properties for method, URI, headers, version, and body.
	- Implement the parsing logic that reads the incoming request data and populates the `HTTPRequest` object.
	- Handle different types of HTTP headers and manage content-length for handling the body of POST requests.

3. **Error Handling in Parsing**:
	- Implement error checking during parsing to handle malformed requests.
	- Generate appropriate error responses for bad requests, such as `400 Bad Request`.
</details>

<details>

<summary><b>Task 2: Implement HTTP Response Generation</b></summary>

**Objective**: Construct HTTP responses based on the server's handling of the request, including generating headers and the appropriate body content.

**Steps**:

1. **Design Response Generator**:
	- Create a class `HTTPResponse` that includes properties for the status code, headers, and body.
	- Implement methods to easily add headers, set the status code, and append data to the body.

2. **Generating Responses**:
	- Based on the parsed request and server logic (e.g., fetching a static file or handling a form submission), fill the `HTTPResponse` object.
	- Ensure proper HTTP status codes are used (e.g., 200 OK, 404 Not Found).

3. **Content-Type Handling**:
	- Implement a mechanism to correctly identify and set the `Content-Type` header based on the file requested or data being served.

</details>

<details>

<summary><b>Task 3: Serve Static Content</b></summary>


**Objective**: Handle requests for static files like HTML, CSS, and images stored in the server's file system.

**Steps**:

1. **File System Access**:
	- Implement file reading capabilities that allow the server to fetch and serve files located in the server's document root or specified directories.
	- Handle file not found or access errors by responding with `404 Not Found` or` 403 Forbidden`.
2. **MIME Types**:
	- Set up a MIME type dictionary that maps file extensions to their corresponding MIME types, ensuring that the server responds with the correct `Content-Type`.
	- Use this dictionary when constructing the headers for static content responses.

</details>

<details>

<summary><b>Task 4: Implement Support for Different HTTP Methods</b></summary>

**Objective**: Support multiple HTTP methods, initially focusing on GET, POST, and DELETE.

**Steps**:

1. **GET Method**:
	- Implement handling for GET requests where the server returns the requested resource or a 404 if not found.

2. **POST Method**:
	- Handle POST requests, typically used for submitting forms. Process the data sent in the request body and possibly store it or respond based on the input.

3. **DELETE Method**:
	- Implement DELETE requests that allow clients to request the deletion of a resource. Ensure proper permissions and conditions are checked before deletion.
</details>


<details>

<summary><b>Task 5: Testing and Validation</b></summary>

**Objective**: Ensure the HTTP handling components function correctly and robustly across a range of expected and edge-case scenarios.

**Steps**:

1. **Unit Testing**:
	- Write comprehensive unit tests for each part of the HTTP handling, including request parsing, response generation, and method handling.
2. **Integration Testing**:
	- Test the integration of HTTP handling with the networking code to ensure that requests are correctly received, processed, and responded to.
3. **Functional Testing**:
	- Use tools like Postman or curl to test the server’s response to various HTTP requests, checking for correctness in headers, status codes, and body content.
</details>


</details>

---

### 3. Configuration Management and Logging

The configuration management and logging component are responsible for reading and applying server settings from external files and providing detailed logs for monitoring and debugging purposes.

<details>

<summary><b>Requirements from Subject</b></summary>

- The server must take a configuration file as an argument or use a default path.
- Allow configuration of ports, server names, default error pages, and other settings as specified in the configuration file.
- Implement a robust logging mechanism to aid in monitoring and debugging. (**Optional**)
 
 </details>

<details>


<summary><b>Tasks for Network Infrastructure and Server Setup</b></summary>

<details>

<summary><b>Task 1: Design and Implement Configuration Management</b></summary>

**Objective**: Develop a system to read, parse, and apply configuration settings from external files to control server behavior, such as listening ports, server root, and other operational parameters.

**Steps**:

1. **Understanding Configuration Requirements**:
	- Determine which server aspects should be configurable, including IP binding, port numbers, document root, default error pages, and security settings like maximum allowed connections and timeouts.
2. **Configuration File Format**:
	- Decide on a configuration file format like Nginx’s `nginx.conf`
3. **Implement Configuration Parser**:
	- Create a `ConfigParser` class that reads and parses the configuration file.
	- Use standard file I/O to read the configuration file and parse it into a structured format that the server can use.
	- Validate configuration values and provide defaults where necessary.

4. **Apply Configuration**:
	- Integrate the configuration settings into the server’s startup sequence.
	- Ensure that the server behaves according to the settings specified in the configuration file, such as starting on the correct port and serving files from the specified document root.

</details>


<details>

<summary><b>Task 2: Logging as an Optional Enhancement</b> </summary>

**Objective**: Set up a logging system to record important server events, errors, and operational data to aid in troubleshooting and monitoring server performance.

**Steps**:
1. **Design the Logging System**:
	- Determine the types of events that need logging, such as errors, warnings, information messages, and debug-level data.
	- Decide on the format and detail level of log messages. Include timestamps, log levels, and descriptive messages.
2. **Implement Logger Class**:
	- Create a `Logger` class that supports various log levels (e.g., DEBUG, INFO, WARN, ERROR).
	- Implement methods for each log level, e.g., `logInfo()`, `logError()`.
	- Use mutexes or other synchronization methods to make the logger thread-safe if the server handles multiple concurrent connections.
3. **Log Output Options**:
	- Allow logging output to be directed to different destinations, such as the console, a file, or even a network logging service.
	- Implement file-based logging with log rotation to prevent log files from growing indefinitely.
4. **Integration with Server Operations**:
	- Integrate logging calls throughout the server code, particularly at critical operations like starting up, shutting down, handling requests, and catching exceptions.
	- Log all exceptions and significant state changes within the server to provide a clear trace of what happens during operation.

</details>

<details>

<summary><b>Task 3: Testing and Validation</b></summary>

- **Objective**: Ensure the configuration and logging functionalities are robust, perform as expected, and do not introduce overhead or complexities that could impact the server's performance.

**Steps**:

1. **Unit Testing**:
	- Write tests for the configuration parsing to handle various edge cases and malformed configurations.
	- Test the logging system for correct log level handling and file rotation.
2. **Integration Testing**:
	- Verify that configuration settings are correctly applied during server startup.
	- Ensure that log messages accurately reflect server operations and states under different scenarios.
3. **Performance Evaluation**:
	- Assess the impact of logging on server performance, especially under high load, and optimize as necessary.

</details>

</details>

## Topics

**Web server**: A computer program that serves requested HTML pages or files.

- [What is a Server? (Deepdive)](https://youtu.be/VXmvM2QtuMU)
- [Web Server Concepts and Examples](https://www.youtube.com/watch?v=9J1nJOivdyw&t=313s)
- [Coding a Web Server in 25 Lines - Computerphile](https://www.youtube.com/watch?v=7GBlCinu9yg)

**Endianess**: The order in which bytes are stored in memory.

![little-big-endian](https://github.com/user-attachments/assets/3257c5e5-403d-47db-9bc2-f0ccdb79846e)

- **Big-endian**: Most significant byte is stored at the lowest memory address.
- **Little-endian**: Least significant byte is stored at the lowest memory address.

	- [Computerphile - Endianness Explained With an Egg](https://youtu.be/NcaiHcBvDR4)
	- [Computerphile - Lecture 22. Big Endian and Little Endian](https://youtu.be/T1C9Kj_78ek)
	- [What is Endianness? Big-Endian vs Little-Endian Explained with Examples](https://www.freecodecamp.org/news/what-is-endianness-big-endian-vs-little-endian/)
	- [geeksforgeeks - What is Endianness? Big-Endian & Little-Endian](https://www.geeksforgeeks.org/little-and-big-endian-mystery/)

**Networking Basics**:

1. **TCP/IP Protocol Suite**: A set of protocols that allows two or more devices to communicate over a network.

	- **IP (Internet Protocol)**:

   		![ip](https://github.com/user-attachments/assets/f9a63716-3a70-494a-9926-cbdeab056f76)
   
		- An IP address is a unique identifier for a device on a network.
		- IPv4 addresses are 32-bit numbers usually represented in dotted-decimal format (e.g., 192.168.1.1).
		- IPv6 addresses are 128-bit numbers represented in hexadecimal (e.g., 2001:0db8:85a3:0000:0000:8a2e:0370:7334).
	- **TCP (Transmission Control Protocol)**:

		![tcp-1](https://github.com/user-attachments/assets/f37c6f88-a2e9-406c-a358-d211d0022e8d)

		- A connection-oriented protocol that provides reliable, ordered, and error-checked delivery of a stream of bytes.
		- **Three-way handshake**:
			1. Client sends a SYN packet to the server.
			2. Server sends a SYN-ACK packet to the client.
			3. Client sends an ACK packet to the server.
		- **Four-way handshake**:
			1. Client sends a FIN packet to the server.
			2. Server sends an ACK packet to the client.
			3. Server sends a FIN packet to the client.
			4. Client sends an ACK packet to the server.
	- **UDP (User Datagram Protocol)**:
		- A connectionless protocol that provides unreliable, unordered, and error-checked delivery of a stream of bytes.
		- **Connectionless**: No connection is established before data is sent.
		- **Unreliable**: No guarantee that data will be delivered.
		- **Unordered**: No guarantee that data will be received in the order it was sent.
	- **Ports**:
		- A port is a communication endpoint that identifies a specific process or service on a host.
		- Ports are identified by a 16-bit number ranging from 0 to 65535.
		- **Well-known ports**:
			- Ports ranging from 0 to 1023 are reserved for system services.
		- **Registered ports**:
			- Ports ranging from 1024 to 49151 are registered with the IANA.
		- **Dynamic ports**:
			- Ports ranging from 49152 to 65535 are used for private or temporary purposes.
	- **Socket**:

	  	![sockets](https://github.com/user-attachments/assets/5e79d8ba-469a-489b-b4cf-9eb9ba762fa4)

   		- A socket is an endpoint for communication between two machines.
		- A socket is identified by an IP address and a port number.
		- **Server socket**:
			- A server socket listens for incoming connections.
		- **Client socket**:
			- A client socket initiates a connection to a server socket.

- [What is a Protocol? (Deepdive)](https://www.youtube.com/watch?v=d-zn-wv4Di8&t=246s)

2. **Socket Programming**: The process of writing programs that communicate with other programs across a network using sockets.

	- **Server-Client Model**:
		- A server is a program that listens for incoming connections.
		- A client is a program that initiates a connection to a server.
	- **Socket**:
		- A socket is an endpoint for communication between two machines.
		- A socket is identified by an IP address and a port number.
		- **Server socket**:
			- A server socket listens for incoming connections.
		- **Client socket**:
			- A client socket initiates a connection to a server socket.
	- **TCP Server-Client**:
		- <details>
			<summary>
				Server
			</summary>
			
			1. Create a socket using the `socket()` system call.
				```c
				int sockfd = socket(AF_INET, SOCK_STREAM, 0);
				
				// AF_INET: Address family for IPv4.
				// SOCK_STREAM: Type (TCP for reliable, connection-oriented service).
				// 0: Protocol (0 lets the system choose the appropriate protocol).
				```
			2. Bind the socket to an address using the `bind()` system call.
				```c
				struct sockaddr_in addr;
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr = INADDR_ANY;
				addr.sin_port = htons(8080);

				bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

				// struct sockaddr_in: Structure for IPv4 addresses.
				// sin_family: Address family for IPv4.
				// AF_INET: Address family for IPv4.
				// sin_addr.s_addr: IP address of the host.
				// sin_port: Port number.
				// INADDR_ANY: Accept connections to any IP address.
				// htons(): Convert port number to network byte order.
				```
			3. Listen for incoming connections using the `listen()` system call.
				```c
				listen(sockfd, SOMAXCONN);

				// SOMAXCONN: Maximum number of pending connections in the listen queue.
				```
			4. Accept a connection using the `accept()` system call.
				```c
				int new_socket = accept(sockfd, (struct sockaddr *)&address, &addrlen);

				// accept(): Waits for an incoming connection and returns a new socket descriptor.
				// new_socket: Socket descriptor for the new connection.
				```
			5. Send and receive data using the `send()` and `recv()` system calls.
				```c
				send(new_socket, "Hello, World!", 13, 0);
				recv(new_socket, buffer, 1024, 0);
				// send(): Sends data to the connected socket.
				// recv(): Receives data from the connected socket.
				// buffer: Buffer to store received data.
				```
   		</details>
  
   		- <details>
			<summary>
				Client:
			</summary>
			1. Create a socket using the `socket()` system call.
				```c
				int sockfd = socket(AF_INET, SOCK_STREAM, 0);

				// AF_INET: Address family for IPv4.
				// SOCK_STREAM: Type (TCP for reliable, connection-oriented service).
				// 0: Protocol (0 lets the system choose the appropriate protocol).
				```
			2. Connect to a server using the `connect()` system call.
				```c
				struct sockaddr_in addr;
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr = inet_addr("a.b.c.d");
				addr.sin_port = htons(8080);

				connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

				// inet_addr(): Converts an IP address in dotted-decimal notation to a long.
				// htons(): Convert port number to network byte order.
				```
			3. Send and receive data using the `send()` and `recv()` system calls.
				```c
				send(sockfd, "Hello, World!", 13, 0);
				recv(sockfd, buffer, 1024, 0);

				// send(): Sends data to the connected socket.
				// recv(): Receives data from the connected socket.
				// buffer: Buffer to store received data.
				```
     		</details>


	- **UDP Server-Client**:

		- <details>
			<summary>
				Server:
			</summary>
			
			1. Create a socket using the `socket()` system call.
				```c
				int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

				// AF_INET: Address family for IPv4.
				// SOCK_DGRAM: Type (UDP for unreliable, connectionless service).
				// 0: Protocol (0 lets the system choose the appropriate protocol).
				```
			2. Bind the socket to an address using the `bind()` system call.
				```c
				struct sockaddr_in addr;
				addr.sin_family = AF_INET;
				addr.sin_addr.s_addr = INADDR_ANY;
				addr.sin_port = htons(8080);

				bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

				// struct sockaddr_in: Structure for IPv4 addresses.
				// sin_family: Address family for IPv4.
				// AF_INET: Address family for IPv4.
				// sin_addr.s_addr: IP address of the host.
				// sin_port: Port number.
				// INADDR_ANY: Accept connections to any IP address.
				// htons(): Convert port number to network byte order.
				// bind(): Assigns the address specified by addr to the socket sockfd.
				```
			3. Send and receive data using the `sendto()` and `recvfrom()` system calls.
				```c
				sendto(sockfd, "Hello, World!", 13, 0, (struct sockaddr *)&addr, sizeof(addr));
				recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&addr, &addrlen);

				// sendto(): Sends data to a specific address.
				// recvfrom(): Receives data from a specific address.
				```
		</details>
  
  		- <details>
			<summary>
				Client:
			</summary>
			
			1. Create a socket using the `socket()` system call.
				```c
				int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

				// AF_INET: Address family for IPv4.
				// SOCK_DGRAM: Type (UDP for unreliable, connectionless service).
				// 0: Protocol (0 lets the system choose the appropriate protocol).
				```
			2. Send and receive data using the `sendto()` and `recvfrom()` system calls.
				```c
				sendto(sockfd, "Hello, World!", 13, 0, (struct sockaddr *)&addr, sizeof(addr));
				recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&addr, &addrlen);

				// sendto(): Sends data to a specific address.
				// recvfrom(): Receives data from a specific address.
				```
    		</details>
2. **TCP/IP Communication**: The process of establishing a connection between two devices using the TCP/IP protocol suite.

	- **Three-way handshake**:
		1. Client sends a SYN packet to the server.
		2. Server sends a SYN-ACK packet to the client.
		3. Client sends an ACK packet to the server.
	- **Four-way handshake**:
		1. Client sends a FIN packet to the server.
		2. Server sends an ACK packet to the client.
		3. Server sends a FIN packet to the client.
		4. Client sends an ACK packet to the server.
	- **Connection Establishment**:
		1. Client sends a SYN packet to the server.
		2. Server sends 1. **Unit Testing**:
	- Write tests for the configuration parsing to handle various edge cases and malformed configurations.
	- Test the logging system for correct log level handling and file rotation.
2. **Integration Testing**:
	- Verify that configuration settings are correctly applied during server startup.
	- Ensure that log messages accurately reflect server operations and states under different scenarios.
3. **Performance Evaluation**:
	- Assess the impact of logging on server performance, especially under high load, and optimize as necessary.
a SYN-ACK packet to the client.
		3. Client sends an ACK packet to the server.
	- **Connection Termination**:
		1. Client sends a FIN packet to the server.
		2. Server sends an ACK packet to the client.
		3. Server sends a FIN packet to the client.
		4. Client sends an ACK packet to the server.
	- **TCP Header**:

		![tcp-header-format](https://github.com/user-attachments/assets/82afdbd2-75d6-4c5c-93ee-f653f23fb50c)

	- **TCP Flags**:
		- **SYN (Synchronize)**: Initiates a connection.
		- **ACK (Acknowledgment)**: Acknowledges received data.
		- **FIN (Finish)**: Terminates a connection.
		- **RST (Reset)**: Resets a connection.
		- **PSH (Push)**: Pushes data to the application layer.
		- **URG (Urgent)**: Indicates urgent data.
	- **TCP State Diagram**:
		- **CLOSED**: No connection.
		- **LISTEN**: Waiting for a connection request.
		- **SYN-SENT**: Sent a connection request.
		- **SYN-RECEIVED**: Received a connection request.
		- **ESTABLISHED**: Connection established.
		- **FIN-WAIT-1**: Sent a connection termination request.
		- **FIN-WAIT-2**: Received an acknowledgment for the termination request.
		- **CLOSE-WAIT**: Received a connection termination request.
		- **CLOSING**: Sent an acknowledgment for the termination request.
		- **LAST-ACK**: Received a connection termination request acknowledgment.
		- **TIME-WAIT**: Waiting for delayed packets.
	
4. **Non-blocking I/O and Multiplexing**: The process of handling multiple I/O operations simultaneously without blocking.

	- **Non-blocking I/O**:
		- **Blocking I/O**: The process waits until the I/O operation is complete.
		- **Non-blocking I/O**:
			- The process continues executing while the I/O operation is in progress.
				```c
				fcntl(sockfd, F_SETFL, O_NONBLOCK);

				// fcntl(): Performs operations on file descriptors.
				```
		- **Multiplexing**:
			- **select()**:
				- Monitors multiple file descriptors for I/O readiness.
				- **Read Set**: Contains file descriptors that are ready for reading.
				- **Write Set**: Contains file descriptors that are ready for writing.
				- **Error Set**: Contains file descriptors that have errors.
				- **Timeout**: Specifies the maximum time to wait for an event.
				- **select()** returns the number of ready file descriptors.
				- **FD_ISSET()**: Checks if a file descriptor is in a set.
				- **FD_SET()**: Adds a file descriptor to a set.
				- **FD_CLR()**: Removes a file descriptor from a set.
				- **FD_ZERO()**: Clears a set.
				```c
				fd_set readfds;
				FD_ZERO(&readfds);
				FD_SET(sockfd, &readfds);

				select(sockfd + 1, &readfds, NULL, NULL, NULL);

				if (FD_ISSET(sockfd, &readfds)) {
					// sockfd is ready for reading.
				}

				// FD_ZERO: Clears a set.
				// FD_SET: Adds a file descriptor to a set.
				// FD_CLR: Removes a file descriptor from a set.
				// FD_ISSET: Checks if a file descriptor is in a set.
				```
			- **poll()**:
				- Monitors multiple file descriptors for I/O readiness.
				- **Timeout**:
					- Specifies the maximum time to wait for an event.
				- **poll()** returns the number of ready file descriptors.
				```c
				struct pollfd fds;
				fds.fd = sockfd;
				fds.events = POLLIN;

				poll(&fds, 1, -1);

				if (fds.revents & POLLIN) {
					// sockfd is ready for reading.
				}

				// struct pollfd: Structure for poll events.
				// fds.fd: File descriptor to monitor.
				// fds.events: Events to monitor.
				// fds.revents: Events that occurred.
				```
			- **epoll()**:
				- Monitors multiple file descriptors for I/O readiness.
				- **Timeout**:
					- Specifies the maximum time to wait for an event.
				- **epoll()** returns the number of ready file descriptors.
				```c
				int epfd = epoll_create(1);
				struct epoll_event event;
				event.events = EPOLLIN;
				event.data.fd = sockfd;

				epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event
				epoll_wait(epfd, &event, 1, -1);

				// epoll_create(): Creates an epoll instance.
				// struct epoll_event: Structure for epoll events.
				// event.events: Events to monitor.
				// event.data.fd: File descriptor to monitor.
				// epoll_ctl(): Modifies an epoll instance.
				// epoll_wait(): Waits for an event on an epoll instance.
				```
			- **kqueue()**:
				- Monitors multiple file descriptors for I/O readiness.
				- **Timeout**:
					- Specifies the maximum time to wait for an event.
				- **kqueue()** returns the number of ready file descriptors.
				```c
				int kq = kqueue();
				struct kevent event;
				EV_SET(&event, sockfd, EVFILT_READ, EV_ADD, 0, 0, NULL);

				kevent(kq, &event, 1, &event, 1, NULL);

				// kqueue(): Creates a kqueue instance.
				// struct kevent: Structure for kqueue events.
				// EV_SET: Initializes a kevent structure.
				// kevent(): Modifies a kqueue instance.
				```

5. **HTTP Protocol**: A protocol that allows web browsers and servers to communicate over the internet.

	![whats-http](https://github.com/user-attachments/assets/d6f06e60-fa31-45db-8d28-bb311939e5fc)

 	![http2](https://github.com/user-attachments/assets/d80b4962-f449-45ec-abb7-a64298b86d6e)


- [Everything you need to know about HTTP](https://cs.fyi/guide/http-in-depth)

	- **Request Methods**:
 
  		![request-methods-1](https://github.com/user-attachments/assets/989fabcb-05e4-4859-ad08-f6a1f99b0795)
		
	- **Request Headers**:

   		![request-headers](https://github.com/user-attachments/assets/4f3f6eb4-65bb-4fe5-8335-2dcdb74e6856)

		![anatomy-http-request](https://github.com/user-attachments/assets/a43fe18d-c745-4bbf-a13c-ee3a75ee1d2c)
 
		**Example**:

		```bash
		GET /index.html HTTP/1.1
		Host: www.example.com
		User-Agent: Mozilla/5.0
		Accept: text/html
		Accept-Language: en-US
		Accept-Encoding: gzip, deflate
		Connection: keep-alive
		```

	- **Response Headers**:

   		![anatomy-http-response (1)](https://github.com/user-attachments/assets/5cbafac7-d89b-44f0-84cc-8d958cb19f97)
 		![response-headers](https://github.com/user-attachments/assets/38aa53ff-3133-428e-8e0d-ecfbfc050aa0)
 
   		**Example**:

		```bash
		HTTP/1.1 200 OK
		Date: Mon, 01 Jan 2022 00:00:00 GMT
		Server: Apache/2.4.51
		Content-Type: text/html
		Content-Length: 13
		Connection: keep-alive
		Set-Cookie: session=123
		Last-Modified: Mon, 01 Jan 2022 00:00:00 GMT
		Location: /index.html
		Content-Language: en-US
		X-Frame-Options: DENY
		X-XSS-Protection: 1; mode=block
		```

	- **Status Codes**:
   
		![status-codes](https://github.com/user-attachments/assets/e3c779ac-e364-4bd0-90e0-7a9f334c7313)

	- **HTTP/1.1**:
		- **Persistent Connections**: Allows multiple requests and responses to be sent over a single connection.
		- **Pipelining**: Allows multiple requests to be sent without waiting for the responses.
		- **Chunked Transfer Encoding**: Allows data to be sent in chunks.
		- **Content Negotiation**: Allows the server to send different content based on the client's preferences.
		- **Caching**: Allows the client to store a copy of the response for future use.
		- **Compression**: Allows the server to compress the response before sending it to the client.
		- **Authentication**: Allows the server to require user authentication.
		- **Cookies**: Allows the server to store information on the client's computer.
		- **Redirects**: Allows the server to redirect the client to a different URL.
		- **Error Handling**: Allows the server to send error messages to the client.
		- **Security**: Allows the server to enforce security policies.

	- [HTTP Headers - MDN Web Docs](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers)


---

- [C++ Network Programming Part 1: Sockets](https://youtu.be/gntyAFoZp-E)
- [Sockets in Operating System](https://youtu.be/uagKTbohimU)
- [Socket Programming in C/C++ - GeeksforGeeks](https://www.geeksforgeeks.org/socket-programming-cc/)
- [Socket Programming in C - Tutorialspoint](https://www.tutorialspoint.com/unix_sockets/socket_server_example.htm)
- [Sockets and Pipes Look Like Files (Unix/fdopen)](https://youtu.be/il4N6KjVQ-s)
