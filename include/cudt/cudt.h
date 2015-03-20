#ifndef __C_UDT_H__
#define __C_UDT_H__

#include <sys/socket.h>

typedef struct {

int (* const socket)(int domain, int type, int protocol);
int (* const bind)(int sock, const struct sockaddr * addr, socklen_t addrlen);
int (* const listen)(int sock, int backlog);
int (* const accept)(int sock, struct sockaddr * addr, socklen_t * addrlen);
int (* const connect)(int sock, const struct sockaddr * addr, socklen_t addrlen);
int (* const close)(int sock);
int (* const getsockopt)(int sock, int level, int optname, void * optval, socklen_t * optlen);
int (* const setsockopt)(int sock, int level, int optname, const void * optval, socklen_t optlen);
int (* const send)(int sock, const void * buf, size_t len, int flags);
int (* const recv)(int sock, void * buf, size_t len, int flags);
int (* const getlasterror_code)();

} udt_namespace;
extern udt_namespace const udt;

#endif//__C_UDT_H__
