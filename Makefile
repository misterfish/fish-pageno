fish_util_dir = fish-lib-util/fish-util

cc = gcc -std=c99 

modules = math aosd glib freetype2 fishutil

# shared
math_all 	:= -lm
aosd_all 	:= $(shell pkg-config --cflags --libs libaosd)
glib_all 	:= $(shell pkg-config --cflags --libs glib-2.0)
freetype2_all 	:= $(shell pkg-config --cflags --libs freetype2)

# static
fishutil_obj 	:= $(fish_util_dir)/fish-util.o
fishutil_dep 	:= $(shell cat $(fish_util_dir)/.obj/fish-util.o/deps)
fishutil_ld 	:= $(shell cat $(fish_util_dir)/.obj/fish-util.o/ld)
fishutil_all 	:= -I$(fish_util_dir) $(fishutil_obj)

all		= $(foreach i,$(modules),$(${i}_all))

all: fish-pageno

vpath %fish-util.c $(fish_util_dir)
vpath %fish-util.h $(fish_util_dir)

$(fishutil_obj): $(fishutil_dep)
	make -C $(fish_util_dir)

fish-pageno: $(fishutil_obj) fish-pageno.c config.h
	$(cc) $(all) fish-pageno.c -o fish-pageno

clean: 
	rm -f *.o
	rm -f fish-pageno

.PHONY: all clean fish-lib-util/fish-util

