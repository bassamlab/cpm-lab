Readme.md


- sudo apt-get install default-jre


./build_all.sh Anleitung

- Foonathan Memory

	- Sicherstellen, dass RPI Toolchain auf 4.9 steht
		- export RASPBIAN_TOOLCHAIN=/opt/cross-pi-gcc/
	- cd cpm_lib/thirdparty/memory/
	- cmake -DCMAKE_TOOLCHAIN_FILE=../../../Toolchain.cmake -DFOONATHAN_MEMORY_BUILD_TOOLS=OFF -DFOONATHAN_MEMORY_BUILD_TESTS=OFF ..


- Fast-DDS-Gen
	- ./gradlew
	- ./gradlew build

	- eventuell Fehler am Ende, klappt aber doch



