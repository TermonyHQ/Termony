include ../utils/Makefrag

all: download/glib
	rm -rf temp build
	mkdir -p temp build
	cd temp && cp -vr ../download/glib glib
	cd temp/glib && rm cross.txt || true
	cd temp/glib && echo "[binaries]" >> cross.txt
	cd temp/glib && echo "c = '${OHOS_SDK_HOME}/native/llvm/bin/aarch64-unknown-linux-ohos-clang'" >> cross.txt
	cd temp/glib && echo "cpp = '${OHOS_SDK_HOME}/native/llvm/bin/aarch64-unknown-linux-ohos-clang++'" >> cross.txt
	cd temp/glib && echo "ar = '${OHOS_SDK_HOME}/native/llvm/bin/llvm-ar'" >> cross.txt
	cd temp/glib && echo "ld = '${OHOS_SDK_HOME}/native/llvm/bin/ld.lld'" >> cross.txt
	cd temp/glib && echo "strip = '${OHOS_SDK_HOME}/native/llvm/bin/llvm-strip'" >> cross.txt
	cd temp/glib && echo "[host_machine]" >> cross.txt
	cd temp/glib && echo "system = 'linux'" >> cross.txt
	cd temp/glib && echo "cpu_family = 'aarch64'" >> cross.txt
	cd temp/glib && echo "cpu = 'aarch64'" >> cross.txt
	cd temp/glib && echo "endian = 'little'" >> cross.txt
	cd temp/glib && echo "[properties]" >> cross.txt
	cd temp/glib && echo "needs_exe_wrapper = true" >> cross.txt
	cd temp/glib && PKG_CONFIG=$(shell which pkg-config) PKG_CONFIG_LIBDIR=$(shell pwd)/../sysroot/lib/pkgconfig meson --cross-file cross.txt --prefix=/data/app/base.org/base_1.0 -Dselinux=false -Dinstalled_tests=false -Ddtrace=disabled -Dsystemtap=disabled -Dselinux=disabled -Dlibelf=disabled -Dlibmount=disabled -Ddefault_library=both -Dsysprof=disabled build
	cd temp/glib/build && meson compile
	mkdir -p ../sysroot
	cd temp/glib/build && DESTDIR=$(shell pwd)/build meson install
	cp -rfv ./build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/glib:
	mkdir -p download
	cd download && rm -rf glib && git clone -b 2.82.0 --recursive --depth=1 https://gitlab.gnome.org/GNOME/glib.git
