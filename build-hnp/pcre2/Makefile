include ../utils/Makefrag

SOURCE_URL = https://github.com/PCRE2Project/pcre2/releases/download/pcre2-10.45/pcre2-10.45.tar.gz
SOURCE_FILE = pcre2-10.45.tar.gz
SOURCE_DIR = pcre2-10.45
CMAKE_ARGS = -DPCRE2_BUILD_PCRE2_32=ON -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo

$(eval $(call define_cmake_package))
