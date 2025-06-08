all: download/curl-8.14.1.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/curl-8.14.1.tar.xz
	cd temp/curl-8.14.1 && PKG_CONFIG=/usr/bin/false ./configure --host aarch64-unknown-linux-musl --without-libssh --without-libssh2 --without-libpsl --with-libidn2 --enable-ares --without-nghttp2 --without-brotli --without-zstd --without-libgsasl --disable-shared --enable-static --disable-ldap --disable-ldaps --disable-rtsp --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smtp --disable-gopher --disable-manual --disable-unix-sockets --with-ca-path=/etc/ssl/certs --with-ca-bundle=/etc/ssl/certs/cacert.pem --disable-http-auth-gssnegotiate --disable-mqtt --disable-nls --disable-proxy --enable-optimize --with-ssl CC=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang CFLAGS="-O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer"  CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" LDFLAGS="-L$(shell pwd)/../sysroot/lib"
	cd temp/curl-8.14.1 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot

download/curl-8.14.1.tar.xz:
	mkdir -p download
	cd download && wget https://github.com/curl/curl/releases/download/curl-8_14_1/curl-8.14.1.tar.xz