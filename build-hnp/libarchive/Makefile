include ../utils/Makefrag

SOURCE_URL = https://www.libarchive.org/downloads/libarchive-3.8.1.tar.gz
SOURCE_FILE = libarchive-3.8.1.tar.gz
SOURCE_DIR = libarchive-3.8.1
CMAKE_ARGS = \
	-DCMAKE_SYSTEM_NAME=Linux \
	-DCMAKE_SYSTEM_PROCESSOR=aarch64 \
	-DBUILD_SHARED_LIBS=ON \
	-DBUILD_STATIC_LIBS=OFF \
	-DCMAKE_C_COMPILER=$(OHOS_SDK_HOME)/native/llvm/bin/aarch64-unknown-linux-ohos-clang \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DENABLE_TEST=OFF \
	-DENABLE_PCREPOSIX=OFF \
	-DENABLE_PCRE2POSIX=OFF \
	-DENABLE_LIBB2=OFF \
	-DENABLE_LZMA=OFF \
	-DENABLE_EXPAT=OFF \
	-DENABLE_LIBXML2=OFF \
	-DOPENSSL_USE_STATIC_LIBS=OFF \
	-DCMAKE_INSTALL_PREFIX=$(PREFIX) \
	-DOPENSSL_INCLUDE_DIR=$(shell pwd)/../sysroot/include \
	-DOPENSSL_SSL_LIBRARY=$(shell pwd)./../sysroot/lib/libssl.so \
	-DOPENSSL_CRYPTO_LIBRARY=$(shell pwd)/../sysroot/lib/libcrypto.so \
	-DLZ4_INCLUDE_DIR=$(shell pwd)/../sysroot/include \
	-DLZ4_LIBRARY=$(shell pwd)/../sysroot/lib/liblz4.so \
	-DZLIB_INCLUDE_DIR=$(shell pwd)/../sysroot/include \
	-DZLIB_LIBRARY=$(shell pwd)/../sysroot/lib/libz.so \
	-DZSTD_INCLUDE_DIR=$(shell pwd)/../sysroot/include \
	-DZSTD_LIBRARY=$(shell pwd)/../sysroot/lib/libzstd.so

$(eval $(call define_cmake_package))
