include ../common/Makefile.inc

ARCH		!= ../common/det-arch.sh
EXECUTABLE	= gf-bench-all
MAIN		= $(EXECUTABLE).c
INTERFACES	= 
SRCS		= $(MAIN) $(INTERFACES)
OBJS		= $(SRCS:.c=.o)
LIBS		= 
LIBPATH		= 
INCPATH		= -I../common/
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

