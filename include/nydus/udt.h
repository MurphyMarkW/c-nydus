#ifndef __NYDUS_UDT_H__
#define __NYDUS_UDT_H__

#include <nydus/nydus.h>

typedef struct {

int (* const server)(struct canal * canal, int (*client)(struct canal * canal));
int (* const client)(struct canal * canal);

} nydus_udt_namespace;
extern nydus_udt_namespace const nydus_udt;

extern int nydus_udt_server(struct canal * canal, int (*client)(struct canal * canal));
extern int nydus_udt_client(struct canal * canal);

#endif//__NYDUS_UDT_H__
