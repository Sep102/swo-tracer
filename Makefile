CFLAGS = -Wall -Werror -std=gnu99

include os-detect.inc

SRCS = tracer-$(OS).c frame.c

OBJS = $(patsubst %.c,%.o,$(SRCS))

swo-tracer: $(OBJS)
	gcc $(CFLAGS) $^ -o $@

clean:
	@rm -f $(OBJS) swo-tracer
