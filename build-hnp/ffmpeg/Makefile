include ../utils/Makefrag

export PKG_CONFIG_LIBDIR := $(shell pwd)/temp/pkgconfig:$(PKG_CONFIG_LIBDIR)

all: download/src.zip ohos_config.sh
	rm -rf temp build
	mkdir -p temp/build temp/pkgconfig build/bin

	cd temp && unzip ../download/src.zip
	cd temp && mv third_party_ffmpeg-OpenHarmony-5.1.0-Release src

	# fix script permission
	cd temp/src && cp ../../ohos_config.sh . && chmod +x ./ffbuild/*.sh

	# fix typo in src
	cd temp/src && sed -i 's/zsteam/zstream/g' libavcodec/mvha.c

	# fix missing vulkan_beta.h
	cd temp/src && touch vulkan_beta.h
	
	# fix pkgconfig prefix
	cd temp/pkgconfig && cp ../../../sysroot/lib/pkgconfig/SvtAv1Enc.pc . && sed -i 's@$(PREFIX)@$(shell pwd)/../sysroot@g' SvtAv1Enc.pc
	cd temp/pkgconfig && cp ../../../sysroot/lib/pkgconfig/opus.pc . && sed -i 's@$(PREFIX)@$(shell pwd)/../sysroot@g' opus.pc

	# debug output
	pkg-config --cflags --libs SvtAv1Enc opus

	# configure and build
	cd temp/src && ./ohos_config.sh $(shell pwd)/temp/src $(PREFIX) aarch64 $(OHOS_SDK_HOME)/native/llvm $(OHOS_SDK_HOME)/native/sysroot false
	cd temp/src && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build

	# strip all elf files
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build$(PREFIX)/bin/*
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build$(PREFIX)/lib/*.a

	mkdir -p ../sysroot/bin
	cp -rfv build$(PREFIX)/. ../sysroot | tee file.lst

download/src.zip:
	mkdir -p download
	cd download && wget -O src.zip https://gitee.com/openharmony/third_party_ffmpeg/repository/archive/OpenHarmony-5.1.0-Release.zip
