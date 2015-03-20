#ifndef __C_NYDUS_H__
#define __C_NYDUS_H__

#include <sys/socket.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <argp.h>

// Socket libraries.
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

// Error handling / logging.
#include <errno.h>
#include <syslog.h>

// Threading / event handling.
#include <pthread.h>
#include <sys/epoll.h>

// Nydus data structures.
#include <nydus/canal.h>

// Transport protocols.
#include <nydus/tcp.h>
#include <nydus/udt.h>


#define BUFFER_SIZE 67108864

typedef struct {

    void * (* const tcp_handler)(void * conn);
    void * (* const udt_handler)(void * conn);
    void * (* const tcp_proxy)(struct canal * canal);
    void * (* const udt_proxy)(struct canal * canal);

} nydus_namespace;
extern nydus_namespace const nydus;

extern void * tcp_proxy(void * canal);
extern void * udt_proxy(void * canal);

#endif//__C_NYDUS_H__
