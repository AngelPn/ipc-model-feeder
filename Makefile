OBJS= main.o
CFLAGS = -g -Wall -I.
PROGRAM= ex

$(PROGRAM): clean $(OBJS)
	g++ -g $(OBJS) -o $(PROGRAM)

clean:
	rm -f $(PROGRAM) $(OBJS)