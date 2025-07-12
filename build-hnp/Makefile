PKGS= \
	pcre2 \
	glib \
	gettext \
	ncurses \
	libevent \
	expat \
	libiconv \
	libffi \
	lz4 \
	xz \
	tar \
	talloc \
	zstd \
	zlib \
	gettext \
	libusb \
	binutils \
	busybox \
	coremark \
	bash \
	make \
	openssl \
	proot \
	strace \
	tree \
	vim \
	xxhash \
	libunistring \
	libidn2 \
	c-ares \
	curl \
	yyjson \
	fastfetch \
	sl \
	openssh \
	ncnn \
	readline \
	htop \
	vkpeak \
	kbd \
	stream \
	tmux \
	fish \
	elf-loader \
	gmp \
	mpfr \
	mpc \
	gcc \
	libarchive \
	git \
	aria2 \
	gdb \
	python \
	hdc \
	qemu \
	qemu-vroot
STAMP=$(patsubst %,%/.stamp,$(PKGS))

all: copy

copy: base.hnp
	rm -f ../entry/hnp/arm64-v8a/*.hnp
	cp $^ ../entry/hnp/arm64-v8a
	cp $^ ../entry/hnp/arm64-v8a/base-public.hnp

base.hnp: $(STAMP) utils/pbcopy utils/pbpaste utils/empty.a Makefile
	# reduce size
	rm -rfv sysroot/share/man
	rm -rfv sysroot/share/doc
	rm -rfv sysroot/share/info
	# add pasteboard utilities
	cp utils/pbcopy sysroot/bin
	cp utils/pbpaste sysroot/bin
	# let gcc work out of box
	mkdir -p sysroot/aarch64-unknown-linux-musl/lib
	cp $(OHOS_SDK_HOME)/native/sysroot/usr/lib/aarch64-linux-ohos/crt1.o sysroot/aarch64-unknown-linux-musl/lib/crt1.o
	cp $(OHOS_SDK_HOME)/native/sysroot/usr/lib/aarch64-linux-ohos/crti.o sysroot/aarch64-unknown-linux-musl/lib/crti.o
	cp $(OHOS_SDK_HOME)/native/sysroot/usr/lib/aarch64-linux-ohos/crtn.o sysroot/aarch64-unknown-linux-musl/lib/crtn.o
	cp $(OHOS_SDK_HOME)/native/llvm/lib/clang/15.0.4/lib/aarch64-linux-ohos/clang_rt.crtbegin.o sysroot/aarch64-unknown-linux-musl/lib/crtbegin.o
	cp $(OHOS_SDK_HOME)/native/llvm/lib/clang/15.0.4/lib/aarch64-linux-ohos/clang_rt.crtend.o sysroot/aarch64-unknown-linux-musl/lib/crtend.o
	cp $(OHOS_SDK_HOME)/native/llvm/lib/clang/15.0.4/lib/aarch64-linux-ohos/libclang_rt.builtins.a sysroot/aarch64-unknown-linux-musl/lib/libgcc.a
	cp $(OHOS_SDK_HOME)/native/llvm/lib/clang/15.0.4/lib/aarch64-linux-ohos/libclang_rt.builtins.a sysroot/aarch64-unknown-linux-musl/lib/libgcc_s.a
	cp utils/empty.a sysroot/aarch64-unknown-linux-musl/lib/libgcc_eh.a
	# create hnp manually
	# $(OHOS_SDK_HOME)/toolchains/hnpcli pack -i sysroot -n base -v 1.0
	cp hnp.json sysroot
	rm -f base.hnp
	zip -r base.hnp sysroot

%/.stamp: %/Makefile
	make -C $(patsubst %/.stamp,%,$@)
	touch $@

rebuild-%:
	make -C $(patsubst rebuild-%,%,$@)
	touch $(patsubst rebuild-%,%/.stamp,$@)

clean:
	rm -f $(STAMP)
	rm -rf sysroot
