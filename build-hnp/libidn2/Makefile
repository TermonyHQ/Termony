all: download/libidn2-2.3.8.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/libidn2-2.3.8.tar.gz
	cd temp/libidn2-2.3.8 && PKG_CONFIG=/usr/bin/false ./configure --host aarch64-unknown-linux-musl --disable-shared --enable-static --disable-rpath CC=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-strip
	cd temp/libidn2-2.3.8 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot

download/libidn2-2.3.8.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/libidn/libidn2-2.3.8.tar.gz