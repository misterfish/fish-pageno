fishutilx_topdir = fish-lib-util
fishutil_dir = $(fishutilx_topdir)/fish-util

cc = gcc -std=c99 

modules = math aosd glib freetype2 fishutil

# shared
math_inc	:= 
math_all 	:= -lm
aosd_inc 	:= $(shell pkg-config --cflags libaosd)
aosd_all 	:= $(shell pkg-config --cflags --libs libaosd) 
glib_inc 	:= $(shell pkg-config --cflags glib-2.0)
glib_all 	:= $(shell pkg-config --cflags --libs glib-2.0)
freetype2_inc 	:= $(shell pkg-config --cflags freetype2)
freetype2_all 	:= $(shell pkg-config --cflags --libs freetype2)

# static

# sets <module>_inc, <module>_obj, <module>_src_dep, <module>_ld, and <module>_all.
include $(fishutil_dir)/fishutil.mk
VPATH=$(fishutil_dir)

inc		= $(foreach i,$(modules),$(${i}_inc))
all		= $(foreach i,$(modules),$(${i}_all))

main		:= fish-pageno
src		:= $(main).c $(main).h draw.c arg.c global.h const.h config.h draw.h arg.h
obj		:= arg.o draw.o

all: $(obj) $(main)

# Doesn't depend on .h's XX
$(obj): %.o: %.c 
	$(cc) $(inc) -c $^ -o $@

$(fishutil_obj): $(fishutil_src_dep)
	make -C $(fishutilx_topdir)

fish-pageno: $(fishutil_obj) $(src) 
	$(cc) $(all) fish-pageno.c $(obj) -o fish-pageno

clean: 
	rm -f *.o
	rm -f $(main)

mrproper: clean
	cd $(fishutilx_topdir) && make mrproper

.PHONY: all clean mrproper

