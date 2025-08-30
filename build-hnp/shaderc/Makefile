include ../utils/Makefrag

SOURCE_URL = https://github.com/google/shaderc/archive/refs/tags/v2025.3.tar.gz
SOURCE_FILE = v2025.3.tar.gz
SOURCE_DIR = shaderc-2025.3

PATCH_SOURCE = make subproject

CMAKE_ARGS = -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=$(OHOS_ARCH) -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DSHADERC_SKIP_TESTS=ON

SPIRVTools_URL = https://github.com/KhronosGroup/SPIRV-Tools/archive/refs/tags/vulkan-sdk-1.4.309.0.tar.gz
SPIRVTools_FILE = SPIRV-Tools.tar.gz
SPIRVTools_DIR = $(SOURCE_DIR)/third_party/spirv-tools

SPIRVHeaders_URL = https://github.com/KhronosGroup/SPIRV-Headers/archive/refs/tags/vulkan-sdk-1.4.309.0.tar.gz
SPIRVHeaders_FILE = SPIRV-Headers.tar.gz
SPIRVHeaders_DIR = $(SPIRVTools_DIR)/external/spirv-headers

glslang_URL = https://github.com/KhronosGroup/glslang/archive/refs/tags/15.4.0.tar.gz
glslang_FILE = glslang.tar.gz
glslang_DIR = $(SOURCE_DIR)/third_party/glslang

$(eval $(call define_cmake_package))

subproject: download/$(SPIRVTools_FILE) download/$(SPIRVHeaders_FILE) download/$(glslang_FILE)
	mkdir temp/$(SPIRVTools_DIR)
	cd temp/$(SPIRVTools_DIR) && tar vxf $(shell pwd)/download/$(SPIRVTools_FILE) --strip-components=1

	mkdir temp/$(SPIRVHeaders_DIR)
	cd temp/$(SPIRVHeaders_DIR) && tar vxf $(shell pwd)/download/$(SPIRVHeaders_FILE) --strip-components=1

	mkdir temp/$(glslang_DIR)
	cd temp/$(glslang_DIR) && tar vxf $(shell pwd)/download/$(glslang_FILE) --strip-components=1

download/$(SPIRVTools_FILE):
	mkdir -p download
	cd download && wget -O $(SPIRVTools_FILE) --retry-connrefused --read-timeout=20 --timeout=15 $(SPIRVTools_URL)

download/$(SPIRVHeaders_FILE):
	mkdir -p download
	cd download && wget -O $(SPIRVHeaders_FILE) --retry-connrefused --read-timeout=20 --timeout=15 $(SPIRVHeaders_URL)

download/$(glslang_FILE):
	mkdir -p download
	cd download && wget -O $(glslang_FILE) --retry-connrefused --read-timeout=20 --timeout=15 $(glslang_URL)
