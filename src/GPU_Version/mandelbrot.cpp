#define CL_HPP_ENABLE_EXCEPTIONS //新的 opencl 库将 Error 的 class 放到了一个 #if 当中
                                 //这里满足这个条件以确保这个下面的 Error 类可以被正常使用
                                 //很多 AI 提供的 opencl 的使用方式，都 include 的是 CL/cl2.hpp 这个库，如果翻开这个库文件，你会发现这个库被重定向到了 CL/opencl.hpp
                                 //而原本的 cl2.hpp 这个库已经被弃用了，AI 提供的是明显过时的信息，而且 AI 也完全没有提到这个 #if 的问题
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 
//同样的问题，AI 给出的 sources 的构造函数的方式，在新的 opencl 库中被放在了一个 #if 中，也就是说被默认弃用了，默认使用的是 vector<unsigned char> 类型，提供的信息也是过时的
#include<bits/stdc++.h>
#include<nlohmann/json.hpp>  // 这是一个处理 json 的外部库
#include<opencv2/opencv.hpp> // opencv 显示库
#include<CL/opencl.hpp>



using namespace std;
using json = nlohmann::json;

struct double2{
    double x,y;
    double2(double xx, double yy){
        x = xx, y = yy;
    }
    double2(){
        x = 0, y = 0;
    }
};
const int WIDTH = 600, HEIGHT = 800;
double xmin, xmax, ymin, ymax;
int resLevel, width, height, loop;
double step;

cv::Mat canvas = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1);
//以下是 opencl 的内核的 opencl C 的代码，将会由 opencl 编译为二进制代码
const char* kernelSource = R"(
    __kernel void getDepth(__global const double2* a, const int loop, __global double* dis) {
        double2 x = (double2)(0.0, 0.0);
        double2 dx = (double2)(0.0, 0.0);
        double tmpx = 0.0, tmpdx = 0.0;
        int dep = 0;
        int gid = get_global_id(0);
        while(dep < loop){
            if(x.x * x.x + x.y * x.y > 4){
                dis[gid] = sqrt((x.x * x.x + x.y * x.y) / (dx.x * dx.x + dx.y * dx.y)) * 0.5 * log(x.x * x.x + x.y * x.y);
                break;
            }
            tmpx = x.x;
            x.x = x.x * x.x - x.y * x.y + a[gid].x;
            x.y = 2.0 * tmpx * x.y + a[gid].y;
            tmpdx = dx.x;
            dx.x = (x.x * dx.x - x.y * dx.y) * 2.0 + 1.0;
            dx.y = (x.x * dx.y + x.y * tmpdx) * 2.0;
            dep++;
        }
        if(dep == loop)
            dis[gid] = -1.0;
    }
    )";
    
//用于缓存上下文、队列和程序
cl::Context context;
cl::CommandQueue que;
cl::Program program;

void initOpenCL() { // 初始化 OpenCL 环境
    try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms); // 获取平台和设备
        cl::Device device;
        int cnt = 0;
        bool isok = false;
        for(auto platform: platforms){
            cnt ++;
            std::vector<cl::Device> devices;
            platform.getDevices(CL_DEVICE_TYPE_GPU, &devices); //优先寻找 GPU
            if(devices.empty()){
                cerr << "No GPU device at platform " << cnt << endl;
            }
            else{
                cerr << "GPU Device found at platform " << cnt << endl;
                device = devices.front();
                isok = true;
                break;
            }
        }
        if(!isok){ //如果没有 GPU 设备，使用 CPU 设备
            for(auto platform: platforms){
                cnt++;
                std::vector<cl::Device> devices;
                platform.getDevices(CL_DEVICE_TYPE_CPU, &devices); //再寻找 CPU
                if(devices.empty()){
                    cerr << "No CPU device at platform " << cnt << endl;
                }
                else{
                    cerr << "CPU Device found at platform " << cnt << endl;
                    device = devices.front();
                    break;
                }
            }
        }
        
        
        context = cl::Context(device);
        que = cl::CommandQueue(context, device); // 创建上下文和命令队列

        // 创建并构建程序
        cl::Program::Sources sources(1, make_pair(kernelSource, strlen(kernelSource)));
        program = cl::Program(context, sources);
        try {
            program.build("-cl-std=CL1.2");
        } catch (cl::BuildError &e) {
            string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
            cerr << "Build log for device: " << buildLog << endl; //获取 build 过程中的 Error
            throw;
        }

    } catch (const cl::Error& err) {
        cerr << "ERROR: " << err.what() << ", " << err.err() << endl;
        throw;
    }
}

void reDraw() {
    step = 1.0 / pow(10.0, resLevel);
    canvas = cv::Mat::zeros(height, width, CV_8UC1);

    const size_t siz = width * height;
    vector<double2> vec(siz);
    vector<double> Dis(siz); //在原本平台的数据
    for(int locx = 0; locx < width; locx++){
        for(int locy = 0; locy < height; locy++){
            vec[locx * height + locy] = double2(step * locx + xmin, step * locy + ymin);
        } //在 opencl 运算中不支持二维数组，将二维展开到一维
    }
    //给准备传入的 vector 设置初值

    cl::Kernel kernel(program, "getDepth");

    cl::Buffer bufferA(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(double2) * siz, vec.data());
    cl::Buffer bufferResult(context, CL_MEM_WRITE_ONLY, sizeof(double) * siz); //创建 opencl 的数据缓冲区

    kernel.setArg(0, bufferA);
    kernel.setArg(1, loop);
    kernel.setArg(2, bufferResult);

    que.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(siz), cl::NullRange);
    que.enqueueReadBuffer(bufferResult, CL_TRUE, 0, sizeof(double) * siz, Dis.data()); //运行已经编译好的内核

    for(int locx = 0; locx < width; locx++){
        for(int locy = 0; locy < height; locy++) {
            double dis = Dis[locx * height + locy];
            if(dis < 0) {
                canvas.at<uchar>(locy, locx) = 255;
            }
            else {
                long double areaSquare = (xmax - xmin) * (ymax - ymin) / width / height;
                long double density = M_PI * dis * dis / areaSquare;
                int grey;
                if(density > 1)
                    grey = 0;
                else 
                    grey = density * 255;
                canvas.at<uchar>(locy, locx) = grey; // 此处不使用 %，让其自然溢出
            }
        }
    }
}

int main() {
    ifstream in("config.json", ios::binary);

    if(!in.is_open()) {
        cerr << "Can't find config file!" << endl;
        return 1;
    }

    json j;
    try {
        in >> j;
    } catch (json::parse_error& ex) { // 捕获解析错误
        cerr << "JSON error: " << ex.what() << endl;
        return 2;
    }
    xmin = j["xmin"], xmax = j["xmax"];
    ymin = j["ymin"], ymax = j["ymax"];
    resLevel = j["resolution_level"]; // 以上读入config.json获取要求的范围和分辨率
                                      // 分辨率等级代表，单位 1 的长度将由 10^level 个点来表示
    loop = j["loop_time"]; //代表递归的最大迭代数
    step = 1.0 / pow(10.0, resLevel);
    width = (xmax - xmin) / step, height = (ymax - ymin) / step;
    cerr << width << " " << height << endl;
    int cnt = 0;
    initOpenCL();
    while(1) {
        double midx = (xmax + xmin) / 2.0, midy = (ymax + ymin) / 2.0;
        double lenx = (xmax - xmin), leny = (ymax - ymin); //计算中心点以及区间长度

        auto start = chrono::high_resolution_clock::now();
    
        reDraw(); // 改变后重新绘图

        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end - start; // 一次重新绘图的计算耗时
        
        cout << "Redraw time: " << elapsed.count() << " s\n";

        cv::imshow("Display window", canvas);
        int key = cv::waitKey(0);
        if(key ==  'z') {
            xmax = midx + lenx / 20.0, xmin = midx - lenx / 20.0;
            ymax = midy + leny / 20.0, ymin = midy - leny / 20.0;
            
            resLevel++;
        } // 放大

        if(key == 'x') {
            xmax = midx + lenx * 5, xmin = midx - lenx * 5;
            ymax = midy + leny * 5, ymin = midy - leny * 5;
            
            resLevel--;
        } // 缩小

        if(key == 'w') {
            ymax -= leny * 0.2, ymin -= leny * 0.2;
        }
        if(key == 's') {
            ymax += leny * 0.2, ymin += leny * 0.2;
        }
        if(key == 'a') {
            xmax -= lenx * 0.2, xmin -= lenx * 0.2;
        }
        if(key == 'd') {
            xmax += lenx * 0.2, xmin += lenx * 0.2;
        } //移动
        if(key == 'p'){
            cv::imwrite("output" + to_string(++cnt) + ".jpg", canvas);
        } //输出为 jpg
        if(key == 'b') {
            exit(0);
        } //退出
    }
    // cv::namedWindow("Display window", cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    // cv::resizeWindow("Display window", 800, 600);
}
