nju_minnow
==============================

Stanford cs144课程的代码个人实现。本代码为本校课程定制，与cs144原版略有不同。

cs144课程网址: https://cs144.stanford.edu

运行方式：

To set up the build system: `cmake -S . -B build`

To compile: `cmake --build build`

To run tests: `cmake --build build --target test`

To run speed benchmarks: `cmake --build build --target speed`

To run clang-tidy (which suggests improvements): `cmake --build build --target tidy`

To format code: `cmake --build build --target format`
