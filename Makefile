CFLAGS = -Wall -Werror -std=gnu99

include os-detect.inc
CFLAGS += -DOS=$(OS)

swo-tracer: main.c
	gcc $(CFLAGS) $^ -o $@

clean:
	@rm -f *.o swo-tracer
