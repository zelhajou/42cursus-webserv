
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
