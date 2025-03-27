### 外部依赖

- opencv

    安装指令为 `sudo apt-get install libopencv-dev`。

- nlohmann/json

    安装指令为 `sudo apt-get install nlohmann-json3-dev`。

- Intel TBB（仅限于 CPU 版本）

    安装指令为 `sudo apt-get install libtbb-dev`。

- OpenCL（仅限于 GPU 版本）

    安装指令为 `sudo apt-get install ocl-icd-opencl-dev opencl-headers`

- PoCL（仅限于基于 WSL2 的 GPU 版本）

    安装见下 GPU 版本的使用

- CLang 以及其关联的 Lib（仅限于基于 WSL2 的 GPU 版本）

    安装见下

- Nvidia CUDA

    安装见下

**以上所有库都使用CMake动态链接**

### 编译方法以及使用

#### CPU 版本的编译
1. 在目录 `src/CPU_Version/build` 中运行 `cmake ..` ，然后运行 `make`，这将在目录 build 下生成一个名为 `Test` 的可执行文件
2. 配置 config.json

- `xmin`, `xmax`, `ymin`, `ymax` 分别表示 x 方向和 y 方向的范围
- `loop_time` 将设置最大迭代次数，超出该迭代次数仍然没有逃逸出半径为 2 的圆的点将被认为被包含于 Mandelbrot 集合
- `resolution_level` 设置分辨率等级，表示数轴上单位 1 长度将由 `10^resolution_level` 个像素点来表示，请不要将他的初始值设置的太大（不应该超过 3，否则部分机器的 opencv 可能会无法显示图像）

3. 在 build 目录下执行 `./Test`

#### GPU 版本的编译

1. 在目录 `src/CPU_Version/build` 中运行 `cmake ..` ，然后运行 `make`，这将在目录 build 下生成一个名为 `Test` 的可执行文件
2. 配置 OpenCL 的运行环境

- 如果你使用的是基于 WSL2 的 Linux 发行版，并且你将要使用 Nvidia GPU，你应当要使用 PoCL 以正确地在系统中运行 OpenCL（**沟槽的 WSL 对纯 OpenCL 的支持仅限于 Intel GPU，该系统的安装见下注意事项**）
- 如果你使用的是一般的 Linux 系统或者你使用的是 Intel GPU，那么你不需要配置 PoCL 环境，你应当只需要保证你的 OpenCL 和 CUDA 被正确安装，这里将不提供这种情况下的环境配置方法。 

3. 验证 OpenCL 能够找到你的 GPU。`sudo apt-get install clinfo` 以安装 clinfo，并在终端中执行 clinfo。如果你在 Device Name 中能够找到如 `NVIDIA GeForce XXX GPU` 的字段，那么环境应当配置正确。

4. 配置 config.json 同上

5. 在 build 目录下执行 `./Test`
---
#### 程序的使用
开始程序将会用你所设定的范围和分辨率生成一张图片，你可以分别按 w a s d 键来平移显示的图像范围。

或者你也可以按 z 键放大，这会将你 x 轴和 y 轴的数字范围同时缩小为十分之一，同时将 `resolution_level` 加 1。

你也可以按 x 键缩小，它所造成的结果与 z 完全相反。

如果你想保存当前看到的图像，你可以按 p 键，这将会输出一张命名为 `outputX.jpg` 的图片，X 将从 1 开始，每输出一张增加 1。如果重新运行程序，他将被重置。

如果你想关闭程序，按 b 键即可。

---

report.tex 的 Makefile 已经放在目录中，在目录 doc 中 `make` 即可。

### 注意事项和已知问题

1. （仅限于 CPU 版本）由于不明原因，在不同版本的 Intel TBB 外部库中，CMake 所需要用的动态链接指令是不同的，有 `target_link_libraries(XXXXX TBB::tbb)` 和 `target_link_libraries(XXXXX ${TBB_LIBRARY})` 两种。如果无法编译，尝试将指令替换。

2. `config.json` 请不要有任何一项缺省，这会导致崩溃。如果不想进行配置，在 build 目录下有一个默认满足要求的 `config.json`

3. 理论上使用 BigDecimal 等类型可以保证无限放大，但那将极大增加计算的常数，而由于使用的 double 类型的特殊结构，当放大的倍数非常大使得 `resolution_level` 很大时，精度损失，无法保证图像精确。

4. （仅限于 CPU 版本）由于上述精度问题，当不断放大到 resLevel 为 17 时，由于此时循环步长为 10^(-17)，这会导致 `j += step` 时加上去的值被忽略，导致程序出现死循环。这个 bug 是不可避免的，除非使用上述的 BigDecimal 或者改变循环逻辑，用 `int` 循环再重新映射回到 `double`，但那样的精度损失也是不可控的。

5. （仅限于 GPU 版本）请务必确保你的 clinfo 能正确识别你的 GPU，不然程序是无法运行的，即使你能编译出二进制文件，代码运行时 OpenCL 的 kernel 也是无法运行的。

6. （仅限于 GPU 版本）该版本的程序应当只能在 Intel GPU 或者 Nvidia GPU 的环境下正常运行，如果你使用的是 AMD GPU，不要使用此版本（我不想再写一个 ROCm 的版本了捏，而且 WSL 又不支持全版本的 OpenCL）。

#### （仅限于 GPU 版本）通过 PoCL 使 Nvidia GPU 能够在 WSL2 运行 OpenCL
在终端执行以下步骤（**请注意注释**）：

```
sudo apt update
sudo apt install -y python3-dev libpython3-dev build-essential ocl-icd-libopencl1 cmake git pkg-config make ninja-build ocl-icd-dev ocl-icd-opencl-dev libhwloc-dev zlib1g zlib1g-dev clinfo dialog apt-utils libxml2-dev opencl-headers

mkdir ~/Downloads
cd ~/Downloads

#如果你已经安装了一个 CUDA，请略过下面两行
wget https://developer.download.nvidia.com/compute/cuda/12.8.1/local_installers/cuda_12.8.1_570.124.06_linux.run
sudo bash ./cuda_12.8.1_570.124.06_linux.run --silent --toolkit --no-opengl-libs

#此处安装用于构建 PoCL 的 Clang 以及所需包
export LLVM_VERSION=14
sudo apt install -y libclang-${LLVM_VERSION}-dev clang-${LLVM_VERSION} llvm-${LLVM_VERSION}  libclang-cpp${LLVM_VERSION}-dev libclang-cpp${LLVM_VERSION} llvm-${LLVM_VERSION}-dev 

#从 github clone 下 PoCL 在本地 build
git clone https://github.com/pocl/pocl -b v6.0
mkdir pocl/build
cd pocl/build
cmake -DCMAKE_C_FLAGS=-L/usr/lib/wsl/lib \
  -DCMAKE_CXX_FLAGS=-L/usr/lib/wsl/lib \
  -DWITH_LLVM_CONFIG=/usr/bin/llvm-config-${LLVM_VERSION} \
  -DENABLE_HOST_CPU_DEVICES=OFF \
  -DENABLE_CUDA=ON ..

#完成 PoCL 的 make 并安装
make -j`nproc`
sudo make install
sudo mkdir -p /etc/OpenCL/vendors # amended by @kirse 's suggestion 
sudo cp /usr/local/etc/OpenCL/vendors/pocl.icd /etc/OpenCL/vendors/pocl.icd
```