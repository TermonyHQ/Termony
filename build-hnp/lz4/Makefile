all: download/lz4-1.10.0.tar.gz
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && tar xvf ../download/lz4-1.10.0.tar.gz
	cd temp/lz4-1.10.0 && mkdir build-lz4
	cd temp/lz4-1.10.0/build-lz4 && cmake ../build/cmake/ -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DCMAKE_C_COMPILER=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang  -DCMAKE_BUILD_TYPE=RelWithDebInfo
	cd temp/lz4-1.10.0/build-lz4 && make -j $(shell nproc)
	cd temp/lz4-1.10.0/build-lz4 && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rv build/usr/local/. ../sysroot/

download/lz4-1.10.0.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/lz4/lz4/releases/download/v1.10.0/lz4-1.10.0.tar.gz