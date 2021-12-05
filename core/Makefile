#
# List data structure makefile
#
# Important! Dependencies are done automatically by 'make dep', which also
# removes any old dependencies. Do not modify it...
# 2021, d3phys
#

OBJS  = tree.o dump_tree.o optostr.o parse.o wolfram.o tex_tree.o

core.o: $(OBJS) subdirs
	$(LD) -r -o $@ $(OBJS)

include $(TOPDIR)/Rules.makefile

### Dependencies ###
dump_tree.o: dump_tree.cpp /home/d3phys/Code/wolfram/include/logs.h \
 /home/d3phys/Code/wolfram/include/wolfram/tree.h \
 /home/d3phys/Code/wolfram/include/wolfram/operators.h \
 /home/d3phys/Code/wolfram/include/wolfram/wolfram.h \
 /home/d3phys/Code/wolfram/include/stack.h
optostr.o: optostr.cpp \
 /home/d3phys/Code/wolfram/include/wolfram/operators.h
parse.o: parse.cpp /home/d3phys/Code/wolfram/include/iommap.h \
 /home/d3phys/Code/wolfram/include/hash.h \
 /home/d3phys/Code/wolfram/include/logs.h \
 /home/d3phys/Code/wolfram/include/wolfram/parse.h \
 /home/d3phys/Code/wolfram/include/wolfram/tree.h \
 /home/d3phys/Code/wolfram/include/wolfram/operators.h \
 /home/d3phys/Code/wolfram/include/wolfram/wolfram.h \
 /home/d3phys/Code/wolfram/include/stack.h
tex_tree.o: tex_tree.cpp /home/d3phys/Code/wolfram/include/logs.h \
 /home/d3phys/Code/wolfram/include/wolfram/tree.h \
 /home/d3phys/Code/wolfram/include/wolfram/operators.h \
 /home/d3phys/Code/wolfram/include/wolfram/wolfram.h \
 /home/d3phys/Code/wolfram/include/stack.h
tree.o: tree.cpp /home/d3phys/Code/wolfram/include/logs.h \
 /home/d3phys/Code/wolfram/include/wolfram/tree.h \
 /home/d3phys/Code/wolfram/include/wolfram/wolfram.h \
 /home/d3phys/Code/wolfram/include/stack.h
wolfram.o: wolfram.cpp /home/d3phys/Code/wolfram/include/logs.h \
 /home/d3phys/Code/wolfram/include/stack.h \
 /home/d3phys/Code/wolfram/include/wolfram/wolfram.h \
 /home/d3phys/Code/wolfram/include/wolfram/tree.h \
 /home/d3phys/Code/wolfram/include/wolfram/operators.h
