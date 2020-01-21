# Modules forming the editor's model in dependency order.
history = history.c
lines = lines.c
cursors = cursors.c history.c
action = action.c

# Find the OS platform using the uname command (using MSYS2 on Windows)
Linux := $(findstring Linux, $(shell uname -s))
MacOS := $(findstring Darwin, $(shell uname -s))
Windows := $(findstring NT, $(shell uname -s))

# Set up the compiler options for production or debugging.
FLAGS = -std=c11 -Wall -pedantic
PRODUCTION = $(FLAGS) -O2 -flto
DEBUGGING = $(FLAGS) -g -fsanitize=undefined -fsanitize=address
ifdef Windows
	DEBUGGING = $(FLAGS) -g
endif

# Build and test a module. Always use the same executable.
%: %.c
	gcc -D$@Test $(DEBUGGING) $($@) -o snipe
	./snipe

# Build the whole model, as a library.
model:
	gcc $(PRODUCTION) -c $($@)
	ar rcs model.a *.o
	rm *.o
