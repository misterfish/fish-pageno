fish_util_dir = fish-lib-util/fish-util

cc = gcc -std=c99 

modules = math aosd glib freetype2 fishutil

# shared
math_all 	:= -lm
aosd_all 	:= $(shell pkg-config --cflags --libs libaosd)
glib_all 	:= $(shell pkg-config --cflags --libs glib-2.0)
freetype2_all 	:= $(shell pkg-config --cflags --libs freetype2)

# static

# sets <module>_obj, <module>_src_dep, <module>_ld, and <module>_all.
include $(fish_util_dir)/fish-util.mk
VPATH=$(fish_util_dir)


all		= $(foreach i,$(modules),$(${i}_all))

all: fish-pageno

$(fishutil_obj): $(fishutil_src_dep)
	make -C $(fish_util_dir)

fish-pageno: $(fishutil_obj) fish-pageno.c config.h
	$(cc) $(all) fish-pageno.c -o fish-pageno

clean: 
	rm -f *.o
	rm -f fish-pageno

.PHONY: all clean

