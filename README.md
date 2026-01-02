# Make TAP

![Build](https://github.com/mdontu/mktap/workflows/CI/badge.svg)

This tool can be used to create TAP files that can then be loaded onto a ZX Spectrum computer.

## Usage

```shell
mktap -a 32768 -n example -o example.tap example.bin
```

## Build and install

### Requirements

* cmake >= 3.19
* gcc >= 13.0
* make

### Release

```shell
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -v
sudo cmake --install build -v
```

### Debug

```shell
cmake -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build -v
sudo cmake --install build -v
```
