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
#include <unistd.h>

#define ITM_ADDRESS     0xf8
#define ITM_HW_SOURCE   0x04
#define ITM_SIZE        0x03

#define ITM_OVERFLOW    0x70

extern int running;

static size_t handle_packet(uint8_t *buf, unsigned int offs, ssize_t nr)
{
    size_t nb = 1;

    if (buf[offs] & ITM_HW_SOURCE)
        printf("HW @ %02X ", (buf[offs] & ITM_ADDRESS) >> 3);

    switch (buf[offs] & ITM_SIZE) {
        case 0x01:
            if (offs + 1 < nr)
                printf("%c", buf[offs+1]);
            nb += sizeof(uint8_t);
            break;

        case 0x02:
            if (offs + 2 < nr)
                printf("[%02X]", *(uint16_t *)(&buf[offs+1]));
            nb += sizeof(uint16_t);
            break;

        case 0x03:
            if (offs + 4 < nr)
                printf("[%04X]", *(uint32_t *)(&buf[offs+1]));
            nb += sizeof(uint32_t);
            break;
    }

    if (buf[offs] & ITM_HW_SOURCE)
        putchar('\n');

    return nb;
}

ssize_t read_frame(int fd)
{
    unsigned char buf[512];
    ssize_t nr;

    nr = read(fd, buf, sizeof(buf));
    if (nr > 0) {
        unsigned int offs = 0;

        do {
            if (buf[offs] == 0 || buf[offs] == 0x80) { /* ignore sync */
                offs += 1;
            } else if (buf[offs] == ITM_OVERFLOW) {
                printf("[overflow]\n");
                offs += 1;
            } else {
                offs += handle_packet(buf, offs, nr);
            }
        } while (running && offs < nr);
    }

    return nr;
}
