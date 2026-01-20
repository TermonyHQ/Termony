export CMAKE_CCACHE := 1

include ../utils/Makefrag

all: download/ncnn-20260113-full-source.zip
	rm -rf temp build
	mkdir -p temp/ncnn-20260113 build
	cd temp/ncnn-20260113 && unzip -o ../../download/ncnn-20260113-full-source.zip && mkdir build
	cd temp/ncnn-20260113/build && CFLAGS="-O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer -nodefaultlibs -fno-builtin -fno-stack-protector -nostdinc++ -mno-outline-atomics -lc" CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" LDFLAGS="-L$(shell pwd)/../sysroot/lib" cmake ../ -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_INSTALL_PREFIX=/usr/local/ -DCMAKE_SYSTEM_PROCESSOR=$(OHOS_ARCH) -DNCNN_SHARED_LIB=ON -DNCNN_SIMPLEOCV=ON -DNCNN_BUILD_EXAMPLES=ON -DNCNN_STDIO=ON -DNCNN_STRING=ON -DNCNN_BUILD_BENCHMARK=ON -DNCNN_OPENMP=OFF -DNCNN_VULKAN=ON -DCMAKE_C_COMPILER=$(OHOS_SDK_HOME)/native/llvm/bin/$(OHOS_ARCH)-unknown-linux-ohos-clang -DCMAKE_CXX_COMPILER=$(OHOS_SDK_HOME)/native/llvm/bin/$(OHOS_ARCH)-unknown-linux-ohos-clang++ -DCMAKE_BUILD_TYPE=RelWithDebInfo 
	cd temp/ncnn-20260113/build && make -j $(shell nproc) && make install -j $(shell nproc) DESTDIR=$(shell pwd)/build
	mkdir -p build/usr/local/share/benchncnn
	cp -rfv temp/ncnn-20260113/build/benchmark/benchncnn build/usr/local/share/benchncnn/
	mkdir -p ../sysroot
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/usr/local/lib/*.so
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/ncnn-20260113-full-source.zip:
	mkdir -p download
	cd download && $(WGET) https://github.com/Tencent/ncnn/releases/download/20260113/ncnn-20260113-full-source.zip
