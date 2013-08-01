CFLAGS = -Wall -Werror -std=gnu99

swo-tracer: main.c
	gcc $(CFLAGS) $^ -o $@

clean:
	@rm -f *.o swo-tracer
