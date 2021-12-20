#include <opencv2/opencv.hpp>
#include <math.h>

#define WIDE 320
#define HEIGHT 180
#define PAN 58.2
#define TILT 34
#define PI 3.1415926


using namespace cv;
using namespace std;

void findMousePoint(int x, int y)
{
    Mat img;
    img = imread("./result/fisheye3.jpg", IMREAD_COLOR);
    double theta, R, Xf, Yf;
    double w;
    double ry, rx;
    R = img.size().height / 2;
    w = img.size().height * PI;
    printf("%d %d\n", y, x);
    x = x - R;
    y = y - R;
    ry = sqrt(x ^ 2 + y ^ 2);

    theta = atan2(y, x) * 180 / PI;
   
    printf("%f %d\n", ry, 3^2);
}

void onMouse(int event, int x, int y, int flags, void* param) {
    switch (event) {

    case EVENT_LBUTTONDOWN:
        //printf("%d %d\n", x, y);
        findMousePoint(x, y);
        break;

    case EVENT_LBUTTONUP:
        //cout << "Button up mouse left button" << endl;
        break;

    }
}



Point2f matchFisheye(int i, int j, double R, double cfx, double cfy, double H, double W) {
    Point2f fisheyePoint;
    //i 와이드 j 높이,W둘레
    double theta, r, Xf, Yf;

    r = j / H * R;
    theta = i / W * 2.0 * PI;
    Xf = cfx + r * cos(theta);
    Yf = cfy + r * sin(theta);
    fisheyePoint.x = Xf  ;
    fisheyePoint.y = Yf  ;


    return fisheyePoint;
}   
Mat wraping2(Mat src)
{
    vector<Point2f> corners(4);
    vector<Point2f> warpCorners(4);
    double co = 10.8 / PAN;

    corners[0] = Point2f(0, 0);
    corners[1] = Point2f(src.size().width * co, 0);
    corners[2] = Point2f(0, src.size().height);
    corners[3] = Point2f(src.size().width * co, src.size().height);

    if (src.size().height == 47)
    {
        Size warpSize(WIDE * co, src.size().height);
        Mat warpImg(warpSize, src.type());
        warpCorners[0] = Point2f(0, 0);
        warpCorners[1] = Point2f(warpImg.cols, 0);
        warpCorners[2] = Point2f(0, warpImg.rows);
        warpCorners[3] = Point2f(warpImg.cols, warpImg.rows);
        Mat trans = getPerspectiveTransform(corners, warpCorners);
        warpPerspective(src, warpImg, trans, warpSize);

        return warpImg;
    }
    else {
        Size warpSize(WIDE * co, HEIGHT);
        Mat warpImg(warpSize, src.type());
        warpCorners[0] = Point2f(0, 0);
        warpCorners[1] = Point2f(warpImg.cols, 0);
        warpCorners[2] = Point2f(0, warpImg.rows);
        warpCorners[3] = Point2f(warpImg.cols, warpImg.rows);
        Mat trans = getPerspectiveTransform(corners, warpCorners);
        warpPerspective(src, warpImg, trans, warpSize);

        return warpImg;

    }


}
Mat wraping(Mat src, int a, int W, int H)
{
    vector<Point2f> corners(4);

    double co4 = W * cos(17 * PI / 180);
    double co1 = cos(51 * PI / 180);
    double ans = W / ((1 / co1) * co4);
    ans = 1 - ((1 - ans) / 2);
    double co2 = 1 - ((1 - cos(22 * PI / 180)) / 2);
    double co3 = cos(85 * PI / 180);
    double ans2 = W / ((1 / co3) * co4);
    ans2 = 1 - ((1 - ans2) / 2);
    double co6 = 5 / 34.0;

    if (a == 2){
        corners[0] = Point2f(W - (W * ans), 0);
        corners[1] = Point2f(W * ans, 0);
        corners[2] = Point2f(0, H);
        corners[3] = Point2f(W, H);
    }
    else if (a == 1){
        corners[0] = Point2f(W - (W * ans2), 0);
        corners[1] = Point2f(W * ans2, 0);
        corners[2] = Point2f(W - (W * ans), H);
        corners[3] = Point2f(W * ans, H);
    }
    else {
        corners[0] = Point2f(W/2.0 - 50, H - (H * co6));
        corners[1] = Point2f(W/2.0 + 50, H - (H * co6));
        corners[2] = Point2f(W - (W * ans2), H);
        corners[3] = Point2f(W * ans2, H);
    }

    vector<Point2f> warpCorners(4);

    if (a == 0){
        Size warpSize(WIDE, WIDE * co6);
        Mat warpImg(warpSize, src.type());
        warpCorners[0] = Point2f(0, 0);
        warpCorners[1] = Point2f(warpImg.cols, 0);
        warpCorners[2] = Point2f(0, warpImg.rows);
        warpCorners[3] = Point2f(warpImg.cols, warpImg.rows);

        Mat trans = getPerspectiveTransform(corners, warpCorners);
        warpPerspective(src, warpImg, trans, warpSize);

        return warpImg;
    }
    else {
        Size warpSize(WIDE, HEIGHT);
        Mat warpImg(warpSize, src.type());
        warpCorners[0] = Point2f(0, 0);
        warpCorners[1] = Point2f(warpImg.cols, 0);
        warpCorners[2] = Point2f(0, warpImg.rows);
        warpCorners[3] = Point2f(warpImg.cols, warpImg.rows);

        Mat trans = getPerspectiveTransform(corners, warpCorners);
        warpPerspective(src, warpImg, trans, warpSize);

        return warpImg;
    }

}


int main()
{
    String path("./image2/*.jpg");
    vector<String> str;

    glob(path, str, false);

    if (str.size() == 0){
        cout << "파일이 존재하지 않습니다." << endl;
    }
    else {
        cout << "파일 갯수:" << str.size() << endl;
    }

    Mat src[28];
    Mat resu;
    const char* c;
    int size = str.size();

    for (int i = 0; i < size; i++){
        c = str[i].c_str();
        src[i] = imread(c);
    }
    for (int i = 0; i < (size / 4) * 3; i++){
        src[i] = wraping(src[i], i / (size / 4), src[i].size().width, src[i].size().height);
        
        if (i % 7 == 6){
            src[i] = wraping2(src[i]);
        }
    }

    src[27] = wraping2(src[27]);
    for (int i = 21; i < size - 1; i++)
    {
        resize(src[i], src[i], Size(WIDE, HEIGHT));
    }

    for (int i = 0; i < size / 7; i++)
    {
        for (int j = 1; j < size / 4; j++)
        {
            hconcat(src[i * 7], src[(i * 7) + j], src[i * 7]);
        }
        if (i > 0)
        {
            vconcat(src[0], src[i * 7], src[0]);
        }
    }


    imwrite("./result/eye.jpg", src[0]);

    Mat origin;
    origin = imread("./result/eye.jpg", IMREAD_COLOR);

    imshow("fisheye", origin);
    imwrite("./result/origin.jpg", origin);
    resize(origin, origin, Size(4398, 700));
    double H, W;
    double R, cfx, cfy;
    H = origin.size().height;

    W = origin.size().width ;
    R = W / PI/2.0;
    int We;
    cfx = H ;
    cfy = H;
    We = (int)2 * PI * R;
    resu.create(H*2, H*2, origin.type());

    for (int i = 0; i < origin.size().width; i++){

        for (int j = 0; j < origin.size().height; j++){

            resu.at<Vec3b>(matchFisheye(i, j, R, cfx, cfy, R, We)) = origin.at<Vec3b>(Point(i, j));
        }

    }
    //resize(resu, resu,Size(540, 540));
    medianBlur(resu,resu, 3);
    imshow("resu", resu);
    imwrite("./result/fisheye3.jpg", resu);
    setMouseCallback("resu", onMouse, 0);

    waitKey();

    return 0;
}