###########################################################################
#
#	Makefile							
#
###########################################################################

MAKE	= make

SUBDIR	= common multiplication division

###########################################################################

all:	subdirs

subdirs:
	for i in $(SUBDIR) ; do \
		( cd $$i ; $(MAKE) ) ; \
	done

clean:
	for i in $(SUBDIR) ; do \
		( cd $$i ; $(MAKE) clean ) ; \
	done

depend:
	for i in $(SUBDIR) ; do \
		( cd $$i ; $(MAKE) depend ) ; \
	done

bench:
	for i in $(SUBDIR) ; do \
		( cd $$i ; $(MAKE) bench ) ; \
	done
