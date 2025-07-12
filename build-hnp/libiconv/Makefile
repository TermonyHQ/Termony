include ../utils/Makefrag

SOURCE_URL = $(GNU_MIRROR)/gnu/libiconv/libiconv-1.18.tar.gz
SOURCE_FILE = libiconv-1.18.tar.gz
SOURCE_DIR = libiconv-1.18
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --host aarch64-unknown-linux-musl

$(eval $(call define_autotools_package))
