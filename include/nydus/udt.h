#ifndef __NYDUS_UDT_H__
#define __NYDUS_UDT_H__

#include <nydus/nydus.h>
#include <cudt/cudt.h>

typedef struct {
    int (* const proxy)(
        char * source_host,
        char * source_port,
        char * target_host,
        char * target_port
    );
} nydus_udt_namespace;
extern nydus_udt_namespace const nydust_udt;

#endif//__NYDUS_UDT_H__
