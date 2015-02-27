CC = g++
LD = g++

CFLAGS = -c -pedantic-errors -std=c++11 -Wall -fno-elide-constructors
LFLAGS = -pedantic -Wall

OBJS = p3_main.o Record.o Collection.o Utility.o
PROG = p3exe

default: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(LFLAGS) $(OBJS) -o $(PROG)

p3_main.o: p3_main.cpp Record.h Collection.h Utility.h
	$(CC) $(CFLAGS) p3_main.cpp

Record.o: Record.cpp Record.h Utility.h
	$(CC) $(CFLAGS) Record.cpp

Collection.o: Collection.cpp Collection.h Record.h Utility.h
	$(CC) $(CFLAGS) Collection.cpp

Utility.o: Utility.cpp Utility.h
	$(CC) $(CFLAGS) Utility.cpp

clean:
	rm -f *.o

real_clean:
	rm -f *.o
	rm -f *exe

