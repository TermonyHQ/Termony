include ../utils/Makefrag

SOURCE_URL = https://github.com/curl/curl/releases/download/curl-8_16_0/curl-8.16.0.tar.xz
SOURCE_FILE = curl-8.16.0.tar.xz
SOURCE_DIR = curl-8.16.0
CONFIG_ARGS = --prefix=$(PREFIX) --host $(OHOS_ARCH)-unknown-linux-musl --without-libssh --without-libssh2 --without-libpsl --with-libidn2 --enable-ares --without-nghttp2 --without-brotli --without-zstd --without-libgsasl --enable-shared --disable-static --disable-ldap --disable-ldaps --disable-rtsp --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smtp --disable-gopher --disable-manual --disable-unix-sockets --with-ca-path=/etc/ssl/certs --with-ca-bundle=/etc/ssl/certs/cacert.pem --disable-http-auth-gssnegotiate --disable-mqtt --disable-nls --disable-proxy --enable-optimize --with-ssl CFLAGS="-O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer" CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2"

$(eval $(call define_autotools_package))
