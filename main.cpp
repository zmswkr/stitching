#include <opencv2/opencv.hpp>
#include <math.h>
#include <time.h>


const int PICTURE_X = 8; // The number of x-axis photos.
const int PICTURE_Y = 4; // The number of y-axis photos.
const float PAN = 58.2f; // CAMERA FOV's WIDE 
const float TILT = 34.0f; // CAMERA FOV's HEIGHT
const float APPLY_WIDE = 45.0f; // 0~360, PICTURE_X * APPLY_WIDE = 360 (APPLY_WIDE<=FOV_WIDE)
const float APPLY_HEIGHT = 26.75f; // 0~107, PICTURE_Y * APPLY_HEIGHT = (90+ (Fov of Height / 2))  (APPLY_HEIGHT<=FOV_HEIGHT)
const float PI = 3.1415926f;
const float CIRCLE_R = 500.0f; // FIsheye image's radius, The higher the size, the slower the speed.
const int ESC = 27;
using namespace cv;
using namespace std;

#define RADIAN(d)                   (PI*(d)/180.0)

void callImg(Mat* src, int size, vector<String> str)
{

    clock_t start, finish;
    double duration;
    start = clock();

    for (int i = 0; i < size; i++) {
        src[i] = imread(str[i]);
    }

    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    printf("Call IMG: %f초 \n", duration);
}

void cropOverlap(Mat* src, int size)
{

    clock_t start, finish;
    double duration;
    int w = (CIRCLE_R * 2 * PI) / PICTURE_X;
    int h = CIRCLE_R / PICTURE_Y;
    start = clock();
    for (int i = 0; i < size; i++) {
        //사진을 1920, 1080에서 320, 180의 사이즈로 resize    
        resize(src[i], src[i], Size(((CIRCLE_R * 2 * PI) / PICTURE_X), (CIRCLE_R / PICTURE_Y)));
        flip(src[i], src[i], 0);
    }

    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    printf("Crop overlap IMG: %f초 \n", duration);
    imwrite("./result3/eye.jpg", src[0]);


}


void calculPoint2(Mat* panorama, double theta, double t, Point2f fisheyePoint)
{
    Point2f fisheyePoint2;

    t = t / 107.0 * 90.0;
    double r = CIRCLE_R;
    r = r * sin(RADIAN(t)) / 1;
    float Cfx = 499.5;
    float Cfy = 499.5;
    float Xf = Cfx + r * cos(RADIAN(theta));
    float Yf = Cfy + r * sin(RADIAN(theta));
    if (Yf < 0 || Xf < 0)
    {
        printf("error");
    }
    fisheyePoint2.x = Xf;
    fisheyePoint2.y = Yf;

    int x = 0;
    int y = 0;

}
Point2f findFisheye(int Xe, int Ye, double R, double Cfx, double Cfy, double He, double We) {
    Point2f fisheyePoint;
    double theta, r, Xf, Yf; //Polar coordinates

    r = Ye / He * R;
    theta = Xe / We * 2.0 * PI;
    Xf = Cfx + r * sin(theta);
    Yf = Cfy + r * cos(theta);
    fisheyePoint.x = Xf;
    fisheyePoint.y = Yf;

    return fisheyePoint;
}

void panorama()
{
    Mat fisheyeImage, equirectangularImage;
    const string PATH_IMAGE = "./rere.jpg";
    fisheyeImage = imread(PATH_IMAGE, IMREAD_COLOR);
    namedWindow("Fisheye Image", WINDOW_AUTOSIZE);
    imshow("Fisheye Image", fisheyeImage);


    while (waitKey(0) != ESC) {
        //wait until the key ESC is pressed
    }

    //destroyWindow("Fisheye Image");

    int Hf, Wf, He, We;
    double R, Cfx, Cfy;

    Hf = fisheyeImage.size().height;
    Wf = fisheyeImage.size().width;
    R = Hf / 2; //The fisheye image is a square of 1400x1400 pixels containing a circle so the radius is half of the width or height size
    Cfx = Wf / 2; //The fisheye image is a square so the center in x is located at half the distance of the width
    Cfy = Hf / 2; //The fisheye image is a square so the center in y is located at half the distance of the height

    He = (int)R;
    We = (int)2 * PI * R;

    equirectangularImage.create(He, We, fisheyeImage.type());

    for (int Xe = 0; Xe < equirectangularImage.size().width; Xe++) {
        for (int Ye = 0; Ye < equirectangularImage.size().height; Ye++) {

            equirectangularImage.at<Vec3b>(Point(Xe, Ye)) = fisheyeImage.at<Vec3b>(findFisheye(Xe, Ye, R, Cfx, Cfy, He, We));
        }
    }

    namedWindow("Equirectangular Image", WINDOW_AUTOSIZE);
    imshow("Equirectangular Image", equirectangularImage);

    while (waitKey(0) != ESC) {
        //wait until the key ESC is pressed
    }

    //destroyWindow("Fisheye Image");

    imwrite("./32_panorama.jpg", equirectangularImage);

}

void MakeFisheye(Mat* src, int size)
{

    clock_t start, finish;
    double duration;
    start = clock();
    Point2f po;
    float fx = 0.52;
    float fy = 0.52 * 9 / 16.0;
    float u, v;
    float cx = (src[0].size().width - 1) / 2.0;
    float cy = (src[0].size().height - 1) / 2.0;
    float a, b, c, new_p, new_t;
    Mat result[2];

    result[0].create(2 * CIRCLE_R, 2 * CIRCLE_R, src[0].type());
    result[0] = Mat::zeros(2 * CIRCLE_R, 2 * CIRCLE_R, src[0].type());
    result[1].create(2 * CIRCLE_R, 2 * CIRCLE_R, src[0].type());
    result[1] = Mat::zeros(2 * CIRCLE_R, 2 * CIRCLE_R, src[0].type());

    static float R = ((float)(src[0].size().width)) / tanf((float)RADIAN(PAN / 2.0));
    for (int i = 0; i < 8; i++) {
        for (int m = 0; m < 4; m++) {
            for (int j = 0; j < src[i].size().width; j++) {
                for (int k = 0; k < src[i].size().height; k++) {
                    if (i >= 0) {
                        u = (j - cx) / fx;
                        v = (k - cy) / fy;

                        a = (R * cos(RADIAN(APPLY_WIDE * i)) * cos(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m)))) - u * sin(RADIAN(APPLY_WIDE * i)) + v * cos(RADIAN(APPLY_WIDE * i)) * sin(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m)))));
                        b = (R * sin(RADIAN(APPLY_WIDE * i)) * cos(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m)))) + u * cos(RADIAN(APPLY_WIDE * i)) + v * sin(RADIAN(APPLY_WIDE * i)) * sin(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m)))));
                        c = (-R * sin(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m)))) + v * cos(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m)))));

                        new_p = -atan2(a, b) * 180.0 / PI;
                        new_t = asinf(c / sqrt(R * R + u * u + v * v)) * 180.0 / PI;
                        new_t = 90.0 - new_t;

                        Point2f fisheyePoint2;
                        double r = CIRCLE_R;
                        r = r * new_t / 90.0;

                        float Xf = (result[0].size().width - 1) / 2.0 + r * cos(RADIAN(new_p));
                        float Yf = (result[0].size().height - 1) / 2.0 + r * sin(RADIAN(new_p));
                        fisheyePoint2.x = Xf / 2.0 + (result[0].size().width / 4.0);
                        fisheyePoint2.y = Yf / 2.0 + (result[0].size().height / 4.0);
                        if (new_t < 107)
                        {
                            if (result[0].at<Vec3b>(fisheyePoint2) == result[0].at<Vec3b>(Point(0, 0)))
                            {
                                result[0].at<Vec3b>(fisheyePoint2) = src[m * PICTURE_X + i].at<Vec3b>(Point(j, k));
                            }
                        }
                    }
                }
            }
        }
    }

    imshow("result", result[0]);
    imwrite("./rere.jpg", result[0]);
    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;

    printf("Make fisheye IMG: %f초 \n", duration);
    waitKey();
}


int main()
{
    String path("./image6/*.jpg"); //이미지 경로
    vector<String> str; //이미지 경로를 저장해줄 Vector
    glob(path, str, false);  //이미지를 한번에 불러와주는 opencv의 함수
    Mat src[PICTURE_X * PICTURE_Y];
    int size = str.size(); //불러온 이미지의 size

    if (str.size() == 0) {
        cout << "파일이 존재하지 않습니다." << endl;

        return 0;
    }
    else {
        cout << "파일 갯수:" << str.size() << endl;
    }


    callImg(src, size, str);
    cropOverlap(src, size);
    MakeFisheye(src, size);
    //panorama();


    return 0;
}