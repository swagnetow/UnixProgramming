#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/times.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h> // For gethostname().

main() {
    int error_ret;
    char buffer[500];
    struct timeval mytv;
    char hostname[20];

    memset(buffer, 0, sizeof(buffer));
    gettimeofday(&mytv, NULL);
    gethostname(hostname, 20); // System call to get the hostname of the system.
    sprintf(&buffer[0],"Hello from %s on %s at %.*s!\n", "jpv", hostname, 24, ctime(&(mytv.tv_sec)));
    error_ret = write(1, buffer, sizeof(buffer));

    if(error_ret < 0)
        perror("My program failed with this error:");
}
