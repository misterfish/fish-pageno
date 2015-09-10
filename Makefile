cc 		= gcc 
CC 		= $(cc) 

CFLAGS		+= -std=c99 
LDFLAGS		?= 

main		= fish-pageno

# Will be looped over to build <module>_cflags, <module>_ldflags, etc.
modules_manual 	= fishutil fishutils
modules_pkgconfig	= libaosd glib-2.0 freetype2

# Subdirectories (will be make -C'ed).
submodules	= fish-lib-util

fishutil_dir		= fish-lib-util
fishutil_cflags		= $(shell PKG_CONFIG_PATH=$(fishutil_dir)/pkg-config/static pkg-config --cflags fish-util)
fishutil_ldflags	= $(shell PKG_CONFIG_PATH=$(fishutil_dir)/pkg-config/static pkg-config --static --libs fish-util)
fishutils_cflags	= $(shell PKG_CONFIG_PATH=$(fishutil_dir)/pkg-config/static pkg-config --cflags fish-utils)
fishutils_ldflags	= $(shell PKG_CONFIG_PATH=$(fishutil_dir)/pkg-config/static pkg-config --static --libs fish-utils)

ifneq ($(modules_pkgconfig),)
    # Check user has correct packages installed (and found by pkg-config).
    pkgs_ok := $(shell pkg-config --print-errors --exists $(modules_pkgconfig) && echo 1)
    ifneq ($(pkgs_ok),1)
        $(error Cannot find required package(s\). Please \
        check you have the above packages installed and try again.)
    endif
endif

CFLAGS		+= -Werror=implicit-function-declaration -W -Wall -Wextra -I./
CFLAGS		+= -Wno-missing-field-initializers # GCC bug with {0}
CFLAGS		+= $(foreach i,$(modules_manual),$(${i}_cflags))
CFLAGS		+= $(foreach i,$(modules_pkgconfig),$(shell pkg-config "$i" --cflags))

LDFLAGS		+= -Wl,--export-dynamic
LDFLAGS		+= -lm # log
LDFLAGS		+= $(foreach i,$(modules_manual),$(${i}_ldflags))
LDFLAGS		+= $(foreach i,$(modules_pkgconfig),$(shell pkg-config "$i" --libs))

src		= $(main).c draw.c arg.c

hdr		= $(main).h global.h const.h config.h draw.h arg.h

obj		= $(main).o draw.o arg.o

all: submodules $(main)

submodules: 
	@for i in $(submodules); do \
	    make -C "$$i"; \
	done

$(main): $(src) $(hdr) $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $(main)

# Note that all objs get rebuilt if any header changes. 

$(obj): %.o: %.c $(hdr)
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f *.o
	rm -f *.so
	rm -f $(main)
	cd $(fishutil_dir) && make clean

mrproper: clean
	cd $(fishutil_dir) && make mrproper

.PHONY: all clean mrproper
