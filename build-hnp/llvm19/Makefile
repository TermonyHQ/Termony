

all: download/llvm-project-19.1.7.src.tar.xz
	# rm -rf temp build
	# mkdir -p temp build
	# cd temp && tar xvf ../download/llvm-project-19.1.7.src.tar.xz
	# cd temp/llvm-project-19.1.7.src && mkdir -p build
	cd temp/llvm-project-19.1.7.src/build && PKG_CONFIG=/usr/bin/false cmake ../ -DLLVM_TARGETS_TO_BUILD=AArch64 -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;lld" -DLLVM_ENABLE_RUNTIMES="" -S ../llvm -DLLVM_HOST_TRIPLE=aarch64-linux-musl -DLLVM_ENABLE_RTTI=ON -DLLVM_ENABLE_EH=ON -DLLVM_ENABLE_LIBXML2=OFF -DLLVM_ENABLE_Z3_SOLVER=OFF -DLLVM_ENABLE_LIBEDIT=OFF -DLLVM_ENABLE_LIBPFM=OFF -DLLVM_ENABLE_LIBCXX=OFF -DLLVM_ENABLE_LIBCXXABI=OFF -DLLVM_ENABLE_ZLIB=OFF -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_ENABLE_LZMA=OFF -DLLVM_ENABLE_ZSTD=OFF -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_C_COMPILER=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang -DCMAKE_CXX_COMPILER=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang++ -DCMAKE_AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar -DCMAKE_RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib -DCMAKE_NM=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-nm -DCMAKE_EXE_LINKER_FLAGS="-L $(shell pwd)/../sysroot/lib"
	cd temp/llvm-project-19.1.7.src/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build

	find build/usr/local/bin/ -type f -exec $(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip {} \; || true
	find build/usr/local/lib/ -type f -exec $(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip {} \; || true
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/llvm-project-19.1.7.src.tar.xz:
	mkdir -p download
	cd download && wget https://github.com/llvm/llvm-project/releases/download/llvmorg-19.1.7/llvm-project-19.1.7.src.tar.xz
