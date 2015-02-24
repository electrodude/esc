MODULES=spinc

.PHONY:		all clean modules

all:		modules

clean:		
		rm -vf *.so
		for module in ${MODULES}; do make -C $$module clean; done

modules:	${MODULES}.so

%.so:		FORCE
		make -C $* all $*.so
		cp $*/$*.so $@

FORCE:
