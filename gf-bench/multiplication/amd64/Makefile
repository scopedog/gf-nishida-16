###########################################################################
#
#	Makefile							
#
###########################################################################

MAKE	= make

SUBDIR	= gf-solaris-128

###########################################################################

all:	subdirs

subdirs:
	for i in $(SUBDIR) ; do \
		( cd $$i ; $(MAKE) ) ; \
	done

clean:
	for i in $(SUBDIR) ; do \
		( cd $$i ; $(MAKE) clean ; rm -f .depend ) ; \
	done

depend:
	for i in $(SUBDIR) ; do \
		( cd $$i ; $(MAKE) depend ) ; \
	done

bench:
	for i in $(SUBDIR) ; do \
		( cd $$i ; $(MAKE) bench ) ; \
	done
