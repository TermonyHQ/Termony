include ../utils/Makefrag

all: download/vkpeak
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && cp -vr ../download/vkpeak vkpeak-20260112
	cd temp/vkpeak-20260112 && mkdir -p build
	cd temp/vkpeak-20260112/build && cmake ../ -DCMAKE_SYSTEM_PROCESSOR=$(OHOS_ARCH) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_C_COMPILER=$(OHOS_SDK_HOME)/native/llvm/bin/$(OHOS_ARCH)-unknown-linux-ohos-clang -DCMAKE_CXX_COMPILER=$(OHOS_SDK_HOME)/native/llvm/bin/$(OHOS_ARCH)-unknown-linux-ohos-clang++ -DCPPFLAGS="-I$(shell pwd)/../../sysroot/include -D_FORTIFY_SOURCE=2" -DLDFLAGS="-L$(shell pwd)/../../sysroot/lib" -DCMAKE_BUILD_TYPE=RelWithDebInfo
	cd temp/vkpeak-20260112/build && make -j $(shell nproc)
	cd temp/vkpeak-20260112/build && cp -vf vkpeak $(shell pwd)/build/bin
	mkdir -p ../sysroot
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/bin/*
	cp -rv ./build/. ../sysroot | tee file.lst

download/vkpeak:
	mkdir -p download
	cd download && rm -rf vkpeak && git clone -b 20260112 --depth=1 --recursive https://github.com/nihui/vkpeak.git
