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

src		:= fish-pageno.c arg.c fish-pageno.h config.h const.h arg.h
obj		:= arg.o

all: $(obj) fish-pageno

arg.o: %.o: %.c
	$(cc) $(all) -c $^ -o $@

$(fishutil_obj): $(fishutil_src_dep)
	make -C $(fish_util_dir)

fish-pageno: $(fishutil_obj) $(src) 
	$(cc) $(all) fish-pageno.c $(obj) -o fish-pageno

clean: 
	rm -f *.o
	rm -f fish-pageno

.PHONY: all clean

