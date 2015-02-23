MODULES=spinc

.PHONY:		all clean modules

all:		modules

clean:		
		rm -vf *.so
		for module in ${MODULES}; do make -C $$module clean; done

modules:	${MODULES}.so

%.so:		
		make -C $* all $*.so
		cp $*/$*.so $@
