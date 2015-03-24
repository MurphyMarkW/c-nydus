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

#define BUFFER_SIZE 67108864

#endif//__C_NYDUS_H__
