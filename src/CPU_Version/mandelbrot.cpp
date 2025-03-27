
#include<bits/stdc++.h>
#include<nlohmann/json.hpp>  // 这是一个处理 json 的外部库
#include<opencv2/opencv.hpp> // opencv 显示库
#include<tbb/tbb.h>          // intel tbb 库，是一个支持多线程处理的开源库

using namespace std;
using json = nlohmann::json;

class Complex {
private:
    double a,b;
public:    
    Complex operator+(const Complex &y) {
        Complex tmp;
        tmp.a = this->a + y.a;
        tmp.b = this->b + y.b;
        return tmp;
    } 
    Complex operator*(const Complex &y) {
        Complex tmp;
        tmp.a = this->a * y.a - this->b * y.b;
        tmp.b = this->a * y.b + this->b * y.a;
        return tmp;
    }   
    Complex operator*(const double k) {
        Complex tmp;
        tmp.a = this->a * k;
        tmp.b = this->b * k;
        return tmp;
    }
    // 以上编写了复数的乘法和加法
    double modSqr() {
        return a * a + b * b; // 模的平方
    }
    Complex(double aa = 0, double bb = 0) {
        a = aa, b = bb;
    } // 构造函数
};
const int WIDTH = 600, HEIGHT = 800;
double xmin, xmax, ymin, ymax;
int resLevel, width, height, loop;
double step;

cv::Mat canvas = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1);

double getDepth(Complex &c) {
    Complex x = Complex(0, 0);
    Complex dx = Complex(0, 0);
    int dep = 0;
    while(dep < loop){
        if(x.modSqr() > 4) {
            return sqrt(x.modSqr() / dx.modSqr()) * 0.5 * log(x.modSqr()); //距离估算算法
        }
        x = x * x + c;
        dx = x * 2.0 * dx + Complex(1.0, 0); //迭代复数的导数
        dep ++;
    }
    return -1;
}

void reDraw() {
    step = 1.0 / pow(10.0, resLevel);
    canvas = cv::Mat::zeros(height, width, CV_8UC1);
    int N = static_cast<int>((xmax - xmin) / step);  // 因为 tbb 的 blocked_range 仅支持整数分块，把 double 映射为 int
    tbb::parallel_for(tbb::blocked_range<int>(0, N), // 使用 Intel tbb 库的多线程来运算
       [&](const tbb::blocked_range<int>& r) {
           for (int ii = r.begin(); ii != r.end(); ++ii) {
                double i = xmin + 1.0 * ii * step;
                for(double j = ymin; j <= ymax; j += step) {
                    Complex c = Complex(i, j);
                    double dis = getDepth(c);
                    int locx = round((i - xmin) / step), locy = (j - ymin) / step; // 此处的 round 函数是因为多线程分块的整数重新映射回 double 时出现精度丢失
                    if(locx < 0 || locx > width || locy < 0 || locy > height) {
                        continue;
                    }
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
    );
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
