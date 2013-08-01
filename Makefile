CFLAGS = -Wall -pedantic -Werror

swo-tracer: main.c
	gcc $(CFLAGS) $^ -o $@

clean:
	@rm -f *.o swo-tracer
