#ifndef __NYDUS_TCP_H__
#define __NYDUS_TCP_H__

#include <nydus/nydus.h>
#include <cudt/cudt.h>

typedef struct {
    int (* const proxy)(
        char * source_host,
        char * source_port,
        char * target_host,
        char * target_port
    );
} nydus_tcp_namespace;
extern nydus_tcp_namespace const nydus_tcp;

#endif//__NYDUS_TCP_H__
