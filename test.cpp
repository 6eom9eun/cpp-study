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

int BilinearInterpolation(int** image, int width, int height, double x, double y)
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

int EX0923_0()
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
#define Clipping(x) (GetMin(GetMin(x, 255), 0))

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

void Ex0923_5(char* winname1, char* winname2, char* winname3)
{
	int height, width;
	int** img1 = (int**)ReadImage((char*)"barbara.png", &height, &width);
	int** img2 = (int**)ReadImage((char*)"lena.png", &height, &width);
	int** img_out = (int**)IntAlloc2(height, width);

	float alpha = 0.5;
	ImageMixing( alpha, height, width, img1, img2, img_out);

	ImageShow(winname1, img1, height, width);
	ImageShow(winname2, img2, height, width);
	ImageShow(winname3, img_out, height, width);
}

void main()
{
	Ex0923_5((char*)"곽범근", (char*)"홍길동", (char*)"성춘향");
}

//0930 예비군훈련

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
	DrawHistogram((char*)"히스토그램", Hist); // histogram을 그려주는 함수
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

void main()  //간단하게 한 히스토그램 함수2 사용
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

/*
void main()
{
	int C = 100;
	int* D;

	D = &C;
	printf("%d %d \n", C, &C);
	printf("%d %d \n", *(D+0), D);
	printf("%d %d \n", (D[0]), D);
}
*/