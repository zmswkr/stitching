#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;
#define WIDE 480
#define HEIGHT 360

Mat wraping(Mat src)
{
    vector<Point2f> corners(4);
    /*corners[0] = Point2f(320, 540);
    corners[1] = Point2f(1600, 0);
    corners[2] = Point2f(80, 1080);
    corners[3] = Point2f(1840, 1080);*/
    corners[0] = Point2f(0, 0);
    corners[1] = Point2f(1920, 0);
    corners[2] = Point2f(0, 1080);
    corners[3] = Point2f(1920, 1080);

    Size warpSize(WIDE, HEIGHT);

    Mat warpImg(warpSize, src.type());


    vector<Point2f> warpCorners(4);
    warpCorners[0] = Point2f(0, 0);
    warpCorners[1] = Point2f(warpImg.cols, 0);
    warpCorners[2] = Point2f(0, warpImg.rows);
    warpCorners[3] = Point2f(warpImg.cols-0, warpImg.rows);


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
    String path("*.jpg");

    vector<String> str;

    glob(path, str, false);



    if (str.size() == 0)
    {
        cout << "파일이 존재하지 않습니다." << endl;
    }
    else {
        cout << "파일 갯수:" << str.size() << endl;
    }

    Mat src[6];
    IplImage* src2[6];
    IplImage* dst[6];
    IplImage* dsta = NULL;
    const char* c;
    int size = str.size();


    for (int i = 0; i < size; i++)
    {
        c = str[i].c_str();
        src[i] = imread(c);
    }
    src[0] = wraping(src[0]);
    src[2] = wraping(src[2]);

    for (int i = 0; i < size; i++)
    {
        src2[i] = new IplImage(src[i]);
    }

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            dst[(3 * i) + j] = cvCreateImage(cvSize(WIDE, HEIGHT), IPL_DEPTH_8U, 3);
            cvResize(src2[(3 * i) + j], dst[(3 * i) + j], CV_INTER_LINEAR);
            if (i == 0 && j == 0)
            {
                dsta = cvCreateImage(cvSize(WIDE * 3, HEIGHT * 2), dst[0]->depth, dst[0]->nChannels);
            }
            cvSetImageROI(dsta, cvRect(WIDE * j, HEIGHT * i, dst[(3 * i) + j]->width, dst[(3 * i) + j]->height));
            cvCopy(dst[(3 * i) + j], dsta);
        }
    }


    cvResetImageROI(dsta);
    cvNamedWindow("Merge", CV_WINDOW_AUTOSIZE);
    cvShowImage("Merge", dsta);
    waitKey();

    return 0;
}