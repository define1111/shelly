SRCDIR = sources
SRCFILES += $(SRCDIR)/main.c $(SRCDIR)/command.c $(SRCDIR)/shelly_string.c
SRCFILES += $(SRCDIR)/parser.c $(SRCDIR)/passes.c
SRCFILES += $(SRCDIR)/pass_open_files.c $(SRCDIR)/pass_builtin_commands.c
OBJFILES = $(SRCFILES:.c=.o)

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

ifdef ASAN
CC = clang
CFLAGS = -O1 -g -fsanitize=address
else
CC = gcc
CFLAGS = -O2 -Wall -Wextra
endif

run: shelly
	./shelly

shelly: $(OBJFILES)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -rf $(SRCDIR)/*.o shelly
