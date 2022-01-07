#include <opencv2/opencv.hpp>
#include <math.h>
#include <time.h>

const int WIDE = 320;
const int HEIGHT = 180;
const int PICTURE_X = 8; // The number of x-axis photos.
const int PICTURE_Y = 4; // The number of y-axis photos.
const float PAN = 58.2f; // CAMERA FOV's WIDE 
const float TILT = 34.0f; // CAMERA FOV's HEIGHT
const float APPLY_WIDE = 45.0f; // 0~360, APPLY_WIDE<=FOV_WIDE
const float APPLY_HEIGHT = 26.75f; // 0~90, APPLY_HEIGHT<=FOV_HEIGHT
const float START_ANGLE = 6.6f; // From 58.2 to 45, 6.6 starts to 51.6 ends.
const float END_ANGLE = 51.6f;
const float PI = 3.1415926f;
const float CIRCLE_R = 500.0f; // FIsheye image's radius, The higher the size, the slower the speed.
const int FLAG = 1; // The flag is 0: default ,The flag is 1: The highter the graphic but the slower the speed.  

using namespace cv;
using namespace std;

void findMousePoint(int x, int y) {
	Mat img;
	img = imread("./result3/fisheye3_6.jpg", IMREAD_COLOR);
	double theta, r, w, tilt;
	tilt = (img.size().height / 2) / (90.0 + TILT / 2.0);
	r = img.size().height / 2;
	x = x - r;
	y = y - r;
	w = sqrt(pow(x, 2) + pow(y, 2));
	theta = atan2(y, x) * 180 / PI;

	printf("%f %f \n", theta, w / tilt);
}

void onMouse(int event, int x, int y, int flags, void* param) {
	switch (event) {
	case EVENT_LBUTTONDOWN:
		findMousePoint(x, y);
		break;
	}
}



Point2f matchFisheyePoint(int x, int y, double R, double cfx, double cfy, double H, double W) {   //i 와이드 j 높이,W둘레
	Point2f fisheyePoint;

	double theta, r, Xf, Yf;
	r = y;
	theta = (x / W * 2.0 * PI) - ((90.0 - (APPLY_WIDE / 2)) / 180.0) * PI;
	Xf = cfx + r * sin(theta);
	Yf = cfy + r * cos(theta);
	if (FLAG == 0) {
		fisheyePoint.x = Xf;
		fisheyePoint.y = Yf;
	}
	else if (FLAG == 1) {
		fisheyePoint.x = Xf / 2;
		fisheyePoint.y = Yf / 2;
	}

	return fisheyePoint;
}

Point2f carculateCos(int i, int j, double w, double h) {   //i 와이드 j 높이,W둘레
	Point2f fisheyePoint;

	double Xf;
	int k;
	double fi, fj, ow, ogh;

	fi = i;
	w = w / PICTURE_X;
	ow = w;
	k = i / w;
	ogh = h / 4;
	h = h / PICTURE_Y * (PICTURE_Y - 1);

	if (i > w) {
		fi = i - k * w;
	}
	if (j >= h) {
		fisheyePoint.x = i;
		fisheyePoint.y = j;
		return fisheyePoint;
	}
	if (i == 5891 && j == 726)
	{
		printf("dsads");
	}
	fj = -j;
	fj = fj + h + h * (975.0 / 9025.0);

	w = (int)abs(w - (fi * 2)) / 1;
	h = h + h * (975.0 / 9025.0);
	Xf = (ow / 2) - cos(fj / h * PI / 2.0) * (w / 2);


	/*if (j <= ogh && fi >= ow / 2) {
		Xf = (ow-((y * ((ow / 2) - straightLine) / ogh)+straightLine)) + k*ow ;
		fisheyePoint.x = Xf;
		fisheyePoint.y = j;

		return fisheyePoint;
	}
	else if (j<= ogh)	{
		Xf = (y * ((ow / 2) - straightLine) / ogh) + straightLine+k*ow;
		fisheyePoint.x = Xf;
		fisheyePoint.y = j;

		return fisheyePoint;
	}*/


	/*if (fi > ow / 2) {
		Xf =  fi - Xf + k * ow;
	}
	else {
		Xf = fi + Xf + k * ow;
	}*/
	if (fi > ow / 2) {
		Xf = (ow - Xf) + k * ow;
	}
	else {
		Xf = Xf + k * ow;
	}
	if (Xf >= 6282)
	{
		Xf = Xf - 1;
	}
	fisheyePoint.x = Xf;
	fisheyePoint.y = j;


	return fisheyePoint;
}

void callImg(Mat* src, int size, vector<String> str)
{
	clock_t start, finish;
	double duration;
	start = clock();
	const char* c;

	for (int i = 0; i < size; i++) {
		c = str[i].c_str();
		src[i] = imread(c);
	}

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("Call IMG: %f초 \n", duration);
}
void makePanorama2(Mat* src, int size)
{

	clock_t start, finish;
	double duration;
	start = clock();

	for (int i = 0; i < size / PICTURE_X; i++) {
		for (int j = 1; j < size / PICTURE_Y; j++) {
			hconcat(src[i * PICTURE_X], src[(i * PICTURE_X) + j], src[i * PICTURE_X]);
		}
		if (i > 0) {
			vconcat(src[0], src[i * PICTURE_X], src[0]);
		}
	}

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("Make Panorama: %f초 \n", duration);
	imwrite("./result3/origin.jpg", src[0]);


}


void makePanorama(Mat* src, int size)
{

	clock_t start, finish;
	double duration;
	start = clock();
	for (int i = 0; i < size; i++) {
		resize(src[i], src[i], Size(WIDE, HEIGHT));
		if (i >= 0 && i < PICTURE_X) {
			src[i] = src[i](Range(HEIGHT / TILT * (TILT - APPLY_HEIGHT), HEIGHT - 1), Range(WIDE / PAN * START_ANGLE, WIDE / PAN * END_ANGLE));
		}
		else {
			src[i] = src[i](Range(HEIGHT / TILT * (TILT - APPLY_HEIGHT), HEIGHT - 1), Range(WIDE / PAN * START_ANGLE, WIDE / PAN * END_ANGLE));
		}
	}

	for (int i = 0; i < size / PICTURE_X; i++) {
		for (int j = 1; j < size / PICTURE_Y; j++) {
			hconcat(src[i * PICTURE_X], src[(i * PICTURE_X) + j], src[i * PICTURE_X]);
		}
		if (i > 0) {
			vconcat(src[0], src[i * PICTURE_X], src[0]);
		}
	}

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("Make Panorama: %f초 \n", duration);
	imwrite("./result3/eye.jpg", src[0]);


}

void makeFisheye(Mat* src)
{

	Mat origin, showcase, result;
	clock_t start, finish;
	double duration;
	start = clock();
	origin = src[0];

	double H, W, R, cfx, cfy, We;

	R = CIRCLE_R;
	if (FLAG == 1) {
		R = R * 2;
	}
	W = 2 * PI * R;
	resize(origin, origin, Size(W, R));

	H = origin.size().height;
	W = origin.size().width;

	cfx = R;
	cfy = R;
	We = (int)2 * PI * R;
	showcase.create(origin.size().height, origin.size().width, origin.type());
	showcase = Mat::zeros(origin.size().height, origin.size().width, origin.type());

	if (FLAG == 1) {
		R = R / 2;
	}
	result.create(R * 2 + 1, R * 2 + 1, origin.type());
	result = Mat::zeros(R * 2 + 1, R * 2 + 1, origin.type());

	for (int i = 0; i < origin.size().width; i++) {
		for (int j = 0; j < origin.size().height; j++) {
			result.at<Vec3b>(matchFisheyePoint(i, j, R, cfx, cfy, R, We)) = origin.at<Vec3b>(carculateCos(i, j, origin.size().width, origin.size().height));
			showcase.at<Vec3b>(carculateCos(i, j, origin.size().width, origin.size().height)) = origin.at<Vec3b>(carculateCos(i, j, origin.size().width, origin.size().height));
		}
	}

	imshow("show", showcase);
	imwrite("./result3/showcase_2.jpg", showcase);

	Mat acd;
	if (FLAG == 0) {
		medianBlur(result, result, 3);
	}
	//bilateralFilter(result, acd, 6, 25, 25);
	//resize(result, result, Size(800, 800));
	imshow("asdd", result);
	imwrite("./result3/fisheye3_7.jpg", result);

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("Make Fisheye image: %f초 \n", duration);
	setMouseCallback("asdd", onMouse, 0);
	waitKey();

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


Point2f caculPoint(Mat img,Mat origin,int x,int y) {
	Point2f fisheyePoint;

	double theta, r, w, tilt,Xf,w2,ow;
	tilt = (img.size().height / 2) / (90.0 + TILT / 2.0);
	r = img.size().height / 2;
	int k,k2;
	x = x - r;
	y = y - r;

	double h = origin.size().height;
	h = h / PICTURE_Y * (PICTURE_Y - 1);
	w = sqrt(pow(x, 2) + pow(y, 2));
	w2 = origin.size().width;
	w2 = w2 / PICTURE_X;
	ow = w2;
	theta = atan2(y, x) * 180.0 / PI;
	theta -= 180;
	theta = -theta;
	tilt = w / tilt;
	double fi = origin.size().width / 360.0 * theta;
	k = fi / w2;
	
	if (fi > w) {
		fi = fi - k * w2;
	}

	double fj = origin.size().height / (90.0 + TILT / 2.0) * tilt;
	w2 = (int)abs(w2 - (fi * 2)) / 1;
	
	if (tilt<= (90.0 + TILT / 2.0) && theta <= 360)
	{
		if (tilt < ((90.0 + TILT / 2.0) - 26.75))
		{
			fj = -fj;
			//fj = fj + h;
			fj = fj + h + h * (975.0 / 9025.0);
			h = h + h * (975.0 / 9025.0);
			Xf = (ow / 2) - cos(fj / h * PI / 2.0) * (w2 / 2);

			//Xf = (asin(fj / h) / PI) * w2;
			if (fi > ow / 2) {
				Xf = (ow - Xf) + k * ow;
			}
			else {
				Xf = Xf + k * ow;
			}
			/*if (fi > ow / 2) {
				Xf = fi - Xf + k * ow;
			}
			else {
				Xf = fi + Xf + k * ow;
			}
			*/
			fisheyePoint.x = Xf;
			fisheyePoint.y = origin.size().height / (90.0 + TILT / 2.0) * tilt;
		}
		else {
			fisheyePoint.x = (origin.size().width) / 360.0 * theta;
			fisheyePoint.y = (origin.size().height-1) / (90.0 + TILT / 2.0) * tilt;
		}
	}
	else {
		return 0;
	}


	return fisheyePoint;
}

void fisheyetoP(Mat* src)
{
	Mat origin, showcase, result;
	clock_t start, finish;
	double duration;
	start = clock();
	origin = src[0];

	double H, W, R, cfx, cfy, We;

	R = CIRCLE_R;
	W = 2 * PI * R;
	resize(origin, origin, Size(W, R));

	H = origin.size().height;
	W = origin.size().width;
	//imshow("asdf", origin);
	//waitKey();
	cfx = R;
	cfy = R;
	We = (int)2 * PI * R;
	showcase.create(origin.size().height, origin.size().width, origin.type());
	showcase = Mat::zeros(origin.size().height, origin.size().width, origin.type());
	result.create(R * 2 , R * 2 , origin.type());
	result = Mat::zeros(R * 2 , R * 2, origin.type());
	
	for (int i = 0; i < result.size().width; i++)
	{
		for (int j = 0; j < result.size().height; j++)
		{
			result.at<Vec3b>(Point(i, j)) = origin.at<Vec3b>(caculPoint(result,origin,i,j));
		}
	}

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("Make Panorama: %f초 \n", duration);
	imshow("sad", result);
	imwrite("./result3/kkk.jpg", result);
	waitKey();
	 
}


int main(int argc, char** argv)
{
	String path("./image3/*.jpg");
	vector<String> str;
	glob(path, str, false);

	Mat src[100];
	Mat result;
	int size = str.size();

	if (str.size() == 0) {
		cout << "파일이 존재하지 않습니다." << endl;

		return 0;
	}
	else {
		cout << "파일 갯수:" << str.size() << endl;
	}


	callImg(src, size, str);
	makePanorama(src, size);
	fisheyetoP(src);


	return 0;
}