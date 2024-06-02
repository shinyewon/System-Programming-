CC = gcc
CFLAGS = -Wall

all: main child

main: main.c common.h
	$(CC) $(CFLAGS) -o main main.c

child: child.c common.h
	$(CC) $(CFLAGS) -o child child.c

clean:
	rm -f main child

