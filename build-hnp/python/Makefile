# file: build-hnp/python/Makefile

all: \
	download/Python-3.13.5.tar.xz \
	prepare_env_native_python3 \
	cross_build_native_python3


download/Python-3.13.5.tar.xz:
	mkdir -p download
	cd download && wget https://www.python.org/ftp/python/3.13.5/Python-3.13.5.tar.xz

prepare_env_native_python3:
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/Python-3.13.5.tar.xz
	cd temp/Python-3.13.5 && mkdir build-native
	cd temp/Python-3.13.5/build-native && CC="cc" CXX="" LD="ld" AR="ar" CXXFLAGS="" ../configure
	cd temp/Python-3.13.5/build-native && CC="cc" CXX="" LD="ld" AR="ar" CXXFLAGS="" make -j $(shell nproc)
	cd temp/Python-3.13.5/build-native && CC="cc" CXX="" LD="ld" AR="ar" CXXFLAGS="" make install DESTDIR=$(shell pwd)/temp/native


cross_build_native_python3:
	$(eval include ../utils/Makefrag)
	
	# ensure the sysroot is built, and the libncurses headers are available for ncursesw/term.h
	$(eval export CFLAGS=$(CFLAGS) -I$(shell pwd)/../sysroot/include/ncursesw)

	cd temp/Python-3.13.5 && mkdir build
	
	# HACK: set ANDROID_API_LEVEL=1 to link the shared libraries with libpython
	cd temp/Python-3.13.5/build && ../configure \
		ANDROID_API_LEVEL=1 \
		--prefix=$(PREFIX) \
		--disable-test-modules \
		--enable-shared \
		--host aarch64-unknown-linux-musl \
		--build=aarch64 \
		--enable-ipv6 \
		--with-ensurepip=install \
		--with-build-python=$(shell pwd)/temp/native/usr/local/bin/python3 \
		ac_cv_file__dev_ptmx=no \
		ac_cv_file__dev_ptc=no
	
	# missing gettext in libintl.h
	cd temp/Python-3.13.5/build && sed -i.bak "s/#define HAVE_LIBINTL_H 1//" pyconfig.h
	cd temp/Python-3.13.5/build && make -j $(shell nproc)
	cd temp/Python-3.13.5/build && make install DESTDIR=$(shell pwd)/build
	
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/bin/python3.13
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/lib/*.so*
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/lib/python3.13/config-3.13/libpython3.13.a
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/lib/python3.13/lib-dynload/*.so
	
	mkdir -p ../sysroot
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst