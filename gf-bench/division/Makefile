###########################################################################
#
#	Makefile							
#
###########################################################################

MAKE	= make

SUBDIR	= gf-nishida-8 gf-nishida-16 gf-nishida-region-16 \
	  gf-complete-32 gf-complete-64 \
	  gf-plank-8 gf-plank-16 gf-plank-32 gf-plank-logtable-16 \
	  gf-ff-32 gf-ff-64 \
	  gf-sensor608-8

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
