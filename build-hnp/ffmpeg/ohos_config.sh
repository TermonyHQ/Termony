# FFmpeg build configure

#!/bin/bash

set -ex
FFMPEG_PATH=$1
FFMPEG_OUT_PATH=$2
FFMPEG_PLAT=$3
LLVM_PATH=$4
SYSROOT_PATH=$5
USE_CLANG_COVERAGE=$6
FFMPEG_ENABLE_DEMUXERS=$7
FFMPEG_ENABLE_PARSERS=$8
FFMPEG_ENABLE_DECODERS=$9

FF_CONFIG_OPTIONS="
    --prefix=${FFMPEG_OUT_PATH}
    --enable-vulkan
    --enable-libshaderc
    --enable-libx264
    --enable-libx265
    --enable-libvpx
    --enable-libsvtav1
    --enable-libmp3lame
    --enable-libfdk-aac
    --enable-libopus
    --enable-libvorbis
    --enable-gpl
    --enable-nonfree
"

if [ ${FFMPEG_PLAT} = "${OHOS_ARCH}" ]; then

FF_CONFIG_OPTIONS="$FF_CONFIG_OPTIONS
    --arch=${OHOS_ARCH}
    --target-os=linux
    --enable-cross-compile
    --cc=${LLVM_PATH}/bin/clang
    --ld=${LLVM_PATH}/bin/clang
    --strip=${LLVM_PATH}/bin/llvm-strip
"

if [ -n "${FFMPEG_ENABLE_DEMUXERS}" ]; then
FF_CONFIG_OPTIONS+="
    --enable-demuxer=${FFMPEG_ENABLE_DEMUXERS}"
fi

if [ -n "${FFMPEG_ENABLE_PARSERS}" ]; then
FF_CONFIG_OPTIONS+="
    --enable-parser=${FFMPEG_ENABLE_PARSERS}"
fi

if [ -n "${FFMPEG_ENABLE_DECODERS}" ]; then
FF_CONFIG_OPTIONS+="
    --enable-decoder=${FFMPEG_ENABLE_DECODERS}"
fi

EXTRA_CFLAGS="
    --target=${OHOS_ARCH}-linux-ohos
    --sysroot=${SYSROOT_PATH}
"
EXTRA_LDFLAGS="
    --target=${OHOS_ARCH}-linux-ohos
    --sysroot=${SYSROOT_PATH}
"

if [ ${USE_CLANG_COVERAGE} = "true" ]; then
    EXTRA_CFLAGS="
        --target=${OHOS_ARCH}-linux-ohos
        --sysroot=${SYSROOT_PATH}
        --coverage
        -mllvm
        -limited-coverage-experimental=true
    "
    EXTRA_LDFLAGS="
        --target=${OHOS_ARCH}-linux-ohos
        --sysroot=${SYSROOT_PATH}
        --coverage
        -fno-use-cxa-atexit
    "
fi

FF_CONFIG_OPTIONS=`echo $FF_CONFIG_OPTIONS`

${FFMPEG_PATH}/configure ${FF_CONFIG_OPTIONS} --extra-cflags="${EXTRA_CFLAGS}" --extra-ldflags="${EXTRA_LDFLAGS}"

else

oldPath=`pwd`

FFMPEG_PATH=${oldPath}/${FFMPEG_PATH}
FFMPEG_OUT_PATH=${oldPath}/${FFMPEG_OUT_PATH}
currentPath=${oldPath}/${FFMPEG_OUT_PATH}tmp
mkdir -p ${currentPath}
cd ${currentPath}

if [ -n "${FFMPEG_ENABLE_DEMUXERS}" ]; then
FF_CONFIG_OPTIONS+="
    --enable-demuxer=${FFMPEG_ENABLE_DEMUXERS}"
fi

if [ -n "${FFMPEG_ENABLE_PARSERS}" ]; then
FF_CONFIG_OPTIONS+="
    --enable-parser=${FFMPEG_ENABLE_PARSERS}"
fi

if [ -n "${FFMPEG_ENABLE_DECODERS}" ]; then
FF_CONFIG_OPTIONS+="
    --enable-decoder=${FFMPEG_ENABLE_DECODERS}"
fi

FF_CONFIG_OPTIONS=`echo $FF_CONFIG_OPTIONS`

${FFMPEG_PATH}/configure ${FF_CONFIG_OPTIONS}

fi

sed -i 's/HAVE_SYSCTL 1/HAVE_SYSCTL 0/g' config.h
sed -i 's/HAVE_SYSCTL=yes/!HAVE_SYSCTL=yes/g' ./ffbuild/config.mak
sed -i 's/HAVE_GLOB 1/HAVE_GLOB 0/g' config.h
sed -i 's/HAVE_GLOB=yes/!HAVE_GLOB=yes/g' config.h
sed -i 's/HAVE_GMTIME_R 1/HAVE_GMTIME_R 0/g' config.h
sed -i 's/HAVE_LOCALTIME_R 1/HAVE_LOCALTIME_R 0/g' config.h
sed -i 's/HAVE_PTHREAD_CANCEL 1/HAVE_PTHREAD_CANCEL 0/g' config.h
sed -i 's/HAVE_VALGRIND_VALGRIND_H 1/HAVE_VALGRIND_VALGRIND_H 0/g' config.h

tmp_file=".tmpfile"
## remove invalid restrict define
sed 's/#define av_restrict restrict/#define av_restrict/' ./config.h >$tmp_file
mv $tmp_file ./config.h

## replace original FFMPEG_CONFIGURATION define with $FF_CONFIG_OPTIONS
sed '/^#define FFMPEG_CONFIGURATION/d' ./config.h >$tmp_file
mv $tmp_file ./config.h
total_line=`wc -l ./config.h | cut -d' ' -f 1`
tail_line=`expr $total_line - 3`
head -3 config.h > $tmp_file
echo "#define FFMPEG_CONFIGURATION \"${FF_CONFIG_OPTIONS}\"" >> $tmp_file
tail -$tail_line config.h >> $tmp_file
mv $tmp_file ./config.h

rm -f config.err

## rm BUILD_ROOT information
sed '/^BUILD_ROOT=/d' ./ffbuild/config.mak > $tmp_file
rm -f ./ffbuild/config.mak
mv $tmp_file ./ffbuild/config.mak

## rm arm-eabi-gcc
sed '/^CC=arm-eabi-gcc/d' ./ffbuild/config.mak > $tmp_file
rm -f ./ffbuild/config.mak
mv $tmp_file ./ffbuild/config.mak

## rm arm-eabi-gcc
sed '/^AS=arm-eabi-gcc/d' ./ffbuild/config.mak > $tmp_file
rm -f ./ffbuild/config.mak
mv $tmp_file ./ffbuild/config.mak


## rm arm-eabi-gcc
sed '/^LD=arm-eabi-gcc/d' ./ffbuild/config.mak > $tmp_file
rm -f ./ffbuild/config.mak
mv $tmp_file ./ffbuild/config.mak

## rm arm-eabi-gcc
sed '/^DEPCC=arm-eabi-gcc/d' ./ffbuild/config.mak > $tmp_file
rm -f ./ffbuild/config.mak
mv $tmp_file ./ffbuild/config.mak

sed -i 's/restrict restrict/restrict /g' config.h

sed -i '/getenv(x)/d' config.h
sed -i 's/HAVE_DOS_PATHS 0/HAVE_DOS_PATHS 1/g' config.h

