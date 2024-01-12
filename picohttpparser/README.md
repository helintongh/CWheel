# picohttpparser测试方法

使用g++进行编译。

不带指令集优化的测试文件编译:

```shell
g++ -o test main.cpp -O3
```

带sse4.2指令集的优化的编译:

```shell
g++ -o test2 main.cpp -DCINATRA_SSE -msse4.2 -O3
```

带avx2指令集的优化的编译:

```shell
g++ -o test3 main.cpp -DCINATRA_AVX2 -mavx2 -fpermissive -O3
```

aarch64环境下neon指令集的优化的编译:

```shell
g++ -o v2_test -DCINATRA_ARM_OPT main.cpp -O3 -march=armv8-a+fp+simd
```