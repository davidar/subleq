SRCS = boot.asq menu.asq \
       puts.asq gets.asq putint.asq getint.asq \
       bubblesort.asq calc.asq factorial.asq primes.asq \
       ascii.asq data.asq

CC = cc

all: asq sq

asq:
	cat $(SRCS) | ../thirdparty/sqasm > image.sq

sq:
	$(CC) sq.c -o sq

clean:
	rm image.sq sq
