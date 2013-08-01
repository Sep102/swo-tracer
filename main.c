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
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>

const char *usage_str = "usage: %s [-t] <trace_path>\n";
static int running = 0;

void read_frame(int fd)
{
	unsigned char buf[512];
	ssize_t nr;

	nr = read(fd, buf, sizeof(buf));
	if (nr > 0) {
		unsigned int offs = 0;

		do {
			switch (buf[offs]) {
				case 1:
					if (offs + 1 < nr)
						printf("%c", buf[offs+1]);
					offs += 1 + sizeof(uint8_t);
					break;

				case 2:
					if (offs + 2 < nr)
						printf("[%02X]", *(uint16_t *)(&buf[offs+1]));
					offs += 1 + sizeof(uint16_t);
					break;

				case 3:
					if (offs + 4 < nr)
						printf("[%04X]", *(uint32_t *)(&buf[offs+1]));
					offs += 1 + sizeof(uint32_t);
					break;

				default:
					fprintf(stderr, "unknown designator 0x%X\n", buf[offs]);
					offs++;
					break;
			}
		} while (running && offs < nr);
	}
}

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

	fd = open(argv[optind], flags);

	if (fd >= 0) {
		struct sigaction sa;
		fd_set rfds;
		struct timeval tv;
		int retval;

		sa.sa_handler = handle_signal;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGINT, &sa, NULL);

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		running = 1;
		while (running) {
			retval = select(fd + 1, &rfds, NULL, NULL, &tv);

			if (retval == -1) {
				perror("select()");
				break;
			}

			if (FD_ISSET(fd, &rfds)) {
				read_frame(fd);
			}
		}

		fprintf(stderr, " Exiting..\n");

		close(fd);
		ret = 0;
	} else {
		fprintf(stderr, "unable to open \"%s\" for reading\n", argv[1]);

		ret = EXIT_FAILURE;
	}

	return ret;
}
