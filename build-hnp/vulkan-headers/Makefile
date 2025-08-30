include ../utils/Makefrag

SOURCE_URL = https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/vulkan-sdk-1.4.309.0.tar.gz
SOURCE_FILE = vulkan-sdk-1.4.309.0.tar.gz
SOURCE_DIR = Vulkan-Headers-vulkan-sdk-1.4.309.0
CMAKE_ARGS = -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=$(OHOS_ARCH) -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_POLICY_VERSION_MINIMUM=3.5

$(eval $(call define_cmake_package))
