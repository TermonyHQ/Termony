include ../utils/Makefrag

all: download/qemu-5.0
	rm -rf temp build
	mkdir -p temp build/bin
	cd download/qemu-5.0 && git worktree add -f $(shell pwd)/temp/qemu-5.0 HEAD
	cd temp/qemu-5.0 && git apply ../../0001_port_to_ohos.patch
	cd temp/qemu-5.0 && git apply ../../0002_fix_madvise.patch
	cd temp/qemu-5.0 && git apply ../../0003_support_x86_64.patch
	cd temp/qemu-5.0 && git apply ../../0004_disable_mmap_PROT_EXEC.patch
	cd temp/qemu-5.0 && \
	PKG_CONFIG=$(shell which pkg-config) \
	PKG_CONFIG_PATH= \
	PKG_CONFIG_LIBDIR=$(shell pwd)/../sysroot/lib/pkgconfig:$(shell pwd)/../sysroot/share/pkgconfig \
	PKG_CONFIG_SYSROOT_DIR=$(shell pwd)/../sysroot \
	CFLAGS="-D_UAPI_LINUX_VIRTIO_VSOCK_H -D_UAPI_LINUX_VIRTIO_TYPES_H -D_UAPI_LINUX_VIRTIO_RING_H -D_UAPI_LINUX_VIRTIO_PMEM_H -D_UAPI_LINUX_VIRTIO_NET_H -D_UAPI_LINUX_VIRTIO_IOMMU_H -D_UAPI_LINUX_VIRTIO_FS_H -D_UAPI_LINUX_VIRTIO_CONSOLE_H -D_UAPI_LINUX_VIRTIO_CONFIG_H -D_LINUX_SYSINFO_H -D__user= -D__force= ${CFLAGS} -I$(shell pwd)/../sysroot/include/glib-2.0 -I$(shell pwd)/../sysroot/lib/glib-2.0/include -L$(shell pwd)/../sysroot/lib" \
	./configure --target-list=aarch64-linux-user --cross-prefix= --host-cc=cc --disable-kvm --disable-xen --disable-docs --disable-system --enable-tcg-interpreter --disable-werror
	cd temp/qemu-5.0 && make -j $(shell nproc)
	cp temp/qemu-5.0/aarch64-linux-user/qemu-aarch64 ./build/bin/
# 	cp temp/qemu-5.0/x86_64-linux-user/qemu-x86_64 ./build/bin/
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip ./build/bin/*
	mkdir -p ../sysroot
	cp -rv ./build/. ../sysroot | tee file.lst

download/qemu-5.0:
	mkdir -p download
	cd download && git clone -b stable-5.0 --depth=1 https://github.com/qemu/qemu qemu-5.0
