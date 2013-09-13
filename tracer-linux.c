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
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/inotify.h>

#include "frame.h"

const char *usage_str = "usage: %s [-t] <trace_path>\n";
int running = 1;

static void handle_signal(int sig)
{
	if (sig == SIGINT)
		running = 0;
}

static void process_events(int fd, int fd_d)
{
	struct inotify_event ev;

	if (read(fd, &ev, sizeof(ev)) > 0) {
		if (ev.mask & IN_MODIFY)
			read_frame(fd_d);
	}
}

static int check_event(int fd)
{
	fd_set rfds;
	FD_ZERO (&rfds);
	FD_SET (fd, &rfds);

	return select (FD_SETSIZE, &rfds, NULL, NULL, NULL);
}

int main(int argc, char **argv)
{
	int opt;
	int fd;
	int fd_d;
	int ret = 0;
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

	fd_d = open(argv[optind], flags);

	if (!(flags & O_TRUNC))
		while (read_frame(fd_d) > 0);

	fd = inotify_init();

	if (fd >= 0 && fd_d >= 0) {
		int wd;
		struct sigaction sa;

		sa.sa_handler = handle_signal;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGINT, &sa, NULL);

		wd = inotify_add_watch(fd, argv[optind], IN_MODIFY);

		while (running) {
			if (check_event(fd) > 0)
				process_events(fd, fd_d);
			else
				break;
		}

		fprintf(stderr, " Exiting..\n");

		close(fd_d);
		inotify_rm_watch(fd, wd);
	} else {
		fprintf(stderr, "unable to open \"%s\" for reading\n", argv[optind]);
		ret = EXIT_FAILURE;
	}

	return ret;
}
