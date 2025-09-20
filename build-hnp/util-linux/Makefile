include ../utils/Makefrag

SOURCE_URL = https://www.kernel.org/pub/linux/utils/util-linux/v2.41/util-linux-2.41.1.tar.gz
SOURCE_FILE = util-linux-2.41.1.tar.gz
SOURCE_DIR = util-linux-2.41.1
CONFIG_ARGS = --prefix=$(PREFIX) --host aarch64-unknown-linux-musl --disable-all-programs --enable-libsmartcols --enable-lscpu
$(eval $(call define_autotools_package))
