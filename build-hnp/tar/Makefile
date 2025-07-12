include ../utils/Makefrag

SOURCE_URL = $(GNU_MIRROR)/gnu/tar/tar-1.35.tar.xz
SOURCE_FILE = tar-1.35.tar.xz
SOURCE_DIR = tar-1.35
CONFIG_ARGS = --prefix=$(PREFIX) --host aarch64-unknown-linux-musl --disable-nls LDFLAGS="-L$(shell pwd)/../sysroot/lib -liconv"

$(eval $(call define_autotools_package))
