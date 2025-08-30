include ../utils/Makefrag

export PKG_CONFIG_LIBDIR := $(shell pwd)/temp/pkgconfig:$(PKG_CONFIG_LIBDIR)

all: download/ffmpeg-8.0.tar.xz ohos_config.sh
	rm -rf temp build
	mkdir -p temp/build temp/pkgconfig build/bin

	cd temp && tar vxf ../download/ffmpeg-8.0.tar.xz
	cd temp && mv ffmpeg-8.0 src

	# fix script permission
	cd temp/src && cp ../../ohos_config.sh . && chmod +x ./ffbuild/*.sh

	# fix pkgconfig prefix
	cd temp/pkgconfig && cp ../../../sysroot/lib/pkgconfig/SvtAv1Enc.pc . && sed -i 's@$(PREFIX)@$(shell pwd)/../sysroot@g' SvtAv1Enc.pc
	cd temp/pkgconfig && cp ../../../sysroot/lib/pkgconfig/opus.pc . && sed -i 's@$(PREFIX)@$(shell pwd)/../sysroot@g' opus.pc

	# debug output
	pkg-config --cflags --libs SvtAv1Enc opus

	# configure and build
	cd temp/src && ./ohos_config.sh $(shell pwd)/temp/src $(PREFIX) $(OHOS_ARCH) $(OHOS_SDK_HOME)/native/llvm $(OHOS_SDK_HOME)/native/sysroot false
	cd temp/src && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build

	# strip all elf files
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build$(PREFIX)/bin/*
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build$(PREFIX)/lib/*.a

	mkdir -p ../sysroot/bin
	cp -rfv build$(PREFIX)/. ../sysroot | tee file.lst

download/ffmpeg-8.0.tar.xz:
	mkdir -p download
	cd download && wget -O ffmpeg-8.0.tar.xz https://ffmpeg.org/releases/ffmpeg-8.0.tar.xz
