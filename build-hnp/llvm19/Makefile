

all: download/llvm-project-19.1.7.src.tar.xz
	rm -rf temp build
	mkdir -p temp build #bolt;clang;clang-tools-extra;compiler-rt;cross-project-tests;libc;libclc;lld;lldb;mlir;openmp;polly;pstl;flang
	# -Dzstd_FOUND=ON -Dzstd_INCLUDE_DIR="$(shell pwd)/../sysroot/include" -Dzstd_LIBRARY=zstd -Dzstd_STATIC_LIBRARY=zstd  
	cd temp && tar xvf ../download/llvm-project-19.1.7.src.tar.xz
	cd temp/llvm-project-19.1.7.src && mkdir build
	cd temp/llvm-project-19.1.7.src/build && PKG_CONFIG=/usr/bin/false cmake ../ -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;lld" -DLLVM_ENABLE_RUNTIMES="" -DLIBCXXABI_USE_LLVM_UNWINDER=OFF -S ../llvm -DLLVM_HOST_TRIPLE=aarch64-unknown-linux-ohos -DLLVM_ENABLE_RTTI=ON -DLLVM_ENABLE_EH=ON -DLLVM_ENABLE_LIBXML2=OFF -DLLVM_ENABLE_Z3_SOLVER=OFF -DLLVM_ENABLE_LIBEDIT=OFF -DLLVM_ENABLE_LIBPFM=OFF -DLLVM_ENABLE_LIBCXX=ON -DLLVM_ENABLE_LIBCXXABI=ON -DLLVM_ENABLE_ZLIB=OFF -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_ENABLE_LZMA=OFF -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=ON -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_C_COMPILER=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang -DCMAKE_CXX_COMPILER=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang++ -DCMAKE_AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar -DCMAKE_EXE_LINKER_FLAGS="-L $(shell pwd)/../sysroot/lib" -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON  -DCMAKE_INSTALL_PREFIX=/data/app/base.org/base_1.0 -DLLDB_ENABLE_LZMA=OFF
	cd temp/llvm-project-19.1.7.src/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	$(shell find build/data/app/base.org/base_1.0/bin/ -type f -exec $(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip {} \;)
	$(shell find build/data/app/base.org/base_1.0/lib/ -type f -exec $(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip {} \;)
	mkdir -p ../sysroot
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot

download/llvm-project-19.1.7.src.tar.xz:
	mkdir -p download
	cd download && wget https://github.com/llvm/llvm-project/releases/download/llvmorg-19.1.7/llvm-project-19.1.7.src.tar.xz
