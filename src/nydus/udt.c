#include <nydus/nydus.h>
#include <cudt/cudt.h>


#define fmt(s) "[Nydus][UDT Handle] " s
static void * udt_handler(void * canal) {
    /**
     *  udt_handler() - handles udt connections
     *
     *  @conn:  pointer to udt socket
     *
     *  Handles UDT connections to the Nydus proxy by reading data from the UDT
     *  socket and writing it to a TCP socket, as well as the reverse.
    **/
    // TODO set up new tcp connection
    // TODO read from udt socket
    // TODO write to tcp socket
    // TODO gracefully shut down / handle disconnects
    syslog(LOG_INFO, fmt("Handling connection %lu"), canal);

    int sock;
    struct addrinfo hints;
    struct addrinfo * info;

    const char * host = ((struct canal *)canal)->target_host;
    const char * port = ((struct canal *)canal)->target_port;

    // Look up the destination information.
    memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family      = AF_INET; // Allow IPv4 (IPv6 soon)
        hints.ai_socktype    = SOCK_STREAM; // Request TCP socket
        hints.ai_flags       = AI_PASSIVE; // Whatever interface for now.
        hints.ai_protocol    = 0;
        hints.ai_canonname   = NULL;
        hints.ai_addr        = NULL;
        hints.ai_next        = NULL;

    if(getaddrinfo(host, port, &hints, &info)) {
        syslog(LOG_ERR, fmt("Failed to retrieve target info."));
        syslog(LOG_ERR, fmt("%s"), gai_strerror(errno));
        abort();
    }

    // Try each address in order until we find one that works.
    struct addrinfo * curr;
    for(curr = info; curr != NULL; curr = curr->ai_next) {
        
        // Create the server socket.
        sock = socket(
            curr->ai_family,
            curr->ai_socktype,
            curr->ai_protocol
        );
        if(sock < 0){
            syslog(LOG_WARNING, fmt("Failed to create server socket."));
            syslog(LOG_WARNING, fmt("%s"), strerror(sock));
            continue;
        }
        
        // Connect to the other side.
        if(connect(sock, curr->ai_addr, curr->ai_addrlen)) {
            syslog(LOG_WARNING, fmt("Failed to connect to target server."));
            syslog(LOG_WARNING, fmt("%s"), strerror(errno));
            close(sock);
            continue;
        }
        
        break;
    }

    if(curr == NULL) {
        syslog(LOG_ERR, fmt("Could not build a socket."));
        goto cleanup;
    }

    syslog(LOG_INFO, fmt("TCP connection established."));
    
    // Create a new canal data structure.
    struct canal * target = (struct canal *)malloc(sizeof(struct canal));
    memcpy(target, canal, sizeof(struct canal));
    target->target_sock = sock;

    // TODO move this to an error cleanup section
    free(target);

    // TODO start tcp to udt transfer thread
    // TODO start udt to tcp transfer thread
    // TODO join tcp to udt transfer thread
    // TODO join udt to tcp transfer thread

cleanup:
    freeaddrinfo(info); // NOTE this doesn't set NULL and not idempotent
    info = NULL;

    syslog(LOG_INFO, fmt("Closing TCP connection."));
    close(sock);

    syslog(LOG_INFO, fmt("Closing UDT connection."));
    udt.close(((struct canal *)canal)->source_sock);
    free(canal);

    return NULL;
};
#undef fmt//[Nydus][UDT Handle]



#define fmt(s) "[Nydus][UDT Accept] " s
static void udt_proxy_accept(int sock, struct canal canal) {

    while(1) {
        int conn;
        if((conn = udt.accept(sock, NULL, NULL)) < 0) {
            syslog(LOG_ERR, fmt("Error accepting connection on socket."));
            syslog(LOG_ERR, strerror(errno));
            continue;
        }
        
        syslog(LOG_INFO, fmt("New connection established:"));
        // TODO identify ipv4 and ipv6, print client info
        
        // Create a new canal data structure.
        struct canal * source = (struct canal *)malloc(sizeof(struct canal));
        memcpy(source, &canal, sizeof(struct canal));
        source->source_sock = conn;
        
        // New connection - let a handler handle it.
        pthread_t child;
        if(errno = pthread_create(&child, NULL, udt_handler, source)) {
            syslog(LOG_ERR, fmt("Error creating thread."));
            syslog(LOG_ERR, fmt("%s"), strerror(errno));
            free(source);
            continue;
        }
        pthread_detach(child);
    }

    udt.close(sock);

    return;
}
#undef fmt//[Nydus][UDT Accept]



#define fmt(s) "[Nydus][UDT] " s
void * udt_proxy (void * canal) {
    /**
     *  udt() - starts the udt proxy
     *
     *  @host:  null-terminated string indicating the server fqdn or ip
     *  @port:  null-terminated string indicating the server port
    **/

    int sock;
    struct addrinfo hint;
    struct addrinfo * info;

    const char * host = ((struct canal *)canal)->source_host;
    const char * port = ((struct canal *)canal)->source_port;

    syslog(LOG_INFO, fmt("Starting Nydus UDT proxy on %s:%s"), host, port);

    // Look up the server information.
    memset(&hint, 0, sizeof(struct addrinfo));
        hint.ai_family      = AF_UNSPEC; // Allow IPv4 (IPv6 soon)
        hint.ai_socktype    = SOCK_STREAM; // Request UDT socket
        hint.ai_flags       = AI_PASSIVE; // Whatever interface for now.
        hint.ai_protocol    = 0;
        hint.ai_canonname   = NULL;
        hint.ai_addr        = NULL;
        hint.ai_next        = NULL;

    if(getaddrinfo(host, port, &hint, &info)) {
        syslog(LOG_ERR, fmt("Failed to retrieve server info."));
        syslog(LOG_ERR, fmt("%s"), gai_strerror(errno));
        abort();
    }

    // Try each address in order until we find one that works.
    struct addrinfo * curr;
    for(curr = info; curr != NULL; curr = curr->ai_next) {
        
        // Create the server socket.
        sock = udt.socket(
            curr->ai_family,
            curr->ai_socktype,
            curr->ai_protocol
        );
        if(sock < 0){
            syslog(LOG_WARNING, fmt("Failed to create server socket."));
            syslog(LOG_WARNING, fmt("%s"), strerror(sock));
            continue;
        }
        
        // Bind the server socket.
        if(udt.bind(sock, curr->ai_addr, curr->ai_addrlen)) {
            syslog(LOG_WARNING, fmt("Failed to bind server socket."));
            syslog(LOG_WARNING, fmt("%s"), strerror(errno));
            close(sock);
            continue;
        }
        
        // TODO log information about the bound socket
        
        break;
    }

    if(curr == NULL) {
        syslog(LOG_ERR, fmt("Could not build a socket."));
        exit(1);
    }

    // Start listening for connections.
    syslog(LOG_INFO, fmt("Listening for new connections."));
    udt.listen(sock, SOMAXCONN);

    // Start accepting proxy requests.
    udt_proxy_accept(sock, *(struct canal *)canal);

cleanup:
    freeaddrinfo(info); // NOTE this doesn't set NULL and not idempotent
    info = NULL;

    udt.close(sock);

    free(canal);

    return NULL;
}
#undef fmt//[Nydus][UDT]



#define fmt(s) "[Nydus][UDT 2 TCP] " s
int udt2tcp(int a, int b) {
    // TODO switch to using a circular buffer...
    // TODO switch to using asynchronous read / write
    int size;
    char buffer[BUFFER_SIZE];
    size = udt.recv(a, buffer, BUFFER_SIZE, 0);
    if(size < 0) {
        // TODO handle errors
        // NOTE for now assume disconnect
    }

    while(size) {
        int sent = send(b, buffer, size, 0);
        if(size < 0) {
            // TODO handle errors
            // NOTE for now assume error
            goto error;
        }
        size -= sent;
    }
error:
    NULL;
}
#undef fmt//[Nydus][UDT 2 TCP]
