.PHONY: all

all: class-detect class-sort


# compile
class-detect: class-detect-utf8.c
	$(CC) -Wall -o $@ $^

class-sort: class-sort-utf8.c
	$(CC) -Wall -o $@ $^
