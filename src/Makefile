
SRC_OBJS	= virtual.o format.o period.o player.o read_event.o \
		  win32.o md5.o lfo.o scan.o control.o \
		  med_extras.o filter.o effects.o mixer.o mix_all.o \
		  load_helpers.o load.o hmn_extras.o extras.o \
		  mix_paula.o buffer.o mem.o

SRC_DFILES	= Makefile $(SRC_OBJS:.o=.c) common.h effects.h \
		  format.h lfo.h list.h mixer.h period.h player.h virtual.h \
		  md5.h precomp_lut.h med_extras.h \
		  hmn_extras.h extras.h paula.h \
		  precomp_blep.h buffer.h mem.h

SRC_PATH	= src

OBJS += $(addprefix $(SRC_PATH)/,$(SRC_OBJS))

default-src::
	$(MAKE) -C ..

dist-src::
	mkdir -p $(DIST)/$(SRC_PATH)
	cp -RPp $(addprefix $(SRC_PATH)/,$(SRC_DFILES)) $(DIST)/$(SRC_PATH)

