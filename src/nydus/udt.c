#include <nydus/udt.h>


#define fmt(s) "[Nydus][UDT 2 TCP] " s
static int tcp2udt(int sock_tcp, int sock_udt) {
    // TODO switch to using a circular buffer...
    // TODO switch to using asynchronous read / write
    syslog(LOG_INFO, fmt("Sending data from UDT to TCP."));

    char * buffer = (char *)malloc(sizeof(char) * (BUFFER_SIZE + 1));

    size_t size = udt.recv(sock_udt, buffer, BUFFER_SIZE, 0);

    size_t sent = 0;
    while(sent < size) {
        size_t s = send(sock_tcp, buffer + sent, size - sent, 0);
        if(s < 0) {
            syslog(LOG_ERR, fmt("Error during TCP transfer."));
            syslog(LOG_ERR, fmt("%u"), strerror(errno));
            sent = -1;
            break;
        }
        sent += s;
    }

    free(buffer);

    return sent;
}
#undef fmt//[Nydus][UDT 2 TCP]


#define fmt(s) "[Nydus][TCP 2 UDT] " s
static int udt2tcp(int sock_udt, int sock_tcp) {
    // TODO switch to using a circular buffer...
    // TODO switch to using asynchronous read / write
    syslog(LOG_INFO, fmt("Receiving data from TCP to UDT."));

    char * buffer = (char *)malloc(sizeof(char) * (BUFFER_SIZE + 1));

    size_t size = recv(sock_tcp, buffer, BUFFER_SIZE, 0);

    size_t sent = 0;
    while(sent < size) {
        size_t s = udt.send(sock_udt, buffer + sent, size - sent, 0);
        if(s < 0) {
            syslog(LOG_ERR, fmt("Error during TCP transfer."));
            syslog(LOG_ERR, fmt("%u"), udt.getlasterror_code());
            sent = -1;
            break;
        }
        sent += s;
    }

    free(buffer);

    return sent;
}
#undef fmt//[Nydus][TCP 2 UDT]


struct udt_handler_params {
    int source;
    char * target_host;
    char * target_port;
};


#define fmt(s) "[Nydus][UDT Handle] " s
static void * udt_handler(void * params) {
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
    syslog(LOG_INFO, fmt("Handling connection %lu"), params);

    int target;
    struct addrinfo hints;
    struct addrinfo * info;

    int source = ((struct udt_handler_params *)params)->source;
    char * host = ((struct udt_handler_params *)params)->target_host;
    char * port = ((struct udt_handler_params *)params)->target_port;

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
        goto cleanup;
    }

    // Try each address in order until we find one that works.
    struct addrinfo * curr;
    for(curr = info; curr != NULL; curr = curr->ai_next) {
        
        // Create the server socket.
        target = socket(
            curr->ai_family,
            curr->ai_socktype,
            curr->ai_protocol
        );
        if(target < 0){
            syslog(LOG_WARNING, fmt("Failed to create server socket."));
            syslog(LOG_WARNING, fmt("%s"), strerror(target));
            continue;
        }
        
        // Connect to the other side.
        if(connect(target, curr->ai_addr, curr->ai_addrlen)) {
            syslog(LOG_WARNING, fmt("Failed to connect to target server."));
            syslog(LOG_WARNING, fmt("%s"), strerror(errno));
            close(target);
            continue;
        }
        
        break;
    }

    if(curr == NULL) {
        syslog(LOG_ERR, fmt("Could not build a socket."));
        goto cleanup;
    }

    syslog(LOG_INFO, fmt("TCP connection established."));

    // TODO put this into a thread
    size_t sent = 0;
    do {
        sent = udt2tcp(source, target);
    } while (sent > 0);

    // TODO start tcp to udt transfer thread
    // TODO start udt to tcp transfer thread
    // TODO join tcp to udt transfer thread
    // TODO join udt to tcp transfer thread

cleanup:
    freeaddrinfo(info); // NOTE this doesn't set NULL and not idempotent
    info = NULL;

    syslog(LOG_INFO, fmt("Closing TCP connection."));
    close(target);

    syslog(LOG_INFO, fmt("Closing UDT connection."));
    udt.close(source);

    return NULL;
};
#undef fmt//[Nydus][UDT Handle]


#define fmt(s) "[Nydus][UDT Accept] " s
static void udt_proxy_accept(
    int sock,
    char * target_host,
    char * target_port
) {

    while(1) {
        int source;
        if((source = udt.accept(sock, NULL, NULL)) < 0) {
            syslog(LOG_ERR, fmt("Error accepting connection on socket."));
            syslog(LOG_ERR, strerror(errno));
            continue;
        }
        
        syslog(LOG_INFO, fmt("New connection established:"));
        // TODO identify ipv4 and ipv6, print client info

        struct udt_handler_params * params = (struct udt_handler_params *)malloc(sizeof(struct udt_handler_params));
            params->source = source;
            params->target_host = target_host;
            params->target_port = target_port;
        
        // New connection - let a handler handle it.
        pthread_t child;
        if(errno = pthread_create(&child, NULL, udt_handler, params)) {
            syslog(LOG_ERR, fmt("Error creating thread."));
            syslog(LOG_ERR, fmt("%s"), strerror(errno));
            free(params);
            continue;
        }
        pthread_detach(child);
    }

    udt.close(sock);

    return;
}
#undef fmt//[Nydus][UDT Accept]


#define fmt(s) "[Nydus][UDT] " s
static int nydus_udt_proxy (
    char * source_host,
    char * source_port,
    char * target_host,
    char * target_port
) {
    /**
     *  udt() - starts the udt proxy
     *
     *  @host:  null-terminated string indicating the server fqdn or ip
     *  @port:  null-terminated string indicating the server port
    **/

    int sock;
    struct addrinfo hint;
    struct addrinfo * info;

    syslog(LOG_INFO,
        fmt("Starting Nydus UDT proxy on %s:%s"),
        source_host,
        source_port
    );

    // Look up the server information.
    memset(&hint, 0, sizeof(struct addrinfo));
        hint.ai_family      = AF_INET; // Allow IPv4 (IPv6 soon)
        hint.ai_socktype    = SOCK_STREAM; // Request UDT socket
        hint.ai_flags       = AI_PASSIVE; // Whatever interface for now.
        hint.ai_protocol    = 0;
        hint.ai_canonname   = NULL;
        hint.ai_addr        = NULL;
        hint.ai_next        = NULL;

    if(getaddrinfo(source_host, source_port, &hint, &info)) {
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
    udt_proxy_accept(sock, target_host, target_port);

    freeaddrinfo(info); // NOTE this doesn't set NULL and not idempotent
    info = NULL;

    udt.close(sock);

    return 0;
}
#undef fmt//[Nydus][UDT]


struct nydus const nydus_udt = {
    nydus_udt_proxy,
};
