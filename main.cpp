#include <opencv2/opencv.hpp>
#include <math.h>
#include <time.h>


const int PICTURE_X = 8; // The number of x-axis photos.
const int PICTURE_Y = 4; // The number of y-axis photos.
const float PAN = 58.2f; // CAMERA FOV's WIDE 
const float TILT = 34.0f; // CAMERA FOV's HEIGHT
const float APPLY_WIDE = 45.0f; // 0~360, APPLY_WIDE<=FOV_WIDE
const float APPLY_HEIGHT = 26.75f; // 0~90, APPLY_HEIGHT<=FOV_HEIGHT
const float PI = 3.1415926f;
const float CIRCLE_R = 500.0f; // FIsheye image's radius, The higher the size, the slower the speed.

using namespace cv;
using namespace std;

void findMousePoint(int x, int y) {
	Mat img;
	img = imread("./result3/kkk.jpg", IMREAD_COLOR);
	double theta, r, w, tilt;
	tilt = (img.size().height / 2) / (90.0 + TILT / 2.0);
	r = img.size().height / 2;
	x = x - r;
	y = y - r;
	w = sqrt(pow(x, 2) + pow(y, 2));
	theta = atan2(y, x) * 180 / PI;
	theta = theta - (APPLY_WIDE / 2.0 + 90);
	if (theta < -180)
	{
		theta = theta + 360;
	}
	if (theta > +180)
	{
		theta = theta - 360;
	}
	printf("%f %f \n", theta, w / tilt);
}

void onMouse(int event, int x, int y, int flags, void* param) {
	switch (event) {
	case EVENT_LBUTTONDOWN:
		findMousePoint(x, y);
		break;
	}
}


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
	printf("Call IMG: %fsec \n", duration);
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
		src[i] = src[i](Range(src[i].size().height / TILT * (TILT - APPLY_HEIGHT), src[i].size().height - 1),
			Range(src[i].size().width / PAN * ((PAN - APPLY_WIDE) / 2.0), src[i].size().width / PAN * (PAN - (PAN - APPLY_WIDE) / 2.0)));
		// 사진을 찍은 PAN, TILT의 각도를 기존의 화각에서 그 각도만큼 추출
		// ex) Tilt: 34.0 ~ (34-26.75) = 34.0 ~ 7.25  Pan: (58.2-45)/2 ~ (58.2-(58.2-45))/2 = 6.6 ~ 51.2          
		resize(src[i], src[i], Size((CIRCLE_R * 2 * PI) / PICTURE_X + 1, CIRCLE_R / PICTURE_Y + 1));
	}


	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("Crop overlap IMG: %fsec \n", duration);
	imwrite("./result3/eye.jpg", src[0]);


}




Point2f caculPoint(Mat* src, double x, double y) {
	Point2f fisheyePoint;

	double theta, r, w, tAngle, Xf, ow;
	tAngle = CIRCLE_R / (90.0 + TILT / 2.0); //반지름을 TILT의 각도로 1º에 해당하는 픽셀값을 추출
	r = CIRCLE_R;
	int k;
	x = x - r - 0.5; //0~Wide의 값에 r을 빼주어 -r ~ +r의 범위로 조정 (Wide = Height = 2*r)
	y = y - r - 0.5;

	double h = CIRCLE_R;
	h = h / PICTURE_Y * (PICTURE_Y - 1); //Y축의 맨아랫줄 즉, TILT가 90º일 때를 제외한 Height
	ow = src[0].size().width;
	theta = atan2(x, y) * 180.0 / PI; //theta 즉, Pan 값을 -180º ~ 180º으로 추출
	theta = theta + 180.0;
	w = sqrt(pow(x, 2) + pow(y, 2)); //피타고라스의 법칙을 사용하여 원의 중심으로 부터의 거리를 구함
	tAngle = w / tAngle; // 위에서 구한 것을 tAngle로 나누어 Tilt값 0º ~ 107º를 추출
	double size = src[1].size().width * PICTURE_X;
	double fi = size / 360.0 * theta; // fisheye의 x좌표에 해당하는 파라노마의 x좌표
	double fj = CIRCLE_R / (90.0 + TILT / 2.0) * tAngle; // y좌표

	k = fi / ow;
	if (fi > w) {
		fi = fi - k * ow;
	}

	if (tAngle <= (90.0 + TILT / 2.0) && theta <= 360) //원의 범위에 있다면
	{
		if (tAngle < ((90.0 + TILT / 2.0) - 26.75)) // 틸트 90도의 축이 아니라면
		{
			fj = -fj;
			fj = fj + h + h * ((APPLY_HEIGHT - TILT / 2.0) / (100.0 - (APPLY_HEIGHT - TILT / 2.0)));
			h = h + h * ((APPLY_HEIGHT - TILT / 2.0) / (100.0 - (APPLY_HEIGHT - TILT / 2.0)));
			Xf = (ow / 2.0) - cos(fj / h * PI / 2.0) * (ow / 2.0 - fi);

			//Xf = (asin(fj / h) / PI) * w2;

			fisheyePoint.x = Xf + k * ow;
			fisheyePoint.y = CIRCLE_R / (90.0 + TILT / 2.0) * tAngle;
		}
		else {
			fisheyePoint.x = size / 360.0 * theta;
			fisheyePoint.y = CIRCLE_R / (90.0 + TILT / 2.0) * tAngle;
		}
	}
	else {
		fisheyePoint.x = 0;
		fisheyePoint.y = 0;
		return fisheyePoint;
	}


	return fisheyePoint;
}

void makeFisheye(Mat* src)
{
	Mat origin, result;
	Point2f Point2;
	clock_t start, finish;
	double duration;
	start = clock();
	origin = src[0];
	int x, y;
	double W, R;

	R = CIRCLE_R;
	W = 2 * PI * R;
	//resize(origin, origin, Size(W, R)); //합쳐준 사진을 WIDE를 원둘레 Height를 반지름에 맞게 설정

	result.create(R * 2, R * 2, origin.type());
	result = Mat::zeros(R * 2, R * 2, origin.type());

	for (int i = 0; i < result.size().width; i++)
	{
		for (int j = 0; j < result.size().height; j++)
		{
			x = 0; y = 0;
			if (i > 146 && j > 146)
			{
				//printf("j:%d \n", j);
				//printf("%d %d", src[1].size().width, src[1].size().height);
			}
			Point2 = caculPoint(src, i, j);
			if (Point2.x > (src[1].size().width))
			{
				x = Point2.x / (src[1].size().width);
				Point2.x = Point2.x - ((src[1].size().width) * x);
			}
			if (Point2.y > (src[1].size().height))
			{
				y = Point2.y / (src[1].size().height);
				Point2.y = Point2.y - ((src[1].size().height) * y);
			}
			Point2.x = Point2.x / src[1].size().width * (src[1].size().width - 1);
			Point2.y = Point2.y / src[1].size().height * (src[1].size().height - 1);
			result.at<Vec3b>(Point(i, j)) = src[(PICTURE_X * y + x)].at<Vec3b>(Point2);
		}
	}
	//imshow("sad", result);
	imwrite("./result3/kkk.jpg", result);
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("Make Fisheye IMG: %fsec \n", duration);
	//setMouseCallback("sad", onMouse, 0);

	//waitKey();

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
	makeFisheye(src);

	return 0;
}