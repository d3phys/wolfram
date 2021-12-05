#
# List data structure makefile
#
# Important! Dependencies are done automatically by 'make dep', which also
# removes any old dependencies. Do not modify it...
# 2021, d3phys
#

OBJS  = logs.o stack.o hash.o iommap.o

lib.o: $(OBJS) subdirs
	$(LD) -r -o $@ $(OBJS)

include $(TOPDIR)/Rules.makefile

### Dependencies ###
hash.o: hash.cpp /home/d3phys/Code/wolfram/include/hash.h
iommap.o: iommap.cpp /home/d3phys/Code/wolfram/include/iommap.h
logs.o: logs.cpp /home/d3phys/Code/wolfram/include/logs.h
stack.o: stack.cpp /home/d3phys/Code/wolfram/include/logs.h \
 /home/d3phys/Code/wolfram/include/stack.h
