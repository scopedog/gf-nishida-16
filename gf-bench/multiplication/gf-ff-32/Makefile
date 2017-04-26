CC		= cc
EXECUTABLE	= gf-bench
MAIN		= gf-bench.c
INTERFACES	= ff_2_163.c
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

