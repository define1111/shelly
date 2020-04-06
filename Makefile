CC = gcc
CFLAGS = -O2 -Wall -Wextra

run: shelly
	./shelly

shelly: ./sources/passes.c ./sources/parser.c ./sources/analyzer.c ./sources/main.c
	$(CC) $^ -o shelly $(CFLAGS)

clean:
	rm -rf shelly

