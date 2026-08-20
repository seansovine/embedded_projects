.PHONY: clean configure build

clean:
	rm -rf build/*

configure:
	cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake \
		-DCMAKE_COMMAND=cube-cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
		-S . -B build/Debug -G Ninja

build:
	cmake --build build/Debug
