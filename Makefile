fishutilx_topdir = fish-lib-util
fishutil_dir = $(fishutilx_topdir)/fish-util

cc = gcc -std=c99 

modules = math aosd glib freetype2 fishutil

# shared
math_all 	:= -lm
aosd_all 	:= $(shell pkg-config --cflags --libs libaosd)
glib_all 	:= $(shell pkg-config --cflags --libs glib-2.0)
freetype2_all 	:= $(shell pkg-config --cflags --libs freetype2)

# static

# sets <module>_obj, <module>_src_dep, <module>_ld, and <module>_all.
include $(fishutil_dir)/fishutil.mk
VPATH=$(fishutil_dir)

all		= $(foreach i,$(modules),$(${i}_all))

main		:= fish-pageno
src		:= $(main).c $(main).h arg.c config.h const.h arg.h
obj		:= arg.o

all: $(obj) $(main)

$(obj): %.o: %.c
	$(cc) $(all) -c $^ -o $@

$(fishutil_obj): $(fishutil_src_dep)
	make -C $(fishutil_dir)

fish-pageno: $(fishutil_obj) $(src) 
	$(cc) $(all) fish-pageno.c $(obj) -o fish-pageno

clean: 
	rm -f *.o
	rm -f $(main)

mrproper: clean
	cd $(fishutilx_topdir) && make mrproper

.PHONY: all clean mrproper

