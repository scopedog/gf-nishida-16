include ../../../common/Makefile.inc

EXECUTABLE	= gf-bench
MAIN		= gf-bench.c
INTERFACES	= gf.c ../../../common/mt19937-64.c
SRCS		= $(MAIN) $(INTERFACES)
OBJS		= $(SRCS:.c=.o)
LIBS		= 
LIBPATH		= 
INCPATH		= -I../../../common/
CFLAGS		= -Wall $(OPTFLAGS) -maes $(INCPATH)

##################################################################

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

$(EXECUTABLE): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBPATH) $(LIBS)

all: $(EXECUTABLE)

clean:
	rm -f *.o *.core $(EXECUTABLE) $(LIBRARAY)

depend:
	$(MKDEP) $(CFLAGS) $(SRCS)

bench: $(EXECUTABLE)
	@basename `pwd`
	@./$(EXECUTABLE)

