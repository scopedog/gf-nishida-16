include ../../common/Makefile.inc

EXECUTABLE	= gf-bench
MAIN		= gf-bench.c
INTERFACES	= ../../common/mt19937-64.c
SRCS		= $(MAIN) $(INTERFACES)
OBJS		= $(SRCS:.c=.o)
LIBS		= ../../common/gf-complete/libgf_complete.a
LIBPATH		= 
INCPATH		= -I../../common/ -I../../common/gf-complete
CFLAGS		= -Wall $(OPTFLAGS) -mmmx -msse -DINTEL_SSE -msse2 \
		  -DINTEL_SSE2 -msse3 -DINTEL_SSE3 -mssse3 -DINTEL_SSSE3 -fPIC \
		  $(INCPATH) 

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

