include ../utils/Makefrag

SOURCE_URL = https://github.com/zeux/volk/archive/refs/tags/1.4.304.tar.gz
SOURCE_FILE = 1.4.304.tar.gz
SOURCE_DIR = volk-1.4.304
CMAKE_ARGS = -DCMAKE_INSTALL_PREFIX=$(PREFIX) \
	-DCMAKE_PREFIX_PATH=$(shell pwd)/../sysroot \
	-DCMAKE_SYSTEM_NAME=Linux \
	-DCMAKE_SYSTEM_PROCESSOR=$(OHOS_ARCH) \
	-DBUILD_SHARED_LIBS=ON \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
	-DVOLK_INSTALL=ON

$(eval $(call define_cmake_package))
