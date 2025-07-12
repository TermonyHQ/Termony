include ../utils/Makefrag

all: download/qemu-5.0
	rm -rf temp build
	mkdir -p temp build/bin
	cd download/qemu-5.0 && git worktree add -f $(shell pwd)/temp/qemu-5.0 HEAD
	cd temp/qemu-5.0 && git apply ../../0001-Initial-porting-to-OHOS.patch
	cd temp/qemu-5.0 && git apply ../../0002-Handle-target-madvise.patch
	cd temp/qemu-5.0 && git apply ../../0003-Drop-missing-syscall-for-x86_64-on-OHOS.patch
	cd temp/qemu-5.0 && git apply ../../0004-Drop-PROT_EXEC-for-mmap-on-OHOS.patch
	cd temp/qemu-5.0 && git apply ../../0005-Simulate-proot-for-execve-syscall-on-OHOS.patch
	cd temp/qemu-5.0 && git apply ../../0006-Disable-setuid-setgid-on-OHOS.patch
	cd temp/qemu-5.0 && git apply ../../0007-Find-qemu-abs-path-from-proc-self-exe-instead.patch
	cd temp/qemu-5.0 && git apply ../../0009-Refactor-vroot-path-resolving.patch
	cd temp/qemu-5.0 && git apply ../../0010-Support-specify-relative-virtual-root-directory.patch
	cd temp/qemu-5.0 && git apply ../../0011-Disable-mount-proc-filesystem.patch
	cd temp/qemu-5.0 && git apply ../../0012-Disable-umount2-proc-filesystem.patch
	cd temp/qemu-5.0 && git apply ../../0013-Complete-path-relocation.patch
	cd temp/qemu-5.0 && \
	PKG_CONFIG=$(shell which pkg-config) \
	PKG_CONFIG_PATH= \
	PKG_CONFIG_LIBDIR=$(shell pwd)/../sysroot/lib/pkgconfig:$(shell pwd)/../sysroot/share/pkgconfig \
	PKG_CONFIG_SYSROOT_DIR=$(shell pwd)/../sysroot \
	CFLAGS="-D_UAPI_LINUX_VIRTIO_VSOCK_H -D_UAPI_LINUX_VIRTIO_TYPES_H -D_UAPI_LINUX_VIRTIO_RING_H -D_UAPI_LINUX_VIRTIO_PMEM_H -D_UAPI_LINUX_VIRTIO_NET_H -D_UAPI_LINUX_VIRTIO_IOMMU_H -D_UAPI_LINUX_VIRTIO_FS_H -D_UAPI_LINUX_VIRTIO_CONSOLE_H -D_UAPI_LINUX_VIRTIO_CONFIG_H -D_LINUX_SYSINFO_H -D__user= -D__force= ${CFLAGS} -I$(shell pwd)/../sysroot/include/glib-2.0 -I$(shell pwd)/../sysroot/lib/glib-2.0/include -L$(shell pwd)/../sysroot/lib" \
	./configure --target-list=aarch64-linux-user --cross-prefix= --host-cc=cc --disable-kvm --disable-xen --disable-docs --disable-system --enable-tcg-interpreter --disable-werror \
	 --disable-bsd-user --disable-guest-agent --disable-strip --disable-gcrypt --disable-debug-info --disable-debug-tcg --enable-attr --disable-brlapi --disable-linux-aio --disable-bzip2 --disable-cap-ng --disable-curl --disable-fdt --disable-glusterfs --disable-gnutls --disable-nettle --disable-gtk --disable-rdma --disable-libiscsi --disable-vnc-jpeg --disable-kvm --disable-lzo --disable-curses --disable-libnfs --disable-numa --disable-opengl --disable-vnc-png --disable-rbd --disable-vnc-sasl --disable-sdl --disable-seccomp --disable-smartcard --disable-snappy --disable-spice --disable-libusb --disable-usb-redir --disable-vde --disable-vhost-net --disable-virglrenderer --disable-virtfs --disable-vnc --disable-vte --disable-xen --disable-xen-pci-passthrough --disable-xfsctl --enable-linux-user --disable-blobs --disable-tools
	cd temp/qemu-5.0 && make -j $(shell nproc)
	cp temp/qemu-5.0/aarch64-linux-user/qemu-aarch64 ./build/bin/
# 	cp temp/qemu-5.0/x86_64-linux-user/qemu-x86_64 ./build/bin/
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip ./build/bin/*
	mkdir -p ../sysroot/bin
	cp ./build/bin/qemu-aarch64 ../sysroot/bin/qemu-vroot-aarch64 | tee file.lst
# 	cp ./build/bin/qemu-x86_64 ../sysroot/bin/qemu-vroot-x86_64 | tee file.lst

download/qemu-5.0:
	mkdir -p download
	cd download && git clone -b stable-5.0 --depth=1 https://github.com/qemu/qemu qemu-5.0
