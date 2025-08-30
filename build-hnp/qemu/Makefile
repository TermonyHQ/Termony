include ../utils/Makefrag

all: download/qemu-10
	rm -rf temp build
	mkdir -p temp build/bin
	cd download/qemu-10 && git worktree add -f $(shell pwd)/temp/qemu-10 HEAD
	cd temp/qemu-10 && git apply ../../0001-port-qemu-user-for-aarch64-and-x86_64-to-OHOS.patch
	cd temp/qemu-10 && git apply ../../0002-fix-pc-register-from-tci-to-guest.patch
	cd temp/qemu-10 && git apply ../../0003-update-gitignore.patch
	cd temp/qemu-10 && git apply ../../0004-enable-jit-on-OHOS.patch
	cd temp/qemu-10 && git apply ../../0005-fix-argv-for-geust-on-OHOS.patch
	cd temp/qemu-10 && git apply ../../0006-fix-jit-on-OHOS.patch
	cd temp/qemu-10 && git apply ../../0007-fix-for-x86-target.patch
	cd temp/qemu-10 && \
	PKG_CONFIG=$(shell which pkg-config) \
	PKG_CONFIG_PATH= \
	PKG_CONFIG_LIBDIR=$(shell pwd)/../sysroot/lib/pkgconfig:$(shell pwd)/../sysroot/share/pkgconfig \
	PKG_CONFIG_SYSROOT_DIR=$(shell pwd)/../sysroot \
	CFLAGS="-D_UAPI_LINUX_VIRTIO_VSOCK_H -D_UAPI_LINUX_VIRTIO_TYPES_H -D_UAPI_LINUX_VIRTIO_RING_H -D_UAPI_LINUX_VIRTIO_PMEM_H -D_UAPI_LINUX_VIRTIO_NET_H -D_UAPI_LINUX_VIRTIO_IOMMU_H -D_UAPI_LINUX_VIRTIO_FS_H -D_UAPI_LINUX_VIRTIO_CONSOLE_H -D_UAPI_LINUX_VIRTIO_CONFIG_H -D_LINUX_SYSINFO_H -D__user= -D__force= ${CFLAGS} -Wno-unused-command-line-argument -I$(shell pwd)/../sysroot/include/glib-2.0 -I$(shell pwd)/../sysroot/lib/glib-2.0/include -L$(shell pwd)/../sysroot/lib" \
	./configure --target-list=aarch64-linux-user,x86_64-linux-user,aarch64-softmmu,x86_64-softmmu --cross-prefix= --host-cc=cc --disable-kvm --disable-xen --disable-rust --disable-docs --disable-werror \
	 --disable-bsd-user --disable-guest-agent --disable-gcrypt --disable-debug-info --disable-debug-tcg --enable-attr --disable-brlapi --disable-linux-aio --disable-bzip2 --disable-cap-ng --disable-curl --disable-glusterfs --disable-gnutls --disable-nettle --disable-gtk --disable-rdma --disable-libiscsi --disable-vnc-jpeg --disable-kvm --disable-lzo --disable-curses --disable-libnfs --disable-numa --disable-opengl --disable-rbd --disable-vnc-sasl --disable-sdl --disable-seccomp --disable-smartcard --disable-snappy --disable-spice --disable-libusb --disable-usb-redir --disable-vde --disable-vhost-net --disable-virglrenderer --disable-virtfs --disable-vnc --disable-vte --disable-xen --disable-xen-pci-passthrough --enable-linux-user --disable-tools --disable-libudev --disable-passt
	cd temp/qemu-10 && make -j $(shell nproc)
	cp temp/qemu-10/build/qemu-aarch64 ./build/bin/
	cp temp/qemu-10/build/qemu-x86_64 ./build/bin/
	cp temp/qemu-10/build/qemu-system-aarch64 ./build/bin/
	cp temp/qemu-10/build/qemu-system-x86_64 ./build/bin/
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip ./build/bin/*
	mkdir -p ../sysroot
	cp -rv ./build/. ../sysroot | tee file.lst

download/qemu-10:
	mkdir -p download
	cd download && git clone -b v10.1.0-rc2 --depth=1 https://github.com/qemu/qemu qemu-10
