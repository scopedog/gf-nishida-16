# This is for compiling gf-bench.c that measures the computation
# time taken by multiplication and division with gf-nishida-16.
# mt19937-64.c is used only for generating random integers.

CC		= cc
EXECUTABLE	= gf-bench
MAIN		= gf-bench.c
INTERFACES	= gf.c mt19937-64.c
SRCS		= $(MAIN) $(INTERFACES)
OBJS		= $(SRCS:.c=.o)
LIBS		= 
LIBPATH		= 
INCPATH		= 
CFLAGS		= -Wall -O2 -g

##################################################################

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

$(EXECUTABLE) : $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBPATH) $(LIBS)

all : $(EXECUTABLE)

clean:
	rm -f *.o *.core $(EXECUTABLE) $(LIBRARAY)

depend:
	mkdep $(CFLAGS) $(SRCS)

bench : $(EXECUTABLE)
	@basename `pwd`
	@./$(EXECUTABLE)

