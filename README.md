# Multithreaded Web Server
The project aims to develop a high-performance web server capable of efficiently handling concurrent HTTP requests. Utilizing a multithreaded architecture, the server can process multiple requests simultaneously without blocking other connections, showcasing principles of concurrent programming, network communication, and fault tolerance.

## Design Overview
The multithreaded web server employs a pool of worker threads to handle HTTP requests concurrently. Upon receiving incoming connections on a specified port, the server assigns them to worker threads, which then process the requests. Management of a shared request buffer between listener and worker threads is facilitated through semaphores and mutex locks, ensuring synchronized access.

## Functionality
`client.c`:
- Contains the main function to simulate multiple HTTP client requests to the web server.
- Parses command-line arguments for specifying server IP, port, number of threads, and requested page.
- Creates multiple threads to simulate concurrent client requests to the server.
- Measures request-handling time and outputs the results.

`net.c`:
- Implements network operations such as creating TCP sockets, resolving hostnames to IP addresses, and building HTTP GET queries.
- Provides a `client` function to simulate HTTP client requests to the server.

`webserver.c`:
- Implements the core functionality of the multithreaded web server.
- Defines the `worker` function as the worker thread routine. Worker threads continuously process requests from the request buffer and handle server crashes based on a specified crash rate.
- Defines the `listener` function to listen for incoming connections on the specified port and assign them to worker threads.
- Initializes semaphores and mutex locks for managing access to the shared request buffer.
- Parses command-line arguments to specify server port, number of worker threads, and crash rate.
- Creates the listener thread and multiple worker threads to handle incoming requests concurrently.
- Manages worker thread crashes by replacing crashed threads with new ones.

## File Structure and Content
```
multithreaded-web-server/
├── client.c
├── Makefile
├── net.c
├── README.md
├── Report.pdf
├── resources/
│   └── index.html
├── webserver.c
└── webserver.h
```