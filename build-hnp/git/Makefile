all: download/v2.49.0.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/v2.49.0.tar.gz
	cd temp/git-2.49.0 && mkdir build
	cd temp/git-2.49.0 && make -j $(shell nproc) INSTALL_SYMLINKS=1 uname_S=Linux NO_REGEX=NeedsStartEnd CFLAGS="-DNO_PTHREADS -DNO_GETTEXT -Wno-int-conversion -I$(shell pwd)/../sysroot/include" LDFLAGS="-L $(shell pwd)/../sysroot/lib -lcurl -liconv" V=1 prefix=/data/app/base.org/base_1.0 CC=$(OHOS_SDK_HOME)/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ar RANLIB=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ranlib STRIP=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip LD=$(OHOS_SDK_HOME)/native/llvm/bin/aarch64-unknown-linux-ohos-clang all install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/bin/git
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/bin/git-shell
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/bin/scalar
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/libexec/git-core/git-daemon
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/libexec/git-core/git-http-backend
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/libexec/git-core/git-http-fetch
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/libexec/git-core/git-http-push
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/libexec/git-core/git-imap-send
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/libexec/git-core/git-remote-http
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/libexec/git-core/git-sh-i18n--envsubst
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/v2.49.0.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/git/git/archive/refs/tags/v2.49.0.tar.gz
