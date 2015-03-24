#include <nydus/udt.h>


// UDT host and port.
char * source_host = "localhost";
char * source_port = "9000";

// TCP host and port.
char * target_host = "localhost";
char * target_port = "9090";


#define fmt(s) "[Nydus] " s
int main(int argc, char **argv) {
    // TODO use argp for argument parsing

    // Tell syslog to print to stderr as well.
    openlog(NULL, LOG_PERROR, LOG_USER);

    int retval = nydus_udt.proxy(
        source_host,
        source_port,
        target_host,
        target_port
    );

    closelog();

    return 0;
}
#undef fmt//[Nydus]
