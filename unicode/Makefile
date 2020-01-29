# To reconstruct or upgrade the module in unicode.h, unicode.c:
# 1) make and run unigen
# 2) make and run unicode for testing

# Find the OS platform using the uname command.
Linux := $(findstring Linux, $(shell uname -s))
MacOS := $(findstring Darwin, $(shell uname -s))
Windows := $(findstring NT, $(shell uname -s))

DATA = UnicodeData.txt GraphemeBreakProperty.txt emoji-data.txt
TESTS = GraphemeBreakTest.txt
DEBUG = -g -fsanitize=undefined -fsanitize=address
ifdef Windows
	DEBUG = -g
endif

unigen: unigen.c unicode.h unicode.c $(DATA) $(TESTS)
	gcc -std=c11 -Wall $(DEBUG) unigen.c -o unigen

unigen2: unigen2.c unicode.h unicode.c $(DATA) $(TESTS)
	gcc -std=c11 -Wall $(DEBUG) unigen2.c -o unigen2

unicode: unicode.h unicode.c
	gcc -std=c11 -Wall -DunicodeTest $(DEBUG) unicode.c -o unicode
