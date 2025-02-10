# LFU cache
---
## Overview
Least Frequently Used cache is an algorithm with better performance than LRU

## Build and usage

1. clone repo

```bash
git clone https://github.com/UjeNeTORT/lfu_cache && cd lfu_cache 
```

2. build

```bash
mkdir build && cd build
```

```bash
cmake .. -G Makefile -DCMAKE_BUILD_TYPE=Release
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
./cache
```

## Notes

