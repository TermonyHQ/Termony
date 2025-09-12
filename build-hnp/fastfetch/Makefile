include ../utils/Makefrag

SOURCE_URL = https://github.com/fastfetch-cli/fastfetch/archive/refs/tags/2.52.0.tar.gz
SOURCE_FILE = 2.52.0.tar.gz
SOURCE_DIR = fastfetch-2.52.0
PATCH_SOURCE = cd temp/$(SOURCE_DIR) && /usr/bin/sed -i.bak 's|FF_HAVE_UTMPX=1|FF_HAVE_UTMPX=0|g' CMakeLists.txt
CMAKE_ARGS = -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=$(OHOS_ARCH) -DIS_MUSL=ON -DENABLE_PCIACCESS=ON -DENABLE_SQLITE3=OFF -DENABLE_VULKAN=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_SYSROOT=$(OHOS_SDK_HOME)/native/sysroot -DCMAKE_C_FLAGS="$(CFLAGS) -I$(shell pwd)/../sysroot/include/glib-2.0 -I$(shell pwd)/../sysroot/lib/glib-2.0/include"

$(eval $(call define_cmake_package))
