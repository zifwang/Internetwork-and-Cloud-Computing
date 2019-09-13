  
# To build the program, simply type:
#   make
#
# To clean up and remove the compiled binary and other generated files, type:
#   make clean
CC = g++
CFLAGS=-c -g -std=c++11
SOURCES=server.cpp argParser.cpp packet.cpp
OBJECTS=$(SOURCES:.cpp=.o)
LIBS=
LDFLAGS=
PROGRAM=server

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) -o $(PROGRAM) $(OBJECTS) $(LDFLAGS) $(LIBS) 

.cpp.o: 
	$(CC) $(CFLAGS) $<

depend: .depend

.depend: $(SOURCES)
	rm -f .depend
	$(CC) $(CFLAGS) -MM $^ >> .depend;

include .depend

clean:
	rm -f .depend $(PROGRAM) $(OBJECTS)