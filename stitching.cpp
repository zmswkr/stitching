#include <opencv2/opencv.hpp>
#include <math.h>


#define WIDE 640
#define HEIGHT 360
#define PAN 58.2
#define TILT 34
#define PI 3.1415926


using namespace cv;
using namespace std;

Point2f matchFisheye(int i, int j, double R, double cfx, double cfy, double H, double W) {
    Point2f fisheyePoint;
    double theta, r, Xf, Yf;

    r = j / H * R;
    theta = i / W * 2.0 * PI;
    Xf = cfx + r * sin(theta);
    Yf = cfy + r * cos(theta);
    fisheyePoint.x = Xf / 2;
    fisheyePoint.y = Yf / 2;


    return fisheyePoint;
}
Mat wraping(Mat src, int a)
{
    vector<Point2f> corners(4);

    double co4 = 1920 * cos(17 * PI / 180);
    double co = cos(TILT * PI / 180);

    //double co1 = cos(54 * PI / 180);
    //double ans = 1920/(1 / co1 * co4);
    //ans = 1 - ((1 - ans) / 2);

    double co2 = 1 - ((1 - cos(22 * PI / 180)) / 2);
    double co3 = cos(68 * PI / 180);
    double pa = (double)12 / (double)34;  //마음에 걸리는부분 수정필요 임시값
    double pa2 = (double)22 / (double)34; //이하동문
    //pa2 = (1920 - (1920 * co) - (1920 - (1920 * co * co))) * co2;
    pa2 = (1920 - (1920 * co3));

    if (a == 1)
    {
        corners[0] = Point2f(1920 - (1920 * co), 0);
        corners[1] = Point2f(1920 * co, 0);
        corners[2] = Point2f(0, 1080);
        corners[3] = Point2f(1920, 1080);
        //corners[0] = Point2f(1920 - (1920 * ans), 0);
        //corners[1] = Point2f(1920 * ans, 0);
        //corners[2] = Point2f(0, 1080);
        //corners[3] = Point2f(1920, 1080);
    }
    else
    {
        /*corners[0] = Point2f((1920 - (1920 * co)) - pa2,1080- (1080* co2));
        corners[1] = Point2f((1920 * co) + pa2, 1080-(1080 *co2));
        */
        corners[0] = Point2f((1920 - pa2), 0);
        corners[1] = Point2f(pa2, 0);
        corners[2] = Point2f(1920 - (1920 * co), 1080);
        corners[3] = Point2f((1920 * co), 1080);
    }


    /*
    corners[0] = Point2f(0, 0);
    corners[1] = Point2f(1920, 0);
    corners[2] = Point2f(0, 1080);
    corners[3] = Point2f(1920, 1080);
    */
    Size warpSize(WIDE, HEIGHT);

    Mat warpImg(warpSize, src.type());


    vector<Point2f> warpCorners(4);
    warpCorners[0] = Point2f(0, 0);
    warpCorners[1] = Point2f(warpImg.cols, 0);
    warpCorners[2] = Point2f(0, warpImg.rows);
    warpCorners[3] = Point2f(warpImg.cols, warpImg.rows);


    //Transformation Matrix 구하기
    Mat trans = getPerspectiveTransform(corners, warpCorners);

    //Warping
    warpPerspective(src, warpImg, trans, warpSize);
    /*
    for (int i = 0; i < corners.size(); i++) {

        circle(src, corners[i], 3, Scalar(0, 255, 0), 3);
    }
    */
    return warpImg;
}


int main()
{
    String path("./image2/*.jpg");

    vector<String> str;

    glob(path, str, false);



    if (str.size() == 0)
    {
        cout << "파일이 존재하지 않습니다." << endl;
    }
    else {
        cout << "파일 갯수:" << str.size() << endl;
    }

    Mat src[21];
    Mat resu;
    const char* c;
    int size = str.size();


    for (int i = 0; i < size; i++)
    {
        c = str[i].c_str();
        src[i] = imread(c);
    }
    for (int i = 0; i < size / 3 * 2; i++)
    {
        src[i] = wraping(src[i], i / (size / 3));
    }


    for (int i = 0; i < size; i++)
    {
        resize(src[i], src[i], Size(WIDE, HEIGHT));
    }

    for (int i = 0; i < 3; i++)
    {
        for (int j = 1; j < size / 3; j++)
        {
            hconcat(src[i * 7], src[(i * 7) + j], src[i * 7]);
        }
        if (i > 0)
        {
            vconcat(src[0], src[i * 7], src[0]);
        }
    }


    imshow("fisheye", src[0]);
    imwrite("./result/eye.jpg", src[0]);

    Mat origin;
    origin = imread("./result/eye.jpg", IMREAD_COLOR);

    int H, W;
    double R, cfx, cfy;
    H = origin.size().height * 2;

    R = H * 2 / 7 + 10;
    W = origin.size().width / 2;
    int We;
    cfx = H / 2;
    cfy = H / 2;
    We = (int)2 * PI * R;
    resu.create(H / 2 + 10, H / 2 + 10, origin.type());
    imwrite("./result/origin.jpg", origin);

    for (int i = 0; i < origin.size().width; i++)
    {
        for (int j = 0; j < origin.size().height; j++)
        {
            resu.at<Vec3b>(matchFisheye(i, j, R, cfx, cfy, R, We)) = origin.at<Vec3b>(Point(i, j));
            //printf("%d %d %d \n", origin.at<Vec3b>(Point(i, j))[0], origin.at<Vec3b>(Point(i, j))[1], origin.at<Vec3b>(Point(i, j))[2]);
            //printf("%d %d\n", i, j);
            if (i == 1439 && j == 1079)
            {
                //printf("%d %d\n", i, j);
                //printf("%d %d\n", i, j);
            }
        }//printf("%d \n", i);
    }
    //resize(resu, resu,Size(540, 540));
    imshow("resu", resu);
    imwrite("./result/fisheye3.jpg", resu);
    waitKey();

    return 0;
}