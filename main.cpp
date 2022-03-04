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
using namespace cv;
using namespace std;

#define RADIAN(d)                   (PI*(d)/180.0)
void findMousePoint(int x, int y) {
    double theta, r, w, tilt;
    tilt = 500 / (90.0 + TILT / 2.0);
    r = 500;
    x = x - r;
    y = y - r;
    w = sqrt(pow(x, 2) + pow(y, 2));
    theta = atan2(y, x) * 180 / PI;
    theta = theta + 157.5;
    if (theta > 180)
    {
        theta = theta - 360;
    }
    else if (theta < -180)
    {
        theta = theta + 360;
    }

    printf("Pan:%f      Tilt:-%f \n", theta, w / tilt);
}
void onMouse(int event, int x, int y, int flags, void* param) {
    switch (event) {

    case EVENT_LBUTTONDOWN:
        findMousePoint(x, y);
        break;

    case EVENT_LBUTTONUP:
        break;

    }
}

void callImg(Mat* src, int size, vector<String> str) {

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

void cropOverlap(Mat* src, int size) {

    clock_t start, finish;
    double duration;
    int w = (CIRCLE_R * 2 * PI) / PICTURE_X;
    int h = CIRCLE_R / PICTURE_Y;
    start = clock();
    for (int i = 0; i < size; i++) {
        //사진을 1920, 1080에서 640, 360의 사이즈로 resize (수행 속도를 위함)
        resize(src[i], src[i], Size(640, 360));
        GaussianBlur(src[i], src[i], Size(3, 3), 0);

    }

    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    printf("Crop overlap IMG: %f초 \n", duration);
    imwrite("./result3/eye.jpg", src[0]);


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

void panorama() {
    // fisheye 이미지를 input으로 넣어주면 파노라마 형식으로 펴주는 함수
    Mat fisheyeImage, equirectangularImage;
    const string PATH_IMAGE = "./result.jpg";
    fisheyeImage = imread(PATH_IMAGE, IMREAD_COLOR);
    namedWindow("Fisheye Image", WINDOW_AUTOSIZE);
    imshow("Fisheye Image", fisheyeImage);


    int  He, We;
    double R, Cfx, Cfy;

    R = fisheyeImage.size().height / 2;
    Cfx = fisheyeImage.size().width / 2.0;
    Cfy = fisheyeImage.size().height / 2.0;

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

    waitKey(0);


    imwrite("./32_panorama.jpg", equirectangularImage);

}

void MakeFisheye(Mat* src, int size) {

    cropOverlap(src, size);
    clock_t start, finish;
    double duration;
    start = clock();

    float u, v;
    float cx = (src[0].size().width - 1) / 2.0;
    float cy = (src[0].size().height - 1) / 2.0;
    float wx, wy, wz, new_p, new_t;
    Mat result;

    result.create(2 * CIRCLE_R, 2 * CIRCLE_R, src[0].type());
    result = Mat::zeros(2 * CIRCLE_R, 2 * CIRCLE_R, src[0].type());


    static float R = ((float)(src[0].size().width/2.0)) / tanf((float)RADIAN(PAN / 2.0));

    int _cos, _sin, _cos2, _sin2;

    for (int i = 0; i < 8; i++) {
        for (int m = 0; m < 4; m++) {
            // PAN 45도 단위로 8장 tilt 26.75도 단위로 4장 총 32장을 도는 for문
            for (int x = 0; x < src[i].size().width; x++) {
                for (int y = 0; y < src[i].size().height; y++) {
                    if (i >= 0) {
                        u = (cx - x) / R;
                        v = (cy - y) / R;
                        _cos = cos(RADIAN(APPLY_WIDE * i)) * 1000;
                        _cos2 = cos(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m)))) * 1000;
                        _sin = sin(RADIAN(APPLY_WIDE * i)) * 1000;
                        _sin2 = sin(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m)))) * 1000;

                        wx = _cos / 1000.0 * u - _sin / 1000.0 * _sin2 / 1000.0 * v - _sin / 1000.0 * _cos2 / 1000.0;
                        wy = _sin / 1000.0 * u + _cos / 1000.0 * _sin2 / 1000.0 * v + _cos / 1000.0 * _cos2 / 1000.0;
                        wz = 0 - _cos2 / 1000.0 * v + _sin2 / 1000.0;
                        //행렬 변환을 이용한 3D좌표계 추출
                        new_p = -atan2(wx, wy) * 180.0 / PI;
                        new_t = atan2(wz, sqrt(wx * wx + wy * wy)) * 180.0 / PI;
                        new_t = 90.0 + new_t;


                        Point2f fisheyePoint2;
                        double r = CIRCLE_R;
                        r = r * new_t / 107.0;

                        float Xf = (result.size().width - 1) / 2.0 + r * cos(RADIAN(new_p));
                        float Yf = (result.size().height - 1) / 2.0 + r * sin(RADIAN(new_p));
                        fisheyePoint2.x = Xf;
                        fisheyePoint2.y = Yf;
                        if (new_t < 107) {

                            if (result.at<Vec3b>(fisheyePoint2) == result.at<Vec3b>(Point(0, 0))) {
                                result.at<Vec3b>(fisheyePoint2) = src[m * PICTURE_X + i].at<Vec3b>(Point(x, y));
                            }
                        }
                    }
                }
            }
        }
    }

    //medianBlur(result[0], result[0], 3);
    //GaussianBlur(result[0], result[0], Size(3, 3), 0);
    imshow("result", result);
    imwrite("./result.jpg", result);
    setMouseCallback("result", onMouse, 0);

    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;

    printf("Make fisheye IMG: %f초 \n", duration);
    waitKey();
}

void reverse(Mat* src, int size) {

    //역방향 미완성 
    clock_t start, finish;
    double duration;
    start = clock();
    Point2f po;
    float fx = 0.53;  
    float fy = 0.53; 
    float u, v;
    float cx = (src[0].size().width - 1) / 2.0;
    float cy = (src[0].size().height - 1) / 2.0;
    float a, b, c, new_p, new_t;
    Mat result;

    result.create(2 * CIRCLE_R, 2 * CIRCLE_R, src[0].type());
    result = Mat::zeros(2 * CIRCLE_R, 2 * CIRCLE_R, src[0].type());


    static float R = ((float)(src[0].size().width)) / tanf((float)RADIAN(PAN / 2.0));
    fx = fx * R;
    fy = fy * R;
    float a2, b2, c2, theta2;
    int m2, k2;
    float ptzX, ptzY, wx, wy, wz;
    float _cos, _cos2, _sin, _sin2;
    for (int i = 0; i < result.size().height; i++) {
        for (int j = 0; j < result.size().width; j++) {
            k2 = 0;
            m2 = 0;
            double theta, r, w, tAngle;
            tAngle = CIRCLE_R / (107.0); //반지름을 TILT의 각도로 1º에 해당하는 픽셀값을 추출
            r = CIRCLE_R;
            a = j - r + 0.5; //0~Wide의 값에 r을 빼주어 -r ~ +r의 범위로 조정 (Wide = Height = 2*r)
            b = i - r + 0.5;

            theta = atan2(a, b) * 180.0 / PI; //theta 즉, Pan 값을 -180º ~ 180º으로 추출
            theta = theta + 180;
            w = sqrt(pow(a, 2) + pow(b, 2)); //피타고라스의 법칙을 사용하여 원의 중심으로 부터의 거리를 구함
            tAngle = (w / tAngle); // 위에서 구한 것을 tAngle로 나누어 Tilt값 0º ~ 107º를 추출
            tAngle = tAngle;



            m2 = ((tAngle) * 100) / 2675;
            if (tAngle < 107) {

                k2 = theta / 45;
                float ct = (theta - 22.25 - k2 * 45.0) * cos(RADIAN(90 - tAngle));

                if (ct < 0) {
                    ct = -ct;
                }

                float cc = cy + fy * tan(RADIAN(tAngle - 9.75 - m2 * 26.75));
                float d = sqrt(fy * fy + (cc - cy) * (cc - cy));
                float dd = cx + d * tan(RADIAN(theta - 22.25 - k2 * 45.0));

                _cos = cos(RADIAN(APPLY_WIDE * k2)) * 1000;
                _cos2 = cos(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m2)))) * 1000;
                _sin = sin(RADIAN(APPLY_WIDE * k2)) * 1000;
                _sin2 = sin(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m2)))) * 1000;



                u = (cc - cx) / fx;
                v = (dd - cy) / fy;
                wx = _cos / 1000.0 * u - _sin / 1000.0 * _sin2 / 1000.0 * v - _sin / 1000.0 * _cos2 / 1000.0;
                wy = _sin / 1000.0 * u + _cos / 1000.0 * _sin2 / 1000.0 * v + _cos / 1000.0 * _cos2 / 1000.0;
                wz = 0 - _cos2 / 1000.0 * v + _sin2 / 1000.0;
                new_p = -atan2(wx, wy) * 180.0 / PI;
                new_t = atan2(wz, sqrt(wx * wx + wy * wy)) * 180.0 / PI;
                new_t = 90.0 + new_t;
                
                d = sqrt(u * u + v * v + 1 * 1);
                wx = sin(RADIAN(tAngle)) * cos(RADIAN(theta)) * sqrt(d);
                wy = sin(RADIAN(tAngle)) * sin(RADIAN(theta)) * sqrt(d);
                wz = cos(RADIAN(tAngle)) * sqrt(d);
                theta2 = atan2(wy, wx) * 180.0 / PI;

                theta2 = theta2 + 180;
                k2 = theta2 / 45;
                k2 = k2 - 1;
                if (k2 == -1) {
                    k2 = 7; 
                }
                
                if (tAngle <= (107.0) && theta <= 360) { //원의 범위에 있다면

                    a2 = wx * cos(RADIAN(APPLY_WIDE * k2 - (APPLY_WIDE / 2.0))) + wy * sin(RADIAN(APPLY_WIDE * k2 - (APPLY_WIDE / 2.0)));
                    b2 = (-wx) * sin(RADIAN(APPLY_WIDE * k2 - (APPLY_WIDE / 2.0))) * sin(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m2)))) + wy * cos(RADIAN(APPLY_WIDE * k2 - (APPLY_WIDE / 2.0))) * sin(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m2)))) - wz * cos(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m2))));
                    c2 = (-wx) * sin(RADIAN(APPLY_WIDE * k2 - (APPLY_WIDE / 2.0))) * cos(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m2)))) + wy * cos(RADIAN(APPLY_WIDE * k2 - (APPLY_WIDE / 2.0))) * cos(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m2)))) + wz * sin(RADIAN(-(APPLY_HEIGHT * ((PICTURE_Y - 1) - m2))));
                    ptzX = a2 * fx + cx;
                    ptzY = b2 * fy + cy;




                    Point2f fisheyePoint2;
                    double r = CIRCLE_R;



                    if (ptzY >= 0 && ptzX >= 0 && m2 < 4 && k2 < 8 && ptzY < src[0].size().height && ptzX < src[0].size().width) {

                        result.at<Vec3b>(Point(j, i)) = src[m2 * PICTURE_X + k2].at<Vec3b>(Point(ptzX,ptzY));

                    }
                }


            }
        }
    }
    Mat kkk;
    imshow("result2", result);
    imwrite("./result.jpg", result);
    setMouseCallback("result2", onMouse, 0);

    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;

    printf("Make fisheye IMG: %f초 \n", duration);
    waitKey();
}




int main() {

    String path("./images/*.jpg"); //이미지 경로
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
    panorama();
    MakeFisheye(src, size); //정방향
    ///reverse(src, size);    //역방향


    return 0;
}
