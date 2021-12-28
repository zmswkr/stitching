#include <opencv2/opencv.hpp>
#include <math.h>
#include <time.h>

#define WIDE 320
#define HEIGHT 180
#define PAN 58.2
#define TILT 34.0
#define PI 3.1415926


using namespace cv;
using namespace std;

void findMousePoint(int x, int y) {
	Mat img;
	img = imread("./result3/fisheye3.jpg", IMREAD_COLOR);
	double theta, R, w,t;
	t = (img.size().height / 2)/(90.0+TILT/2.0);
	R = img.size().height / 2;
	x = x - R;
	y = y - R;
	w = sqrt(pow(x, 2) + pow(y, 2));
	theta = atan2(y, x) * 180 / PI;

	printf("%f %f \n", theta, w / t);
}

void onMouse(int event, int x, int y, int flags, void* param) {
	switch (event) {
	case EVENT_LBUTTONDOWN:
		findMousePoint(x, y);
		break;
	}
}



Point2f matchFisheyePoint(int i, int j, double R, double cfx, double cfy, double H, double W) {   //i 와이드 j 높이,W둘레
	Point2f fisheyePoint;

	double theta, r, Xf, Yf;

	r = j;
	theta = (i / W * 2.0 * PI) - (67.5/180.0) * PI;
	Xf = cfx + r * sin(theta);
	Yf = cfy + r * cos(theta);
	fisheyePoint.x = Xf;
	fisheyePoint.y = Yf;


	return fisheyePoint;
}





Mat wraping(Mat src, int a, int W, int H) {
	vector<Point2f> corners(4);

	double co4 = W;
	double co1 = cos(37 * PI / 180);
	double ans = W / ((1 / co1) * co4);
	ans = 1 - ((1 - ans) / 2);
	double co3 = cos(64 * PI / 180);
	double ans2 = W / ((1 / co3) * co4);
	ans2 = 1 - ((1 - ans2) / 2);                                                                                              
	double aa = (W * (6.6 / 58.2)) + (W * (45.0 / 58.2) * ans);
	double aa2 = (W * (6.6 / 58.2)) + (W * (45.0 / 58.2) * ans2);

	if (a == 2) {
		corners[0] = Point2f(W - aa, H * (7.0 / 34.0));
		corners[1] = Point2f((W * (6.6 / 58.2)) + (W * (45.0 / 58.2) * ans), H * (7.0 / 34.0));
		corners[2] = Point2f(W * (6.6 / 58.2), H);
		corners[3] = Point2f(W * (51.6 / 58.2), H);
	}
	else if (a == 1) {
		corners[0] = Point2f(W - aa2, H * (7.0 / 34.0));
		corners[1] = Point2f( aa2 , H * (7.0 / 34.0));
		corners[2] = Point2f(W - aa, H);
		corners[3] = Point2f(aa, H);
	}
	else if (a == 3)
	{
		corners[0] = Point2f(W * (6.6 / 58.2), H * (7.0 / 34.0));
		corners[1] = Point2f(W * (51.6 / 58.2), H * (7.0 / 34.0));
		corners[2] = Point2f(W * (6.6 / 58.2), H);
		corners[3] = Point2f(W * (51.6 / 58.2), H);
	}
	else {
		corners[0] = Point2f(W / 2.0 - 80, H * (7.0 / 34.0));
		corners[1] = Point2f(W / 2.0 + 80, H * (7.0 / 34.0));
		corners[2] = Point2f(W - aa2, H);
		corners[3] = Point2f(aa2, H);
	}

	vector<Point2f> warpCorners(4);


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



int main(int argc, char** argv) {
	String path("./image3/*.jpg");
	vector<String> str;
	clock_t start, finish;

	start = clock();
	glob(path, str, false);

	if (str.size() == 0) {
		cout << "파일이 존재하지 않습니다." << endl;
	}
	else {
		cout << "파일 갯수:" << str.size() << endl;
	}

	Mat src[32];
	Mat resu;
	const char* c;
	int size = str.size();


	for (int i = 0; i < size; i++) {
		c = str[i].c_str();
		src[i] = imread(c);
	}


	for (int i = 0; i < size; i++) {
		src[i] = wraping(src[i], i / (size / 4), src[i].size().width, src[i].size().height);
	}


	for (int i = 0; i < size / 8; i++) {
		for (int j = 1; j < size / 4; j++) {
			hconcat(src[i * 8], src[(i * 8) + j], src[i * 8]);
		}
		if (i > 0) {
			vconcat(src[0], src[i * 8], src[0]);
		}
	}

	imwrite("./result3/eye.jpg", src[0]);
	Mat origin;
	origin = imread("./result3/eye.jpg", IMREAD_COLOR);
	imshow("fisheye", origin);
	imwrite("./result3/origin.jpg", origin);
	resize(origin, origin, Size(4398, 700));
	double H, W;
	double R, cfx, cfy;
	double We;

	H = origin.size().height;
	W = origin.size().width;
	R = W / PI / 2.0;
	
	cfx = R;
	cfy = R;
	We = (int)2 * PI * R;
	resu.create(R * 2 + 1, R * 2 + 1, origin.type()); 

	for (int i = 0; i < origin.size().width; i++) {
		for (int j = 0; j < origin.size().height; j++) {
			resu.at<Vec3b>(matchFisheyePoint(i, j, R, cfx, cfy, R, We)) = origin.at<Vec3b>(Point(i, j));
		}
	}

	Mat acd;	
	medianBlur(resu, resu, 3);
	//GaussianBlur(resu, resu, Size(7, 7), 0);

	resize(resu, resu, Size(800, 800));
	bilateralFilter(resu, acd, 6, 25, 25);
	imshow("resu", acd);
	//imshow("aaa", acd);  
	imwrite("./result3/fisheye3.jpg", resu);
	
	finish = clock();
	double duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("%f초 \n", duration);
	setMouseCallback("resu", onMouse, 0);

	Mat kernel, kernel2, filter_img, filter_img2;

	
	waitKey();
	return 0;
}