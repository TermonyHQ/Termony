include ../utils/Makefrag

SOURCE_URL = https://github.com/ibireme/yyjson/archive/refs/tags/0.11.1.tar.gz
SOURCE_FILE = 0.11.1.tar.gz
SOURCE_DIR = yyjson-0.11.1
CMAKE_ARGS = -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo

$(eval $(call define_cmake_package))
