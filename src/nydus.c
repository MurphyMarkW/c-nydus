#include <nydus.h>


// Endpoint configurations for the nydus canal.
// NOTE volatile to allow for on-the-fly reconfiguration

volatile const char * tcp_source_host = "localhost";
volatile const char * tcp_source_port = "9000";

volatile const char * udt_source_host = "0.0.0.0";
volatile const char * udt_source_port = "9000";

volatile const char * tcp_target_host = "localhost";
volatile const char * tcp_target_port = "9090";

volatile const char * udt_target_host = "0.0.0.0";
volatile const char * udt_target_port = "9090";


#define fmt(s) "[Nydus] " s
int main(int argc, char **argv) {
    // TODO use argp for argument parsing

    // Tell syslog to print to stderr as well.
    openlog(NULL, LOG_PERROR, LOG_USER);

    struct canal * tcp_canal = (struct canal *)malloc(sizeof(struct canal));
        tcp_canal->source_host = (const char *)tcp_source_host;
        tcp_canal->source_port = (const char *)tcp_source_port;
        tcp_canal->target_host = (const char *)udt_target_host;
        tcp_canal->target_port = (const char *)udt_target_port;

    struct canal * udt_canal = (struct canal *)malloc(sizeof(struct canal));
        udt_canal->source_host = (const char *)udt_source_host;
        udt_canal->source_port = (const char *)udt_source_port;
        udt_canal->target_host = (const char *)tcp_target_host;
        udt_canal->target_port = (const char *)tcp_target_port;

    pthread_t tcp_child;
    if(errno = pthread_create(&tcp_child, NULL, tcp_proxy, tcp_canal)) {
        syslog(LOG_ERR, fmt("Error creating thread."));
        syslog(LOG_ERR, fmt("%s"), strerror(errno));
    }
    pthread_detach(tcp_child);

    pthread_t udt_child;
    if(errno = pthread_create(&udt_child, NULL, udt_proxy, udt_canal)) {
        syslog(LOG_ERR, fmt("Error creating thread."));
        syslog(LOG_ERR, fmt("%s"), strerror(errno));
    }
    pthread_detach(udt_child);

    for(;;) {
        pause();
        // TODO check errno for craceful shutdown
    }

    closelog();

    return 0;
}
#undef fmt//[Nydus]
