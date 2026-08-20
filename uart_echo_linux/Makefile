.PHONY: configure build run clean

clean:
	rm -rf build/*

configure:
	cmake -S . -B build

build:
	cmake --build build

run:
	build/uart-server
