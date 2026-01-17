include ../utils/Makefrag

SOURCE_URL = $(GNU_MIRROR)/gnu/nano/nano-8.7.tar.xz
SOURCE_FILE = nano-8.7.tar.xz
SOURCE_DIR = nano-8.7
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --host $(OHOS_ARCH)-unknown-linux-musl

# Locate ncurses
SYSROOT_REL = ../../../../../sysroot
export CFLAGS += -I$(SYSROOT_REL)/include/ncursesw -I$(SYSROOT_REL)/include
export CPPFLAGS += -I$(SYSROOT_REL)/include/ncursesw -I$(SYSROOT_REL)/include

$(eval $(call define_autotools_package))
