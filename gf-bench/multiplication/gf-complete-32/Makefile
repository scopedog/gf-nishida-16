include ../../common/Makefile.inc

ARCH		!= uname -m
EXECUTABLE	= gf-bench
MAIN		= gf-bench.c
INTERFACES	= ../../common/mt19937-64.c
SRCS		= $(MAIN) $(INTERFACES)
OBJS		= $(SRCS:.c=.o)
LIBS		= -lgf_complete
LIBPATH		= -L/usr/local/lib
INCPATH		= -I../../common/ -I/usr/local/include
CFLAGS		= -Wall $(OPTFLAGS) -D_$(ARCH)_ $(INCPATH)

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

