###############################################################################
#########################  lib-of LIBRARY   ###################################
###############################################################################
OBJ :=  sys_spinlock.o sys_dma.o mem_mng.o
INC := $(TOPINC)
DEF := $(TOPDEF)

lsys.o: $(OBJ)
	@(echo "(LD)  $@ <= $^")
	@($(LD) -r -o $@ -Map $*.map $^)

include $(TOPDIR)/Rules.make
