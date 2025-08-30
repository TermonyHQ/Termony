# does not work
export USE_CCACHE := 0

include ../utils/Makefrag

SOURCE_URL = https://github.com/KhronosGroup/Vulkan-Tools/archive/refs/tags/vulkan-sdk-1.4.309.0.tar.gz
SOURCE_FILE = vulkan-sdk-1.4.309.0.tar.gz
SOURCE_DIR = Vulkan-Tools-vulkan-sdk-1.4.309.0

CMAKE_ARGS = -DCMAKE_INSTALL_PREFIX=$(PREFIX) \
	-DCMAKE_PREFIX_PATH=$(shell pwd)/../sysroot \
	-DCMAKE_SYSTEM_NAME=Linux \
	-DCMAKE_SYSTEM_PROCESSOR=$(OHOS_ARCH) \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
	-DBUILD_WSI_XCB_SUPPORT=OFF \
	-DBUILD_WSI_XLIB_SUPPORT=OFF \
	-DBUILD_WSI_WAYLAND_SUPPORT=OFF

$(eval $(call define_cmake_package))
