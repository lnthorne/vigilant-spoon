#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void msg(const char* msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char* msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void do_something(int connfd) {
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        msg("read() error");
        return;
    }

    fprintf(stderr, "client says %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf));
}

int main(void) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    // This is needed for most server applications
    // This turns on the SO_REUSEADDR option, this allows us to bind to the same network port
    // immediatly after the last instance finsihed.
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0); // wildcard 0.0.0.0

    int rv = bind(fd, (const struct sockaddr*) &addr, sizeof(addr));
    if (rv < 0) {
        die("bind{}");
    }

    // listen
    // SOMAXCONN is the size of the accepted queue for the listening socket. default on linuz is 4096
    rv = listen(fd, SOMAXCONN);
    if (rv < 0) {
        die("listen()");
    }

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);

        int connfd = accept(fd, (struct sockaddr*) &client_addr, &addrlen); // returns the peers address
        if (connfd < 0) {
            continue; // error
        }

        do_something(connfd);
        close(connfd);
    }

    return 0;
}