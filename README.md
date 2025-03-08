### 外部依赖

- opencv

    安装指令为 `sudo apt-get install libopencv-dev`。

- nlohmann/json

    安装指令为 `sudo apt-get install nlohmann-json3-dev`。

- Intel TBB

    安装指令为 `sudo apt-get install libtbb-dev`。

**以上所有库都使用CMake动态链接**

### 编译方法以及使用

1. 在目录 src/build中运行 `cmake ..` ，然后运行 `make`
2. 配置 config.json

- xmin, xmax, ymin, ymax 分别表示 x 方向和 y 方向的范围
- loop_time 将设置最大迭代次数，超出该迭代次数仍然没有逃逸出半径为 2 的圆的点将被认为被包含于 Mandelbrot 集合
- resolution_level 设置分辨率等级，表示数轴上单位 1 长度将由 10^resolution_level 个像素点来表示，请不要将他的初始值设置的太大（不应该超过 3，否则部分机器的 opencv 可能会无法显示图像）

3. 在 build 目录下执行 `./Test`

---

开始程序将会用你所设定的范围和分辨率生成一张图片，你可以分别按 w a s d 键来平移显示的图像范围。

或者你也可以按 z 键放大，这会将你 x 轴和 y 轴的数字范围同时缩小为十分之一，同时将 resolution_level 加 1。

你也可以按 x 键缩小，它所造成的结果与 z 完全相反。

如果你想保存当前看到的图像，你可以按 p 键，这将会输出一张命名为 outputX.jpg 的图片，X 将从 1 开始，每输出一张增加 1。如果重新运行程序，他将被重置。

如果你想关闭程序，按 b 键即可。

### 注意事项

1. 由于不明原因，在不同版本的 Intel TBB 外部库中，CMake 所需要用的动态链接指令是不同的，有 `target_link_libraries(XXXXX TBB::tbb)` 和 `target_link_libraries(XXXXX ${TBB_LIBRARY})` 两种。如果无法编译，尝试将指令替换。

2. config.json 请不要有任何一项缺省，这会导致崩溃。如果不想进行配置，在 build 目录下有一个默认满足要求的 config

3. 理论上使用 BigDecimal 等类型可以保证无限放大，但那将极大增加计算的常数，而由于使用 double 类型，当放大的倍数非常大使得 resolution_level 很大时，因为精度损失，无法保证图像精确。