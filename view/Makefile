# Modules forming the editor's model in dependency order.
event = event.c
display = display.c event.c

# Find the OS platform using the uname command (using MSYS2 on Windows)
Linux := $(findstring Linux, $(shell uname -s))
MacOS := $(findstring Darwin, $(shell uname -s))
Windows := $(findstring NT, $(shell uname -s))

# Set up the compiler options for production or debugging.
ALLEGRO = -lallegro -lallegro_main -lallegro_font -lallegro_ttf
FLAGS = -std=c11 -Wall -pedantic $(ALLEGRO)
PRODUCTION = $(FLAGS) -O2 -flto
DEBUGGING = $(FLAGS) -g -fsanitize=undefined -fsanitize=address
ifdef Windows
	DEBUGGING = $(FLAGS) -g
endif

# Build a module. Always use the same executable.
%: %.c
	gcc -D$@Test $($@) $(DEBUGGING) -o snipe

# Run a module. This is separate from compiling, since graphics is involved.
# Switch off leak detection as system graphics libraries leak like crazy.
run:
	ASAN_OPTIONS=detect_leaks=0 ./snipe

# Build the whole view, as a library.
view:
	gcc -c $($@) $(PRODUCTION)
	ar rcs view.a *.o
	rm *.o
