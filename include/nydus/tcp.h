#ifndef __NYDUS_TCP_H__
#define __NYDUS_TCP_H__

#include <nydus/nydus.h>
#include <nydus/canal.h>

typedef struct {

int (* const server)(struct canal * canal, int (*client)(struct canal * canal));
int (* const client)(struct canal * canal);

} nydus_tcp_namespace;
extern nydus_tcp_namespace const nydus_tcp;

extern int nydus_tcp_server(struct canal * canal, int (*client)(struct canal * canal));
extern int nydus_tcp_client(struct canal * canal);

#endif//__NYDUS_TCP_H__
