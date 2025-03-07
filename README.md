# LFU cache
---
## Overview
Least Frequently Used cache is an algorithm with better performance than LRU

## Build and usage

### Build with script 

You can simply (install dependencies)[#Manual-build] and run `build.sh` bash script:

```bash
./build.sh
```

### Manual build

0. install dependencies

```bash
sudo apt install cmake, libgtest-dev
```

1. clone repo

```bash
git clone https://github.com/UjeNeTORT/lfu_cache && cd lfu_cache 
```

2. build

```bash
mkdir build && cd build
```

```bash
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
```

```bash
ninja
```

3. test cache

```bash
./test
```

or run program itself

```bash
./lfu 
```

```bash
./belady
```

## Notes

