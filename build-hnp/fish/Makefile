include ../utils/Makefrag

SOURCE_URL = https://github.com/fish-shell/fish-shell/releases/download/3.7.1/fish-3.7.1.tar.xz
SOURCE_FILE = fish-3.7.1.tar.xz
SOURCE_DIR = fish-3.7.1
# TCSANOW does not work, use TCSADRAIN
PATCH_SOURCE = sed -i.bak 's/TCSANOW,/TCSADRAIN,/' temp/$(SOURCE_DIR)/src/reader.cpp && \
	sed -i.bak 's/TCSANOW,/TCSADRAIN,/' temp/$(SOURCE_DIR)/src/fish_key_reader.cpp && \
	sed -i.bak 's/TCSANOW,/TCSADRAIN,/' temp/$(SOURCE_DIR)/src/builtins/fg.cpp && \
	cd temp/$(SOURCE_DIR) && cat ../../0001-fix-cmake.diff | patch -Np1 && \
	cat ../../0002-fix-getpwuid.diff | patch -Np1
# it makes hap installation fail
AFTER_INSTALL = rm -fv build$(PREFIX)/share/fish/completions/..fish

CMAKE_ARGS = -DSYS_PCRE2_LIB="$(shell pwd)/../sysroot/lib/libpcre2-32.so" -DSYS_PCRE2_INCLUDE_DIR="$(shell pwd)/../sysroot/include" -DCMAKE_DISABLE_FIND_PACKAGE_Curses=ON -DCURSES_HAVE_CURSES_H=ON -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=$(OHOS_ARCH) -DCMAKE_C_COMPILER=$(OHOS_SDK_HOME)/native/llvm/bin/$(OHOS_ARCH)-unknown-linux-ohos-clang -DCMAKE_CXX_COMPILER=$(OHOS_SDK_HOME)/native/llvm/bin/$(OHOS_ARCH)-unknown-linux-ohos-clang++ -DCMAKE_AR=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ar -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCURSES_FOUND=ON -DCURSES_INCLUDE_DIRS="$(shell pwd)/../sysroot/include;$(shell pwd)/../sysroot/include/ncursesw" -DCURSES_LIBRARY=ncursesw -DCURSES_TINFO="$(shell pwd)/../sysroot/lib/libtinfow.so" -DCMAKE_EXE_LINKER_FLAGS="-L $(shell pwd)/../sysroot/lib" -DCMAKE_CXX_FLAGS="-DTPUTS_USES_INT_ARG=1" -DFISH_USE_SYSTEM_PCRE2=ON -DWITH_GETTEXT=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DBUILD_DOCS=OFF

$(eval $(call define_cmake_package))
