#include <udt.h>
#include <cudt.h>

namespace cudt {

static int socket(int domain, int type, int protocol) {
    return UDT::socket(domain, type, protocol);
}

static int bind(int sock, const struct sockaddr * addr, socklen_t addrlen) {
    return UDT::bind(sock, addr, (int)addrlen);
}

static int listen(int sock, int backlog) {
    return UDT::listen(sock, backlog);
}

static int accept(int sock, struct sockaddr * addr, socklen_t * addrlen) {
    // TODO check if addrlen_t * -> int * is acceptable
    return UDT::accept(sock, addr, (int*)addrlen);
}

static int connect(int sock, const struct sockaddr * addr, socklen_t addrlen) {
    // TODO check if addrlen_t * -> int * is acceptable
    return UDT::connect(sock, addr, (int)addrlen);
}

static int close(int sock) {
    return UDT::close(sock);
}

static int getsockopt(int sock, int level, int optname, void * optval, socklen_t * optlen) {
    // TODO can we get rid of the UDT SOCKOPT?
    // TODO check if addrlen_t * -> int * is acceptable
    return UDT::getsockopt(sock, level, (UDT::SOCKOPT)optname, optval, (int*)optlen);
}

static int setsockopt(int sock, int level, int optname, const void * optval, socklen_t optlen) {
    // TODO can we get rid of the UDT SOCKOPT?
    return UDT::setsockopt(sock, level, (UDT::SOCKOPT)optname, optval, (int)optlen);
}

static int send(int sock, const void * buf, size_t len, int flags) {
    return UDT::send(sock, (const char*)buf, len, flags);
}

static int recv(int sock, void * buf, size_t len, int flags) {
    return UDT::recv(sock, (char*)buf, len, flags);
}

static int getlasterror_code() {
    return UDT::getlasterror_code();
}

}

udt_namespace const udt = {
    cudt::socket,
    cudt::bind,
    cudt::listen,
    cudt::accept,
    cudt::connect,
    cudt::close,
    cudt::getsockopt,
    cudt::setsockopt,
    cudt::send,
    cudt::recv,
    cudt::getlasterror_code,
};
