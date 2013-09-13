/* vim: set sw=8: */
/* swo-tracer: listen for and parse ARM SWO trace output
 *
 * Copyright (c) 2013 Andrey Yurovsky
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the author.  The name of the
 * author may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include "frame.h"

const char *usage_str = "usage: %s [-t] <trace_path>\n";
int running = 1;

void handle_signal(int sig)
{
    if (sig == SIGINT)
        running = 0;
}

int main(int argc, char **argv)
{
    int opt;
    int ret;
    int fd;
    int flags = O_RDONLY;

    while ((opt = getopt(argc, argv, "t")) != -1) {
        switch (opt) {
            case 't':
            flags = (O_RDWR | O_TRUNC);
            break;

            default:
            fprintf(stderr, usage_str, argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, usage_str, argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = open(argv[optind], flags | O_ASYNC);

    if (fd >= 0) {
        struct sigaction sa;

        sa.sa_handler = handle_signal;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);

        int q = kqueue();
        struct kevent ev;
        memset(&ev, 0, sizeof(ev));

        ev.ident = fd;
        ev.filter = EVFILT_READ;
        ev.flags = EV_ADD;

        kevent(q, &ev, 1, NULL, 0, NULL);

        while (running) {
                int res = kevent(q, NULL, 0, &ev, 1, NULL);
                if (!res) {
                  printf("kevent failed: %d\n", errno);
                }

                read_frame(fd);
        }

        fprintf(stderr, " Exiting..\n");

        close(fd);
        ret = 0;
    } else {
        fprintf(stderr, "unable to open \"%s\" for reading\n", argv[optind]);

        ret = EXIT_FAILURE;
    }

    return ret;
}
