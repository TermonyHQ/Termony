all: download/zstd-1.5.7.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/zstd-1.5.7.tar.gz
	cd temp/zstd-1.5.7 && cat ../../0001-qsort.diff | patch -Np1
	cd temp/zstd-1.5.7 && mkdir build-zstd
	cd temp/zstd-1.5.7/build-zstd && PKG_CONFIG=/usr/bin/false cmake ../build/cmake -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_C_COMPILER=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang -DCMAKE_CXX_COMPILER=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang++ -DCMAKE_AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar -DCMAKE_BUILD_TYPE=RelWithDebInfo -DZSTD_BUILD_STATIC=ON -DZSTD_BUILD_SHARED=ON
	cd temp/zstd-1.5.7/build-zstd && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	rm -v build/usr/local/lib/*.a
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/zstd-1.5.7.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/facebook/zstd/releases/download/v1.5.7/zstd-1.5.7.tar.gz
