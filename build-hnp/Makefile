PKGS=bash \
	binutils \
	busybox \
	coremark \
	lz4 \
	make \
	ncurses \
	openssl \
	strace \
	tar \
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
	libevent \
	tmux \
	fish \
	zstd \
	llvm19
STAMP=$(patsubst %,%/.stamp,$(PKGS))

all: copy

copy: base.hnp
	rm -f ../entry/hnp/arm64-v8a/*.hnp
	cp $^ ../entry/hnp/arm64-v8a

base.hnp: $(STAMP) utils/pbcopy utils/pbpaste
	# reduce size
	rm -rfv sysroot/share/man
	rm -rfv sysroot/share/doc
	rm -rfv sysroot/share/info
	cp utils/pbcopy sysroot/bin
	cp utils/pbpaste sysroot/bin
	$(TOOL_HOME)/sdk/default/openharmony/toolchains/hnpcli pack -i sysroot -n base -v 1.0

%/.stamp: %/Makefile
	make -C $(patsubst %/.stamp,%,$@)
	touch $@

clean:
	rm -f $(STAMP)
	rm -rf sysroot
