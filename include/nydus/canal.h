#ifndef __C_NYDUS_CANAL_H__
#define __C_NYDUS_CANAL_H__

typedef struct canal {

    int source_sock;
    int target_sock;
    const char * source_host;
    const char * source_port;
    const char * target_host;
    const char * target_port;

} canal;

#endif//__C_NYDUS_CANAL_H__
