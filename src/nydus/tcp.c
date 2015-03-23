#include <nydus/nydus.h>
#include <cudt/cudt.h>


#define fmt(s) "[Nydus][TCP Handle] " s
static void * tcp_handler(void * canal) {
    /**
     *  tcp_handler() - handles tcp connections
     *
     *  @conn:  pointer to tcp socket
     *
     *  Handles TCP connections to the Nydus proxy by reading data from the TCP
     *  socket and writing it to a UDT socket, as well as the reverse.
    **/
    // TODO set up new udt connection
    // TODO read from tcp socket
    // TODO write to udt socket
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
        
        // Connect to the other side.
        if(udt.connect(sock, curr->ai_addr, curr->ai_addrlen)) {
            syslog(LOG_WARNING, fmt("Failed to connect to target server."));
            syslog(LOG_WARNING, fmt("UDT error code: %u"), udt.getlasterror_code());
            close(sock);
            continue;
        }
        
        break;    
    }

    if(curr == NULL) {
        syslog(LOG_ERR, fmt("Could not build a socket."));
        goto cleanup;
    }
    
    syslog(LOG_INFO, fmt("UDT connection established."));

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

    syslog(LOG_INFO, fmt("Closing UDT connection."));
    udt.close(sock);

    syslog(LOG_INFO, fmt("Closing TCP connection."));
    close(((struct canal *)canal)->source_sock);
    free(canal);

    return NULL;
};
#undef fmt//[Nydus][TCP Handle]



#define fmt(s) "[Nydus][TCP Accept] " s
static void tcp_proxy_accept(int sock, struct canal canal) {

    while(1) {
        int conn;
        if((conn = accept(sock, NULL, NULL)) < 0) {
            syslog(LOG_ERR, fmt("Error accepting connection on socket."));
            syslog(LOG_ERR, strerror(errno));
            continue;
        }
        
        syslog(LOG_INFO, fmt("New connection established."));
        // TODO identify ipv4 and ipv6, print client info
        
        // Create a new canal data structure.
        struct canal * source = (struct canal *)malloc(sizeof(struct canal));
        memcpy(source, &canal, sizeof(struct canal));
        source->source_sock = conn;
        
        // New connection - let a handler handle it.
        pthread_t child;
        if(errno = pthread_create(&child, NULL, tcp_handler, source)) {
            syslog(LOG_ERR, fmt("Error creating thread."));
            syslog(LOG_ERR, fmt("%s"), strerror(errno));
            free(source);
            continue;
        }
        pthread_detach(child);
    }

    close(sock);

    return;
}
#undef fmt//[Nydus][TCP Accept]



#define fmt(s) "[Nydus][TCP] " s
void * tcp_proxy(void * canal) {
    /**
     *  tcp() - starts the tcp proxy
     *
     *  @host:  null-terminated string indicating the server fqdn or ip
     *  @port:  null-terminated string indicating the server port
    **/

    int sock;
    struct addrinfo hint;
    struct addrinfo * info;

    const char * host = ((struct canal *)canal)->source_host;
    const char * port = ((struct canal *)canal)->source_port;

    syslog(LOG_INFO, fmt("Starting Nydus TCP proxy on %s:%s"), host, port);

    // Look up the server information.
    memset(&hint, 0, sizeof(struct addrinfo));
        hint.ai_family      = AF_INET; // Allow IPv4 (IPv6 soon)
        hint.ai_socktype    = SOCK_STREAM; // Request TCP socket
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
        
        // Bind the server socket.
        if(bind(sock, curr->ai_addr, curr->ai_addrlen)) {
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
    listen(sock, SOMAXCONN);

    // Start accepting proxy requests.
    tcp_proxy_accept(sock, *(struct canal *)canal);

cleanup:
    freeaddrinfo(info); // NOTE this doesn't set NULL and not idempotent
    info = NULL;

    close(sock);

    free(canal);

    return NULL;
}
#undef fmt//[Nydus][TCP]



// TODO get some better formatting for syslog
#define fmt(s) "[Nydus][TCP 2 UDT] " s
int tcp2udt(int sock_tcp, int sock_udt) {
    // TODO switch to using a circular buffer...
    // TODO switch to using asynchronous read / write
    size_t size;
    char buffer[BUFFER_SIZE];
    size = recv(sock_tcp, buffer, BUFFER_SIZE, 0);
    return udt.send(sock_udt, buffer, size, 0);
}
#undef fmt//[Nydus][TCP 2 UDT]
