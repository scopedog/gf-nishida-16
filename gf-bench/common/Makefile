CC		= cc
LIBRARY		= libgf.a
INTERFACES	= gf.c mt19937-64.c
SRCS		= $(INTERFACES)
OBJS		= $(SRCS:.c=.o)
LIBS		= 
LIBPATH		= 
INCPATH		= 
CFLAGS		= -Wall -Ofast $(INCPATH)

##################################################################

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

$(LIBRARY): $(OBJS)
	ar rcu $@ $^
	ranlib $@

all: $(LIBRARY)

clean:
	rm -f *.o *.core $(LIBRARY)

depend:
	mkdep $(CFLAGS) $(SRCS)

bench : $(EXECUTABLE)
	@basename `pwd`
	@./$(EXECUTABLE)

