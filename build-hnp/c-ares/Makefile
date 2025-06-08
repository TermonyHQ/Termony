all: download/c-ares-1.34.5.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/c-ares-1.34.5.tar.gz
	cd temp/c-ares-1.34.5 && mkdir build
	cd temp/c-ares-1.34.5/build && PKG_CONFIG=/usr/bin/false CFLAGS="-O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer"  CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" LDFLAGS="-L$(shell pwd)/../sysroot/lib" cmake ../ -DCARES_STATIC=ON -DCARES_SHARED=OFF -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_C_COMPILER=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang -DCMAKE_BUILD_TYPE=RelWithDebInfo
	cd temp/c-ares-1.34.5/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot

download/c-ares-1.34.5.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/c-ares/c-ares/releases/download/v1.34.5/c-ares-1.34.5.tar.gz