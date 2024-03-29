#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include <opencv2/opencv.hpp>

using namespace cv;

typedef struct {
	int r, g, b;
}int_rgb;


int** IntAlloc2(int height, int width)
{
	int** tmp;
	tmp = (int**)calloc(height, sizeof(int*));
	for (int i = 0; i < height; i++)
		tmp[i] = (int*)calloc(width, sizeof(int));
	return(tmp);
}

void IntFree2(int** image, int height, int width)
{
	for (int i = 0; i < height; i++)
		free(image[i]);

	free(image);
}

float** FloatAlloc2(int height, int width)
{
	float** tmp;
	tmp = (float**)calloc(height, sizeof(float*));
	for (int i = 0; i < height; i++)
		tmp[i] = (float*)calloc(width, sizeof(float));
	return(tmp);
}

void FloatFree2(float** image, int height, int width)
{
	for (int i = 0; i < height; i++)
		free(image[i]);

	free(image);
}

int_rgb** IntColorAlloc2(int height, int width)
{
	int_rgb** tmp;
	tmp = (int_rgb**)calloc(height, sizeof(int_rgb*));
	for (int i = 0; i < height; i++)
		tmp[i] = (int_rgb*)calloc(width, sizeof(int_rgb));
	return(tmp);
}

void IntColorFree2(int_rgb** image, int height, int width)
{
	for (int i = 0; i < height; i++)
		free(image[i]);

	free(image);
}

int** ReadImage(char* name, int* height, int* width)
{
	Mat img = imread(name, IMREAD_GRAYSCALE);
	int** image = (int**)IntAlloc2(img.rows, img.cols);

	*width = img.cols;
	*height = img.rows;

	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++)
			image[i][j] = img.at<unsigned char>(i, j);

	return(image);
}

void WriteImage(char* name, int** image, int height, int width)
{
	Mat img(height, width, CV_8UC1);
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img.at<unsigned char>(i, j) = (unsigned char)image[i][j];

	imwrite(name, img);
}


void ImageShow(char* winname, int** image, int height, int width)
{
	Mat img(height, width, CV_8UC1);
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			img.at<unsigned char>(i, j) = (unsigned char)image[i][j];
	imshow(winname, img);
	waitKey(0);
}



int_rgb** ReadColorImage(char* name, int* height, int* width)
{
	Mat img = imread(name, IMREAD_COLOR);
	int_rgb** image = (int_rgb**)IntColorAlloc2(img.rows, img.cols);

	*width = img.cols;
	*height = img.rows;

	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++) {
			image[i][j].b = img.at<Vec3b>(i, j)[0];
			image[i][j].g = img.at<Vec3b>(i, j)[1];
			image[i][j].r = img.at<Vec3b>(i, j)[2];
		}

	return(image);
}

void WriteColorImage(char* name, int_rgb** image, int height, int width)
{
	Mat img(height, width, CV_8UC3);
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++) {
			img.at<Vec3b>(i, j)[0] = (unsigned char)image[i][j].b;
			img.at<Vec3b>(i, j)[1] = (unsigned char)image[i][j].g;
			img.at<Vec3b>(i, j)[2] = (unsigned char)image[i][j].r;
		}

	imwrite(name, img);
}

void ColorImageShow(char* winname, int_rgb** image, int height, int width)
{
	Mat img(height, width, CV_8UC3);
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++) {
			img.at<Vec3b>(i, j)[0] = (unsigned char)image[i][j].b;
			img.at<Vec3b>(i, j)[1] = (unsigned char)image[i][j].g;
			img.at<Vec3b>(i, j)[2] = (unsigned char)image[i][j].r;
		}
	imshow(winname, img);

}

template <typename _TP>
void ConnectedComponentLabeling(_TP** seg, int height, int width, int** label, int* no_label)
{

	//Mat bw = threshval < 128 ? (img < threshval) : (img > threshval);
	Mat bw(height, width, CV_8U);

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++)
			bw.at<unsigned char>(i, j) = (unsigned char)seg[i][j];
	}
	Mat labelImage(bw.size(), CV_32S);
	*no_label = connectedComponents(bw, labelImage, 8); // 0까지 포함된 갯수임

	(*no_label)--;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++)
			label[i][j] = labelImage.at<int>(i, j);
	}
}

#define imax(x, y) ((x)>(y) ? x : y)
#define imin(x, y) ((x)<(y) ? x : y)

int BilinearInterpolation(double y, double x, int** image, int height, int width)
{
	int x_int = (int)x;
	int y_int = (int)y;

	int A = image[imin(imax(y_int, 0), height - 1)][imin(imax(x_int, 0), width - 1)];
	int B = image[imin(imax(y_int, 0), height - 1)][imin(imax(x_int + 1, 0), width - 1)];
	int C = image[imin(imax(y_int + 1, 0), height - 1)][imin(imax(x_int, 0), width - 1)];
	int D = image[imin(imax(y_int + 1, 0), height - 1)][imin(imax(x_int + 1, 0), width - 1)];

	double dx = x - x_int;
	double dy = y - y_int;

	double value
		= (1.0 - dx) * (1.0 - dy) * A + dx * (1.0 - dy) * B
		+ (1.0 - dx) * dy * C + dx * dy * D;

	return((int)(value + 0.5));
}


void DrawHistogram(char* comments, int* Hist)
{
	int histSize = 256; /// Establish the number of bins
	// Draw the histograms for B, G and R
	int hist_w = 512; int hist_h = 512;
	int bin_w = cvRound((double)hist_w / histSize);

	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(255, 255, 255));
	Mat r_hist(histSize, 1, CV_32FC1);
	for (int i = 0; i < histSize; i++)
		r_hist.at<float>(i, 0) = Hist[i];
	/// Normalize the result to [ 0, histImage.rows ]
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	/// Draw for each channel
	for (int i = 1; i < histSize; i++)
	{
		line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
			Point(bin_w * (i), hist_h - cvRound(r_hist.at<float>(i))),
			Scalar(255, 0, 0), 2, 8, 0);
	}

	/// Display
	namedWindow(comments, WINDOW_AUTOSIZE);
	imshow(comments, histImage);

	waitKey(0);

}

// for (int i = 0; i < 10; i++) <=> int i = 0; while (i < 10) { i++; }

// (if-참이면 실행 거짓이면 다음 조건 비교)
// (else if - 위에 조건이 거짓일 떄 비교)
// (else 위에 조건이 모두 거짓일 때 실행, 모두 거짓일 때 실행할 것이 있으면 사용)

/* 픽셀값 다루기 09-16  */

void EX0916_1()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"barbara.png", &height, &width);
	
	for (int y = 0; y < height; y++) {  //바로 안에 int 선언 하면 생략 가능
		for (int x = 0; x < width; x++) {
			if (img[y][x] >= 200) {  //밝은 곳은
				img[y][x] = 0;	//어둡게 만듬
			}
			else {
				img[y][x] = 255; //200미만
			}
			//printf("(%d, %d)", y, x);
			//printf("%d", img[y][x]); //밝기값
		}
	}
	//printf("\n height = %d, width = %d", height, width);
	ImageShow((char*)"test", img, height, width);

}

void EX0916_2(int** img, int height, int width)
{
	//int height, width;
	//int** img = (int**)ReadImage((char*)"barbara.png", &height, &width);

	for (int y = 0; y < height; y++) {  //바로 안에 int 선언 하면 생략 가능
		for (int x = 0; x < width; x++) {
			if (img[y][x] >= 200) {  //밝은 곳은
				img[y][x] = 0;	//어둡게 만듬
			}
			else {
				img[y][x] = 255; //200미만
			}
			//printf("(%d, %d)", y, x);
			//printf("%d", img[y][x]); //밝기값
		}
	}
	//printf("\n height = %d, width = %d", height, width);
	ImageShow((char*)"test", img, height, width);

}

int EX0916_3()
{
	int height, width;
	int** img0 = (int**)ReadImage((char*)"barbara.png", &height, &width);
	int** img1 = (int**)ReadImage((char*)"lena.png", &height, &width);

	EX0916_2(img1, height, width);

	return(0);
}

int EX0923_0() //원 만들기 (&&-AND)(||-OR)(!-NOT)
{
	int height, width;
	int** img = (int**)ReadImage((char*)"barbara.png", &height, &width);

	int x0 = 256; int y0 = 256; int r = 100;
	int x1 = 180; int y1 = 200;

	for (int y = 0; y < height; y++) {  //바로 안에 int 선언 하면 생략 가능
		for (int x = 0; x < width; x++) {
			if (img[y][x] >= 0) {
				img[y][x] = 0;
			}	//어둡게 만듬
			if ((y - y0) * (y - y0) + (x - x0) * (x - x0) <= r * r) {
				img[y][x] = 250;
			}
			if ((y - y0) * (y - y0) + (x - x1) * (x - x1) <= r * r) {
				img[y][x] = 100;
			}
			if (((y - y0) * (y - y0) + (x - x0) * (x - x0) <= r * r) && ((y - y0) * (y - y0) + (x - x1) * (x - x1) <= r * r)) {
				img[y][x] = 180;
			}
		}
	}
	ImageShow((char*)"test", img, height, width);
	return(0);
}

void Thresholding(int threshold, int** img, int height, int width, int** img_out) //영상이진화 실습3
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (img[y][x] >= threshold) {
				img_out[y][x] = 255;
			}
			if (img[y][x] <= threshold) {
				img_out[y][x] = 0;
			}
		}
	}
}

void Thresholding2(int threshold, int** img, int height, int width, int** img_out) //영상이진화 실습4
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (img[y][x] >= threshold) if (y <= height / 2 && x <= width / 2)
				img_out[y][x] = 255;
			else if (img[y][x] >= threshold) if (y <= height / 2 && x >= width / 2)
				img_out[y][x] = 180;
			else if (img[y][x] >= threshold) if (y >= height / 2 && x <= width / 2)
				img_out[y][x] = 120;
			else if (img[y][x] >= threshold) if (y >= height / 2 && x >= width / 2)
				img_out[y][x] = 60;
		}
	}
}

/* 영상혼합 09-23 */

int EX0923_01() //영상이진화 실습2
{
	int height, width;
	int** img = (int**)ReadImage((char*)"barbara.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);
	int threshold = 128;

	Thresholding2(threshold, img, height, width, img_out);

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", img_out, height, width);
	return(0);
}

void AddValue2Image(int value, int** img, int height, int width, int** img_out1);

void EX0923_1()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"barbara.png", &height, &width);
	int** img_out1 = (int**)IntAlloc2(height, width);
	int** img_out2 = (int**)IntAlloc2(height, width);

	// img_out1은 barbara영상에 밝기값을 +50 하기
	// img_out2은 barbara영상에 밝기값을 -50 하기

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			img_out1[y][x] = img[y][x] + 50; //세미콜론 하나 한 문장임
		}
	}
		
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			img_out2[y][x] = img[y][x] - 50;
		}
	}
	ImageShow((char*)"출력1영상보기", img_out1, height, width);
	ImageShow((char*)"출력2영상보기", img_out2, height, width);
}


/////////////////////////////

void AddValue2Image(int value, int** img, int height, int width, int** img_out1);

void EX0923_2()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"barbara.png", &height, &width);
	int** img_out1 = (int**)IntAlloc2(height, width);
	int** img_out2 = (int**)IntAlloc2(height, width);

	// img_out1은 barbara영상에 밝기값을 +50 하기
	// img_out2은 barbara영상에 밝기값을 -50 하기

	AddValue2Image(50, img, height, width, img_out1); //함수선언
	AddValue2Image(-50, img, height, width, img_out2); //함수선언

	ImageShow((char*)"출력1영상보기", img_out1, height, width);
	ImageShow((char*)"출력2영상보기", img_out2, height, width);
}

void AddValue2Image(
	int value, // 더해줄 값
	int** img, // 입력영상
	int height, // 영상의 높이
	int width, // 영상의 폭
	int** out_img //출력영상 이름 달라도 괜찮
) //함수 만들기
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			out_img[y][x] = img[y][x] + value; //세미콜론 하나 한 문장임
		}
	}
}

#define SQ(x) ((x)*(x))
#define GetMax(x, y) ((x>y) ? x : y)
#define GetMin(x, y) ((x<y) ? x : y)
#define Clipping(x) (GetMax(GetMin(x, 255), 0))

void ImageClipping(int height, int width, int** img_in, int** img_out)
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			//if (img_in[y][x] < 0)		img_out[y][x] = 0;			// if문을 이용한 클리핑
			//else if (img_in[y][x] > 255)img_out[y][x] = 255;
			//else						img_out[y][x] = img_in[y][x];
			
			//img_out[y][x] = GetMin(GetMin(img_in[y][x], 255), 0); // 매크로를 이용한 클리핑

			img_out[y][x] = Clipping(img_in[y][x]);					// 매크로를 이용한 클리핑 2
		}
	}
}

void EX0923_3()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"barbara.png", &height, &width);
	int** img_out1 = (int**)IntAlloc2(height, width);
	int** img_out2 = (int**)IntAlloc2(height, width);

	// img_out1은 barbara영상에 밝기값을 +50 하기
	// img_out2은 barbara영상에 밝기값을 -50 하기

	AddValue2Image(50, img, height, width, img_out1);
	ImageClipping(height, width, img_out1, img_out2);

	ImageShow((char*)"출력영상보기", img, height, width);
	ImageShow((char*)"출력1영상보기", img_out1, height, width); 
	ImageShow((char*)"출력2영상보기", img_out2, height, width); //클리핑 영상 출력
}

void EX0923_4()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"barbara.png", &height, &width);
	int** img_out1 = (int**)IntAlloc2(height, width);
	int** img_out2 = (int**)IntAlloc2(height, width);

	int maxvalue = GetMax(3, 2);
	int minvalue = GetMax(3, 2);

	int a = 300, b = -10, c = 200;

	a = GetMax(GetMin(a, 255), 0);
	b = GetMax(GetMin(b, 255), 0);
	c = GetMax(GetMin(c, 255), 0);

	printf("%d %d %d\n", a, b, c);

	//ImageShow((char*)"출력1영상보기", img_out1, height, width);
	//ImageShow((char*)"출력2영상보기", img_out2, height, width); //클리핑 영상 출력
}

void ImageMixing(float alpha, int height, int width, int** img1, int** img2, int** img_out)
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			img_out[y][x] = alpha * img1[y][x] + (1.0 - alpha) * img2[y][x];
		}
	}
}

void EX0923_5(char* winname1, char* winname2, char* winname3, char* winname4)
{
	int height, width;
	int** img1 = (int**)ReadImage((char*)"barbara.png", &height, &width);
	int** img2 = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);
	int** img_out2 = (int**)IntAlloc2(height, width);

	float alpha = 0.3;
	float alphaa = 0.7;

	ImageMixing(alpha, height, width, img1, img2, img_out);
	ImageMixing(alphaa, height, width, img1, img2, img_out2);

	ImageShow(winname1, img1, height, width);
	ImageShow(winname2, img2, height, width);
	ImageShow(winname3, img_out, height, width);
	ImageShow(winname4, img_out2, height, width);
}

void EX0923_6()
{
	EX0923_5((char*)"곽범근", (char*)"홍길동", (char*)"성춘향", (char*)"테스트");
}

/* 스트레칭, 히스토그램 09-30 */

void EX0930_1()  //스트레칭   -  기울기를 일직선으로 만들어줌 
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);


	int a = 150; // y=255/a * x, x에는 밝기값 img가 들어감 , x<a, y=255, x>=a

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (img[y][x] < a)    //이미지 밝기가 기울기보다 작으면
				img_out[y][x] = 255.0 / a * img[y][x]; //
			//img_out[y][x] = (float)255 / a * img[y][x];  //둘중 하나가 플롯이면 플롯으로 계산 .0을 붙이거나 변수 바로 앞에 float을 붙임
			else
				img_out[y][x] = 255;
		}
	}
	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", img_out, height, width);
}


#define RoundUp(x) ((int)(x+0.5)) //반올림
void stretching_1(int a, int height, int width, int** img, int** img_out) //스트레칭 함수화
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (img[y][x] < a)
				//img_out[y][x] = 255.0 / a * img[y][x]; //스트레칭
				img_out[y][x] = RoundUp(255.0 / a * img[y][x]); //반올림 매크로 사용
			else
				img_out[y][x] = 255;
		}
	}
}


void EX0930_2()  //스트레칭 함수 갖다씀
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	int a = 150;
	stretching_1(a, height, width, img, img_out);

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", img_out, height, width);
}

void stretching_2(int a, int b, int c, int d,
	int height,
	int width,
	int** img,
	int** img_out) //스트레칭 함수화2
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (img[y][x] <= a)
				img_out[y][x] = RoundUp((float)c / a * img[y][x]);
			else if (img[y][x] < b)
				img_out[y][x] = RoundUp(((float)d - c) / (b - a) * (img[y][x] - a) + c);  //float 연산자는 변수 바로 앞에 붙여야함. 
			else
				img_out[y][x] = RoundUp((255.0 - d) / (255 - b) * (img[y][x] - b) + d);
		}
	}
}

void EX0930_3()  //실습3
{
	int height, width;
	int** img = (int**)ReadImage((char*)"Barbara.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	int a = 100, b = 150, c = 50, d = 200;

	stretching_2(a, b, c, d, height, width, img, img_out);

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", img_out, height, width);
}


//히스토그램

void EX0930_4() //히스토그램
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	int Hist[256] = { 0 };   //초기값을 통째로 다 0
	int brightness = 100;
	//int count = 0;

	for (int brightness = 0; brightness < 256; brightness++) {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (img[y][x] == brightness)
					Hist[brightness]++;
			}
		}
	}

	ImageShow((char*)"입력영상보기", img, height, width);
	DrawHistogram((char*)"히스토그램", Hist); // histogram을 그래프 그려주는 함수
}

void GetHistogram_1(int** img, int height, int width, int* hist) //히스토그램 함수화.  1차원배열이면 별표 하나
{
	for (int brightness = 0; brightness < 256; brightness++) {     //256번 스캔
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (img[y][x] == brightness)
					hist[brightness]++;
			}
		}
	}
}

void GetHistogram_2(int** img, int height, int width, int* hist) //위의 히스토그램 함수 간단하게
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {         //영상 한번만 스캔
			hist[img[y][x]]++;
		}
	}
}

void EX0930_5()  //간단하게 한 히스토그램 함수2 사용
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	int Hist[256] = { 0 };   //초기값을 통째로 다 0
	//GetHistogram_1(img, height, width, Hist);
	GetHistogram_2(img, height, width, Hist);

	ImageShow((char*)"입력영상보기", img, height, width);
	DrawHistogram((char*)"히스토그램", Hist); // histogram을 그려주는 함수
}


/*  배열
void main()
{
	int A[4] = {100, 200, 300, 400};
	int* B;

	B = A + 1;

	printf("%d, %d \n", *(B - 2), B[-2]);

	//printf("%d, %d, %d \n", B, *B, *(B+2));    //*는 A자리에 있는 값
	//printf("%d, %d, %d \n", B, B[0], B[2]);

}
*/

/*  히스토그램 평활화 10-14 */

void C_Histogram(int** img, int height, int width, int* C_Hist)
{
	int Hist[256] = { 0 };
	GetHistogram_2(img, height, width, Hist);
	C_Hist[0] = Hist[0];
	//C_Hist[1] = Hist[0] + Hist[1];
	//C_Hist[2] = Hist[2]; +Hist[1];
	//C_Hist[3] = Hist[3]; +Hist[2]; //적분
	for (int n = 1; n < 256; n++)
		C_Hist[n] = Hist[n] + C_Hist[n - 1];
}

void Norm_C_Histogram(int** img, int height, int width, int* NC_Hist) //평활화 히스토그램
{
	int C_Hist[256] = { 0 };
	C_Histogram(img, height, width, C_Hist); // 누적 히스토그램 함수 재활용
	for (int n = 1; n < 256; n++)
		NC_Hist[n] = (float)C_Hist[n] / (width * height) * 255 + 0.5;
}

void EX1014_1() //누적 히스토그램
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	int Hist[256] = { 0 };   //초기값을 통째로 다 0
	int C_Hist[256] = { 0 };
	int NC_Hist[256] = { 0 };

	GetHistogram_2(img, height, width, Hist);
	C_Histogram(img, height, width, C_Hist);
	Norm_C_Histogram(img, height, width, NC_Hist);

	ImageShow((char*)"입력영상보기", img, height, width);
	DrawHistogram((char*)"히스토그램", Hist); // histogram을 그려주는 함수
	DrawHistogram((char*)"누적 히스토그램", C_Hist); // 누적 C_Histogram을 그려주는 함수 512*512=262144
	DrawHistogram((char*)"누적+평활화 히스토그램", NC_Hist); // 평활화 히스토그램
}

void EX1014_2() //평활화 히스토그램
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	int NC_Hist[256] = { 0 };
	
	Norm_C_Histogram(img, height, width, NC_Hist);

	ImageShow((char*)"입력영상보기", img, height, width);
	DrawHistogram((char*)"평활화 히스토그램", NC_Hist); // 평활화 히스토그램
	}

void HistogramEqualization(int** img, int height, int width, int** img_out) //평활화히스토그램 영상 출력 함수
{
	int NC_Hist[256] = { 0 };

	Norm_C_Histogram(img, height, width, NC_Hist);

	/// 히스토그램 평활화 : Y = T[X]
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++) {
			img_out[y][x] = NC_Hist[img[y][x]];
		}
}


void EX1014_3() //평활화 히스토그램 영상 출력
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lenax0.5.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	HistogramEqualization(img, height, width, img_out);

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", img_out, height, width);
}

/* 평균필터 10-14 */

void Avg3x3(int** img, int height, int width, int** img_out)
{
	// 3x3 필터 구현
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++) {
			if (x == 0 || x == (width - 1) || y == 0 || y == (height - 1)) {
				img_out[y][x] = img[y][x];
			}
			else {
				img_out[y][x] = (int)((img[y - 1][x - 1] + img[y - 1][x] + img[y - 1][x + 1] + 
										img[y][x - 1] + img[y][x] + img[y][x + 1] + 
										img[y + 1][x - 1] + img[y + 1][x] + img[y + 1][x + 1]) / 9.0 + 0.5);
			}
		}
}

void EX1014_4()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);
	
	// 3x3 필터 구현
	Avg3x3(img, height, width, img_out);

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", img_out, height, width);
}

void Avg3x3_two(int** img, int height, int width, int** img_out)
{
	// 3x3 필터 구현
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++) {
			if (x == 0 || x == (width - 1) || y == 0 || y == (height - 1)) {
				img_out[y][x] = img[y][x];
			}
			else {
				int sum = 0;
				for (int i = -1; i <= 1; i++)
					for (int j = -1; j <= 1; j++)
					{
						sum += img[y + i][x + j];
						img_out[y][x] = sum / 9.0 + 0.5;
					}
			}
		}
}

void AvgNxN(int n, int** img, int height, int width, int** img_out) //NxN 필터 구현
{
	int delta = (n - 1) / 2;

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++) {
			if (x < delta || x >= (width - delta) || y < delta || y >= (height - delta)) {
				img_out[y][x] = img[y][x];
			}
			else {
				int sum = 0;
				for (int i = -delta; i <= delta; i++)
					for (int j = -delta; j <= delta; j++)
					{
						sum += img[y + i][x + j];
						img_out[y][x] = (float)sum / (n*n) + 0.5;
					}
			}
		}
}

/* 평균필터, 필터링(마스킹) 11-10 */

void EX1021_1()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	// NxN 필터 구현
	ImageShow((char*)"입력영상보기", img, height, width);

	AvgNxN(3, img, height, width, img_out);
	ImageShow((char*)"3x3출력영상보기", img_out, height, width);

	AvgNxN(5, img, height, width, img_out);
	ImageShow((char*)"5x5출력영상보기", img_out, height, width);

	AvgNxN(7, img, height, width, img_out);
	ImageShow((char*)"7x7출력영상보기", img_out, height, width);

	AvgNxN(9, img, height, width, img_out);
	ImageShow((char*)"9x9출력영상보기", img_out, height, width);

	AvgNxN(19, img, height, width, img_out);
	ImageShow((char*)"19x19출력영상보기", img_out, height, width);

	//ImageShow((char*)"출력영상보기", img_out, height, width);
}

void AvgNxN_two(int n, int** img, int height, int width, int** img_out) //가장자리 처리 NxN 필터
{
	int delta = (n - 1) / 2;

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++) {

			int sum = 0;
			for (int i = -delta; i <= delta; i++)
				for (int j = -delta; j <= delta; j++)
					sum += img[GetMin(GetMax(y + i, 0), height - 1)][GetMin(GetMax(x + j, 0), width - 1)];
					img_out[y][x] = (float)sum / (n * n) + 0.5; //영상 바깥쪽 픽셀의 밝기값을 가장 가까운 유효한 픽셀의 밝기값으로 함
		}
}

void EX1021_2()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);


	ImageShow((char*)"입력영상보기", img, height, width);
	
	AvgNxN(7, img, height, width, img_out);	// NxN 필터
	ImageShow((char*)"AvgNxN 영상보기", img_out, height, width);

	AvgNxN_two(7, img, height, width, img_out); // NxN 필터 (가장자리 처리)
	ImageShow((char*)"AvgNxN_two 영상보기", img_out, height, width);

}

void Avg3x3_WithMask(float mask[3][3], int** img, int height, int width, int** img_out)
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (x == 0 || x == (width - 1) || y == 0 || y == (height - 1)) {
				img_out[y][x] = img[y][x];
			}
			else {
				/*				img_out[y][x] = mask[0][0] * img[y - 1][x - 1] + mask[0][1] * img[y - 1][x] + img[0][2] * img[y - 1][x + 1]
												+ mask[1][0] * img[y][x - 1] + mask[1][1] * img[y][x] + img[1][2] * img[y][x + 1]
												+ mask[2][0] * img[y + 1][x - 1] + mask[2][1] * img[y + 1][x] + img[2][2] * img[y + 1][x + 1] + 0.5;
				*/
				float avg = 0.0;
				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; j++) {
						avg += mask[i + 1][j + 1] * img[y - i][x + j];
					}
				}
				img_out[y][x] = avg + 0.5;
			}
		}
	}
}

void EX1021_4()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	float mask[3][3] = { {1 / 9.0, 1 / 9.0, 1 / 9.0},
						{1 / 9.0, 1 / 9.0, 1 / 9.0},
						{1 / 9.0, 1 / 9.0, 1 / 9.0 } };

	Avg3x3_WithMask(mask, img, height,width,img_out);


	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", img_out, height, width);
}

void Avg3x3_WithMask_two(float** mask, int** img, int height, int width, int** img_out)
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (x == 0 || x == (width - 1) || y == 0 || y == (height - 1)) {
				img_out[y][x] = img[y][x];
			}
			else {
				/*				img_out[y][x] = mask[0][0] * img[y - 1][x - 1] + mask[0][1] * img[y - 1][x] + img[0][2] * img[y - 1][x + 1]
												+ mask[1][0] * img[y][x - 1] + mask[1][1] * img[y][x] + img[1][2] * img[y][x + 1]
												+ mask[2][0] * img[y + 1][x - 1] + mask[2][1] * img[y + 1][x] + img[2][2] * img[y + 1][x + 1] + 0.5;
				*/
				float avg = 0.0;
				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; j++) {
						avg += mask[i + 1][j + 1] * img[y + i][x + j];
					}
				}
				img_out[y][x] = Clipping(avg + 0.5);
			}
		}
	}
}

void Avg3x3_WithMask_NoClipping(float** mask, int** img, int height, int width, int** img_out)
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (x == 0 || x == (width - 1) || y == 0 || y == (height - 1)) {
				img_out[y][x] = img[y][x];
			}
			else {
				/*				img_out[y][x] = mask[0][0] * img[y - 1][x - 1] + mask[0][1] * img[y - 1][x] + img[0][2] * img[y - 1][x + 1]
												+ mask[1][0] * img[y][x - 1] + mask[1][1] * img[y][x] + img[1][2] * img[y][x + 1]
												+ mask[2][0] * img[y + 1][x - 1] + mask[2][1] * img[y + 1][x] + img[2][2] * img[y + 1][x + 1] + 0.5;
				*/
				float avg = 0.0;
				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; j++) {
						avg += mask[i + 1][j + 1] * img[y + i][x + j];
					}
				}
				img_out[y][x] = avg + 0.5;
			}
		}
	}
}

void EX1021_5()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);
	float** mask = (float**)FloatAlloc2(3, 3);
	
	mask[0][0] = 0;				mask[0][1] = -1 / 4.0;				mask[0][2] = 0;
	mask[1][0] = -1 / 4.0;		mask[1][1] = 2.0;					mask[1][2] = -1 / 4.0;
	mask[2][0] = 0;				mask[2][1] = -1 / 4.0;				mask[2][2] = 0;

	Avg3x3_WithMask_two(mask, img, height, width, img_out);


	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", img_out, height, width);
}

/* 에지검출 10-28 */

void main_sample() //최대값 찾기
{
	//E1021_6();

	int data[5] = { 3, -1, 2, 6, 0 };
	int max_value = data[0];
	for (int n = 1; n < 5; n++)

		max_value = GetMax(max_value, data[n]);

	printf("\n max = % d", max_value);


}

void MagGradient_X(int** img, int height, int width, int** img_out) //x방향 그라디언트 크기 구하기
{
	for (int y = 0; y < height - 1; y++) {
		for (int x = 0; x < width - 1; x++) {
			img_out[y][x] = abs(img[y][x + 1] - img[y][x]);
		}
	}
}

void MagGradient_Y(int** img, int height, int width, int** img_out)//y방향 그라디언트 크기 구하기
{
	for (int y = 0; y < height - 1; y++) {
		for (int x = 0; x < width - 1; x++) {
			img_out[y][x] = abs(img[y + 1][x] - img[y][x]);
		}
	}
}

void MagGradient(int** img, int height, int width, int** img_out)//그라디언트 크기 구하기
{
	for (int y = 0; y < height - 1; y++) {
		for (int x = 0; x < width - 1; x++) {
			img_out[y][x] = abs(img[y][x + 1] - img[y][x]) + abs(img[y + 1][x] - img[y][x]);
		}
	}
}

int FindMaxValue(int** img, int height, int width)// 최대값을 찾는 함수
{
	int max_value = img[0][0];
	for (int y = 0; y < height - 1; y++) {
		for (int x = 0; x < width - 1; x++) {
			max_value = GetMax(max_value, img[y][x]);
		}
	}
	return(max_value);
}

int FindMinValue(int** img, int height, int width)// 최소값을 찾는 함수
{
	int min_value = img[0][0];
	for (int y = 0; y < height - 1; y++) {
		for (int x = 0; x < width - 1; x++) {
			min_value = GetMin(min_value, img[y][x]);
		}
	}
	return(min_value);
}

void NormalizeByMax(int** img, int height, int width, int** img_out) //최대값으로 정규화
{
	// 정규화를 위해 최대값 찾기
	int max_value = FindMaxValue(img, height, width);


	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			img[y][x] = ((float)img[y][x] / max_value) * 255;
		}
	}
}

/**void E1028_1()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);

	int max_value_main = NormalizeByMax(img, height, width);

	printf("\n max = % d", max_value_main);
}**/


void EX1028_2() //그라디언트 크기를 최대값으로 정규화 (0~255로 범위로 스케일링)
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** grad = (int**)IntAlloc2(height, width);

	MagGradient(img, height, width, grad); // 그라디언트 크기를 img_out에 씀
	NormalizeByMax(grad, height, width, img); // img_out을 0~255 내로정규화

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", grad, height, width);

}

void EX1028_3() //그라디언트 크기를 최대값으로 정규화 (0~255로 범위로 스케일링)
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lenaGN15.png", &height, &width); //잡음이 있는 경우
	int** grad = (int**)IntAlloc2(height, width);

	MagGradient(img, height, width, grad); // 그라디언트 크기를 img_out에 씀
	NormalizeByMax(grad, height, width, img); // img_out을 0~255 내로정규화

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", grad, height, width);

}

void EX1028_4() //x방향 및 y방향 그라디언트 크기를 최대값으로 정규화 (0~255로 범위로 스케일링)
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out_x = (int**)IntAlloc2(height, width);
	int** img_out_y = (int**)IntAlloc2(height, width);

	MagGradient_X(img, height, width, img_out_x);
	MagGradient_Y(img, height, width, img_out_y);
	NormalizeByMax(img_out_x, height, width, img_out_x);
	NormalizeByMax(img_out_y, height, width, img_out_y);

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력(fx)영상보기", img_out_x, height, width);
	ImageShow((char*)"출력(fy)영상보기", img_out_y, height, width);
}


/* 선명화처리 11-04 */

void EX1104() //라플라시안 크기
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** laplacian = (int**)IntAlloc2(height, width);
	float** mask = (float**)FloatAlloc2(3, 3);

	mask[0][0] = -1; mask[0][1] = -1; mask[0][2]= - 1;
	mask[1][0] = -1; mask[1][1] = 8; mask[1][2]= - 1;
	mask[2][0] = -1; mask[2][1] = -1; mask[2][2]= - 1;

	Avg3x3_WithMask_two(mask, img, height, width, laplacian);

	for (int y = 0; y < height; y++) for (int x = 0; x<width; x++)
		laplacian[y][x] = abs(laplacian[y][x]); //절대값

	NormalizeByMax(laplacian, height, width, laplacian);

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", laplacian, height, width);
}

void MagSobel_X(int** img, int height, int width, int** img_out) //x방향 소벨연산 크기 구하기
{
	float** mask = (float**)FloatAlloc2(3, 3);

	mask[0][0] = -1; mask[0][1] = -2;    mask[0][2] = -1;
	mask[1][0] = 0; mask[1][1] = 0;		 mask[1][2] = 0;
	mask[2][0] = 1; mask[2][1] = 2;		 mask[2][2] = 1;

	Avg3x3_WithMask_NoClipping(mask, img, height, width, img_out);

	for (int y = 0; y < height; y++) for (int x = 0; x < width; x++)
		img_out[y][x] = abs(img_out[y][x]); //절대값

	FloatFree2(mask, 3, 3);
}

void MagSobel_Y(int** img, int height, int width, int** img_out) //y방향 소벨연산 크기 구하기
{
	float** mask = (float**)FloatAlloc2(3, 3);

	mask[0][0] = 1; mask[0][1] = 0;    mask[0][2] = -1;
	mask[1][0] = 2; mask[1][1] = 0;		 mask[1][2] = -2;
	mask[2][0] = 1; mask[2][1] = 0;		 mask[2][2] = -1;

	Avg3x3_WithMask_NoClipping(mask, img, height, width, img_out);

	for (int y = 0; y < height; y++) for (int x = 0; x < width; x++)
		img_out[y][x] = abs(img_out[y][x]); //절대값

	FloatFree2(mask, 3, 3);
}

void EX1104_Sobel_XY() //x방향 및 y방향 소벨 연산
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** sobel_x = (int**)IntAlloc2(height, width);
	int** sobel_y = (int**)IntAlloc2(height, width);
	int** img_out = (int**)IntAlloc2(height, width);

	MagSobel_X(img, height, width, sobel_x);
	MagSobel_Y(img, height, width, sobel_y);
	NormalizeByMax(sobel_x, height, width, sobel_x);
	NormalizeByMax(sobel_y, height, width, sobel_y);

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"x출력영상보기", sobel_x, height, width);
	ImageShow((char*)"y출력영상보기", sobel_y, height, width);
}

void EX1104_Sobel()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** sobel_x = (int**)IntAlloc2(height, width);
	int** sobel_y = (int**)IntAlloc2(height, width);
	int** img_out = (int**)IntAlloc2(height, width);

	MagSobel_X(img, height, width, sobel_x);
	MagSobel_Y(img, height, width, sobel_y);

	for (int y = 0; y < height; y++) for (int x = 0; x < width; x++)
		img_out[y][x] = (sobel_x[y][x] + sobel_y[y][x]);

	NormalizeByMax(img_out, height, width, img_out);

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", img_out, height, width);
}

void Sharpening(int** img, int height, int width, int** img_out, float alpha)
{
	float** mask = (float**)FloatAlloc2(3, 3);

	mask[0][0] = -alpha; mask[0][1] = -alpha;	 mask[0][2] = -alpha;
	mask[1][0] = -alpha; mask[1][1] = 1 + 8.0 * alpha;	 mask[1][2] = -alpha;
	mask[2][0] = -alpha; mask[2][1] = -alpha;	 mask[2][2] = -alpha;

	Avg3x3_WithMask_two(mask, img, height, width, img_out);

}

void EX1104_Sharpening()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);
	int** img_out1 = (int**)IntAlloc2(height, width);
	int** img_out2 = (int**)IntAlloc2(height, width);

	Sharpening(img, height, width, img_out, 0.25);
	Sharpening(img, height, width, img_out1, 0.5);
	Sharpening(img, height, width, img_out2, 0.75);
	
	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기1", img_out, height, width);
	ImageShow((char*)"출력영상보기2", img_out1, height, width);
	ImageShow((char*)"출력영상보기3", img_out2, height, width);
}

/*test1~4 중간

 void stretching(int** img, int height, int width, int** img_out)
{
	int max_value = FindMaxValue(img, height, width);
	int min_value = FindMinValue(img, height, width);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			img_out[y][x] = ((float)img[y][x] / max_value) * 255;
		}
	}
}

void Prob1()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"test1.bmp", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	stretching(img, height, width, img_out);

	ImageShow((char*)"영상보기", img, height, width);
	ImageShow((char*)"출력영상", img_out, height, width);
}

void Prob2()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"test2.bmp", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);
}

void Prob3_1()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"test3.bmp", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	ImageShow((char*)"영상보기", img, height, width);
	ImageShow((char*)"출력영상", img_out, height, width);
}

void Prob3_2()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"test3.bmp", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	ImageShow((char*)"영상보기", img, height, width);
	ImageShow((char*)"출력영상", img_out, height, width);
}

void Aavg3x3(int** img, int height, int width, int** img_out)
{
	int max_value = FindMaxValue(img, height, width);

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++) {
			if (x == 0 || x == (width - 1) || y == 0 || y == (height - 1)) {

				img_out[y][x] = max_value;
			}
			else {
				img_out[y][x] = (int)((img[y - 1][x - 1] + img[y - 1][x] + img[y - 1][x + 1] +
					img[y][x - 1] + img[y][x] + img[y][x + 1] +
					img[y + 1][x - 1] + img[y + 1][x] + img[y + 1][x + 1]) / 9.0 + 0.5);
			}
		}
}

void Prob4()
{
		int height, width;
		int** img = (int**)ReadImage((char*)"test4.bmp", &height, &width);
		int** img_out = (int**)IntAlloc2(height, width);

		Aavg3x3(img, height, width, img_out);
	
		ImageShow((char*)"입력영상보기", img, height, width);
		ImageShow((char*)"출력영상보기", img_out, height, width);
}

int main()
{
	Prob1();
	Prob2();
	Prob3_1();
	Prob3_2();
	Prob4();
}
*/

/* 12-02 */

void Swap(int* a, int* b)
{
	int buff = *a;
	*a = *b;	*b = buff;
}


void Bubbling(int* A, int num)
{
	for (int i = 0; i < num - 1; i++) {
		if (A[i] > A[i + 1])  Swap(&A[i], &A[i + 1]); // 바로 이웃한 값끼리 위치 바꾸기
	}
}

void BubbleSort(int* A, int N)
{
	for (int i = 0; i < N - 1; i++) // 버블링 반복, 맨처음은 N개에 대해, 두번째는 (N-1)개에 대해
		Bubbling(A, N - i);
}

void GetBlock1D(int y0, int x0, int dy, int dx,
	int** img, int height, int width, int* data1D)
{
	for (int y = 0; y < dy; y++) {
		for (int x = 0; x < dx; x++) {
			data1D[y * dx + x] = img[y + y0][x + x0];
		}
	 }
}

void GetBlock2D(int y0, int x0, int dy, int dx,
	int** img, int height, int width, int** block)
{
	for (int y = 0; y < dy; y++) {
		for (int x = 0; x < dx; x++) {
			block[y][x] = img[y + y0][x + x0];
		}
	}
}

void MedianFiltering(int** img, int height, int width, int** img_out)
{
	int data[9];
	//int y0 = 100, x0 = 100, dy = 3, dx = 3; //3-3
	int dy = 3, dx = 3;


	for (int y0 = 0; y0 < height - 2; y0++) {
		for (int x0 = 0; x0 < width - 2; x0++) {
			GetBlock1D(y0, x0, dy, dx, img, height, width, data);
			BubbleSort(data, 9); // data[4]이 중간 값
			img_out[y0 + 1][x0 + 1] = data[4];
		}
	}
}

void EX1202_1()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"lenaSP20.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);
	int** img_out2 = (int**)IntAlloc2(height, width);

	MedianFiltering(img, height, width, img_out);
	MedianFiltering(img_out, height, width, img_out2);

	/*for (int i = 0; i < 9; i++)
		printf("%d \n", data[i]);*/

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"입력영상보기", img_out, height, width);
	ImageShow((char*)"입력영상보기", img_out2, height, width);
}

void UpSamplingx2_repeat(int** img, int height, int width, int** img_out)
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			img_out[2 * y][2 * x] = img[y][x];
			img_out[2 * y][2 * x + 1] = img[y][x];
			img_out[2 * y + 1][2 * x] = img[y][x];
			img_out[2 * y + 1][2 * x + 1] = img[y][x];
		}
	}
}

void UpSamplingx2_AVG(int** img, int height, int width, int** img_out)
{
	for (int y = 0; y < height-1; y++) {
		for (int x = 0; x < width-1; x++) {
			img_out[2 * y][2 * x] = img[y][x];
			img_out[2 * y][2 * x + 1] = (img[y][x] + img[y][x+1]+1)/2 ; // <--- 반올림 1
			img_out[2 * y + 1][2 * x] = (img[y][x] + img[y + 1][x]+1) / 2;// <---
			img_out[2 * y + 1][2 * x + 1] = (img[y][x] + img[y + 1][x] + img[y][x+1] + img[y+1][x+1]+2)/4; // <---  반올림 2
		}
	}
}

void DownSamplingx2(int** img, int height, int width, int** img_out)
{
	for (int y = 0; y < height; y += 2) {
		for (int x = 0; x < width; x += 2) {
			img_out[y/2][x/2] = img[y][x];
		}
	}
}

void EX1202_2()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"s_lena.png", &height, &width);
	int** img_out = (int**)(int**)IntAlloc2(2 * height, 2 * width);
	int** img_out2 = (int**)(int**)IntAlloc2(2 * height, 2 * width);
	int** img_out3 = (int**)(int**)IntAlloc2(height/2, width/2);

	//UpSamplingx2_repeat(img, height, width, img_out);
	//UpSamplingx2_AVG(img, height, width, img_out2);

	DownSamplingx2(img, height, width, img_out3);

	ImageShow((char*)"입력영상보기", img, height, width);
	//ImageShow((char*)"출력(반복)영상보기", img_out, 2 * height, 2 * width);
	//ImageShow((char*)"출력(평균)영상보기", img_out2, 2 * height, 2 * width);
	ImageShow((char*)"출력(축소)영상보기", img_out3, height/2, width/2);
}

//구조체 //img+height+width를 한번에 묶을 때 사용
struct IMAGE {
	int** img;
	int height, width; //묶고 싶은 것들 
};

/*class _IMG {
public:
	int** img;
	int height, width;
	_IMG() {

	}
	_IMG() {

	}
};
*/

void UpSamplingx2_repeat_2(struct IMAGE* input, struct IMAGE* output)
{
	for (int y = 0; y < input->height; y++) { //input.height
		for (int x = 0; x < input->width; x++) { //input.width
			output->img[2 * y][2 * x] = input->img[y][x]; //input.img
			output->img[2 * y][2 * x + 1] = input->img[y][x]; //input.img
			output->img[2 * y + 1][2 * x] = input->img[y][x]; //input.img
			output->img[2 * y + 1][2 * x + 1] = input->img[y][x];//input.img
		}
	}
}

void EX1202_3()
{
	struct IMAGE A;
	struct IMAGE B;

	A.img = (int**)ReadImage((char*)"lena.png", &A.height, &A.width);

	B.height = 2 * A.height;
	B.width = 2 * A.width;
	B.img = (int**)(int**)IntAlloc2(2 * B.height, 2 * B.width);

	UpSamplingx2_repeat_2(&A, &B); /* UpSamplingx2_repeat_2(&A, img_out); &A는 주소일 떄, .을 쓰지 않고 -> 를 사용해야함.*/

	ImageShow((char*)"입력영상보기", A.img, A.height, A.width);
	ImageShow((char*)"출력(반복)입력영상보기", B.img, 2* A.height, 2*A.width);
}

/* 12-09 */;

void EX1209_1()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"s_lena.png", &height, &width);
	int** img_out = (int**)(int**)IntAlloc2(2 * height, 2 * width);

	for (int y = 0; y < 2 * height; y++) {
		for (int x = 0; x < 2 * width; x++) {
			img_out[y][x] = BilinearInterpolation(y / 2.0, x / 2.0, img, height, width); //(2배로 하고 왜 나눌까?)
		}
	}

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력력영상보기", img_out, 2 * height, 2 * width);
}

void EX1209_2()
{
	int height, width;
	int** img = (int**)ReadImage((char*)"s_lena.png", &height, &width);

	float alpha = 1.3;
	int height_out = (int)(alpha * height);
	int width_out = (int)(alpha * width);
	int** img_out = (int**)IntAlloc2(height_out, width_out);
	printf("\n img_out size : h = %d, w = %d", height_out, width_out);

	for (int y = 0; y < height_out; y++) {
		for (int x = 0; x < width_out; x++) {
			img_out[y][x] = BilinearInterpolation(y / alpha, x / alpha, img, height, width );
		}
	}
	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", img_out, height_out, width_out);
}

void Translation(int ty, int tx, int** img, int height, int width, int** img_out)
{
	for (int y_prime = 0; y_prime < height; y_prime++) {
		for (int x_prime = 0; x_prime < width; x_prime++) {
			int y = y_prime - ty;
			int x = x_prime - tx;

			if (!(y < 0 || x < 0 || y >= height || x >= width))
				img_out[y_prime][x_prime] = img[y][x];
		}
	}
}

void Translation_Float(float ty, float tx, int** img, int height, int width, int** img_out)
{
	for (int y_prime = 0; y_prime < height; y_prime++) {
		for (int x_prime = 0; x_prime < width; x_prime++) {
			float y = y_prime - ty; // 100.5 -> 배열 번호 100.5 X
			float x = x_prime - tx;

			if (!(y < 0 || x < 0 || y >= height || x >= width))
				img_out[y_prime][x_prime] = BilinearInterpolation(y, x, img, height, width);
		}
	}
}

// pi/180 = rad/degree ->> rad = degree * pi / 180
void Rotation(float degree, int** img, int height, int width, int** img_out)
{
	float rad = degree * CV_PI / 180;

	for (int y_prime = 0; y_prime < height; y_prime++) {
		for (int x_prime = 0; x_prime < width; x_prime++) {
			float y = - x_prime * sin(rad) + y_prime * cos(rad);
			float x = x_prime * cos(rad) + y_prime * sin(rad);

			if (!(y < 0 || x < 0 || y >= height || x >= width))
				img_out[y_prime][x_prime] = BilinearInterpolation(y, x, img, height, width);
		}
	}
}

void Rotation2(float degree, int y0, int x0, int y0_prime, int x0_prime,
	int** img, int height, int width, int** img_out) 
{
	float rad = degree * CV_PI / 180;

	for (int y_prime = 0; y_prime < height; y_prime++) {
		for (int x_prime = 0; x_prime < width; x_prime++) {
			float y = -(x_prime - x0_prime) * sin(rad) + (y_prime-y0_prime) * cos(rad) + y0;
			float x = (x_prime -x0_prime) * cos(rad) + (y_prime-y0_prime) * sin(rad) + x0;

			if (!(y < 0 || x < 0 || y >= height || x >= width))
				img_out[y_prime][x_prime] = BilinearInterpolation(y, x, img, height, width);
		}
	}
}

void AffineTransform(double a, double b, double c, double d,
	int y0, int x0, int y0_prime, int x0_prime,
	int** img, int height, int width, int** img_out)
{
	double a_prime = d / (a * d - b * c), b_prime = -b / (a * d - b * c);
	double c_prime = -c / (a * d - b * c), d_prime = a / (a * d - b * c);

	for (int y_prime = 0; y_prime < height; y_prime++) {
		for (int x_prime = 0; x_prime < width; x_prime++) {

			int y = c_prime * (x_prime - x0_prime) + d_prime * (y_prime - y0_prime) + y0;
			int x = a_prime * (x_prime - x0_prime) + b_prime * (y_prime - y0_prime) + x0;

			if (x < 0 || x >= width || y < 0 || y >= height) continue;
			else img_out[y_prime][x_prime] = BilinearInterpolation(y, x, img, height, width);
		}
	}
}


void EX1209_3()
{
	int height, width;
	//int** img = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img = (int**)ReadImage((char*)"s_barbara_4affine.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	//int ty = -50, tx = 100;
	//Translation(ty, tx, img, height, width, img_out);
	
    //float ty = -50.7, tx = 100.5;
	//Translation_Float(ty, tx, img, height, width, img_out);

	//float degree = -15.0;
	//Rotation(degree, img, height, width, img_out);

	//float degree = -45.0;

	//int y0 = height / 2, x0 = width / 2;
	//int y0_prime = height / 2, x0_prime = width / 2;
	//Rotation2(degree, y0, x0, y0_prime,  x0_prime, img, height, width,  img_out);

	double a = 1.0, b = 1.0;
	double c = -1.0, d = 1.0;
	int y0 = height / 2, x0 = width / 2;
	int y0_prime = height / 2, x0_prime = width / 2;

	AffineTransform(a, b, c, d, y0, x0, y0_prime, x0_prime, img, height, width, img_out);

	ImageShow((char*)"입력영상보기", img, height, width);
	ImageShow((char*)"출력영상보기", img_out, height, width);

}

void main()
{
	EX1104_Sobel();
}
