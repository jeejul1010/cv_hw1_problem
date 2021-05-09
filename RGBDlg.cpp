// RGBDlg.cpp : 구현 파일
//
#include "stdafx.h"
#include "RGB.h"
#include "RGBDlg.h"
#include "afxdialogex.h"
#include "string.h"
#include <math.h>
#include <stdio.h>
#include <conio.h>
#pragma warning(disable:4996)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FORWARD 0
#define BACKWARD 1

const int MAX_SIZE = 512*512;
int num_region[MAX_SIZE] = { 0, }; //각 영역의 화소 수


typedef struct ContourPoint { //contourtracing 구성하는 점의 구조체
	int x;
	int y;
}ContourPoint;

typedef struct Contour {
	int count;
	ContourPoint *points;
	double *LCS;
};

Contour *g_contour;

const int windowSize = 99;

const int NUM_CLASS = 9;
const int NUM_TRAIN_SAMPLES = 5;
const int NUM_TEST_SAMPLES = 10;

Mat shapeImages[NUM_CLASS][NUM_TRAIN_SAMPLES + NUM_TEST_SAMPLES];
Contour* trainContours[NUM_CLASS][NUM_TRAIN_SAMPLES];

const char* shapes[] = {
	"Circle",
	"Heptagon",
	"Hexagon",
	"Nonagon",
	"Octagon",
	"Pentagon",
	"Square",
	"Star",
	"Triangle"
};

void chageColor(Mat& img, Mat &copy, int i);
Mat changeToGray(const Mat& img);
int argmaxIndex(double* array);

char imgName[100] = "copy";

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRGBDlg 대화 상자



CRGBDlg::CRGBDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_RGB_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRGBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Img, m_pic);
}

BEGIN_MESSAGE_MAP(CRGBDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_Img_Search, &CRGBDlg::OnBnClickedImgSearch)
	ON_BN_CLICKED(IDC_Img_Save, &CRGBDlg::OnBnClickedImgSave)
	ON_BN_CLICKED(IDC_Red_Btn, &CRGBDlg::OnBnClickedRedBtn)
	ON_BN_CLICKED(IDC_Green_Btn, &CRGBDlg::OnBnClickedGreenBtn)
	ON_BN_CLICKED(IDC_Blue_Btn, &CRGBDlg::OnBnClickedBlueBtn)
	ON_BN_CLICKED(IDC_Gray_Btn, &CRGBDlg::OnBnClickedGrayBtn)
	ON_BN_CLICKED(IDC_Btn_Otsu, &CRGBDlg::OnBnClickedBtnOtsu)
	ON_BN_CLICKED(IDC_Btn_Opening, &CRGBDlg::OnBnClickedBtnOpening)
	ON_BN_CLICKED(IDC_Btn_Closing, &CRGBDlg::OnBnClickedBtnClosing)
	ON_BN_CLICKED(IDC_Btn_ContourTracing, &CRGBDlg::OnBnClickedBtnContourtracing)
	ON_BN_CLICKED(IDC_Btn_Classify, &CRGBDlg::OnBnClickedBtnClassify)
	ON_BN_CLICKED(IDC_Btn_LCS, &CRGBDlg::OnBnClickedBtnLcs)
	ON_BN_CLICKED(IDC_Btn_Reverse, &CRGBDlg::OnBnClickedBtnReverse)
	ON_BN_CLICKED(IDC_Btn_DetectShape, &CRGBDlg::OnBnClickedBtnDetectshape)
END_MESSAGE_MAP()


// CRGBDlg 메시지 처리기

BOOL CRGBDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CRGBDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CRGBDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CRGBDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRGBDlg::OnBnClickedImgSearch()
{
	static TCHAR BASED_CODE szFilter[] = _T("이미지 파일(*.BMP, *.GIF, *.JPG, *.PNG) | *.BMP;*.GIF;*.JPG;*.PNG;*.bmp;*.jpg;*.gif;*.png |모든파일(*.*)|*.*||");
	CFileDialog dlg(TRUE, _T("*.jpg"), _T("image"), OFN_HIDEREADONLY, szFilter);
	if (IDOK == dlg.DoModal())
	{
		pathName = dlg.GetPathName();
		CT2CA pszConvertedAnsiString_up(pathName);
		std::string up_pathName_str(pszConvertedAnsiString_up);
		img = cv::imread(up_pathName_str);
		DisplayImage(img, 3);
	}
}

void CRGBDlg::DisplayImage(const Mat& targetMat, int channel)
{
	CDC *pDC = nullptr;
	CImage* mfcImg = nullptr;

	pDC = m_pic.GetDC();
	CStatic *staticSize = (CStatic *)GetDlgItem(IDC_Img);
	staticSize->GetClientRect(rect);

	cv::Mat tempImage;
	cv::resize(targetMat, tempImage, Size(rect.Width(), rect.Height()));

	BITMAPINFO bitmapInfo;
	bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	bitmapInfo.bmiHeader.biBitCount = 24;
	bitmapInfo.bmiHeader.biWidth = tempImage.cols;
	bitmapInfo.bmiHeader.biHeight = tempImage.rows;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	bitmapInfo.bmiHeader.biClrImportant = 0;
	bitmapInfo.bmiHeader.biClrUsed = 0;
	bitmapInfo.bmiHeader.biSizeImage = 0;
	bitmapInfo.bmiHeader.biXPelsPerMeter = 0;

	if (targetMat.channels() == 3)
	{
		mfcImg = new CImage();
		mfcImg->Create(tempImage.cols, tempImage.rows, 24);
	}
	else if (targetMat.channels() == 1)
	{
		cvtColor(tempImage, tempImage, cv::COLOR_GRAY2BGR); //cv::CV_GRAY2RGB //8
		mfcImg = new CImage();
		mfcImg->Create(tempImage.cols, tempImage.rows, 24);
	}
	else if (targetMat.channels() == 4)
	{
		bitmapInfo.bmiHeader.biBitCount = 32;
		mfcImg = new CImage();
		mfcImg->Create(tempImage.cols, tempImage.rows, 32);
	}
	cv::flip(tempImage, tempImage, 0);
	::StretchDIBits(mfcImg->GetDC(), 0, 0, tempImage.cols, tempImage.rows,
		0, 0, tempImage.cols, tempImage.rows, tempImage.data, &bitmapInfo,
		DIB_RGB_COLORS, SRCCOPY); //frame buffer에 그려주는 함수

	mfcImg->BitBlt(::GetDC(m_pic.m_hWnd), 0, 0);

	if (mfcImg)
	{
		mfcImg->ReleaseDC();
		delete mfcImg;// mfcImg = nullptr;
	}
	tempImage.release();
	ReleaseDC(pDC);
}

void CRGBDlg::OnBnClickedImgSave()
{
	strcat(imgName,".jpg");
	imwrite(imgName, img);
	MessageBox(_T("이미지 저장 완료!"), _T(""));
}


void CRGBDlg::OnBnClickedRedBtn()
{
	Mat img_copy = img.clone();
	chageColor(img, img_copy, 2);
	DisplayImage(img_copy, 3);
	img = img_copy.clone();
	strcpy(imgName, "red");
}


void CRGBDlg::OnBnClickedGreenBtn()
{
	Mat img_copy = img.clone();
	chageColor(img, img_copy, 1);
	DisplayImage(img_copy, 3);
	img = img_copy.clone();
	strcpy(imgName, "green");
}


void CRGBDlg::OnBnClickedBlueBtn()
{
	Mat img_copy = img.clone();
	chageColor(img, img_copy, 0);
	DisplayImage(img_copy, 3);
	img = img_copy.clone();
	strcpy(imgName, "blue");
}

void CRGBDlg::OnBnClickedGrayBtn()
{
	Mat gray = changeToGray(img);
	DisplayImage(gray, 3);
	img = gray;
	strcpy(imgName, "gray");
}

void drawHistogram(char* title, double* hist, double maxValue, int number_bins, int hist_w = 512, int hist_h = 512) {
	int bin_w = cvRound((double)hist_w / number_bins);

	Mat histImage(hist_h, hist_w, CV_8UC1, Scalar(0, 0, 0));

	double minValue = 0;
	if (maxValue < 0.00001) {
		maxValue = hist[0];
		minValue = hist[0];
		for (int i = 1; i < number_bins; i++) {
			if (maxValue < hist[i]) {
				maxValue = hist[i];
			}
			if (hist[i] < minValue) {
				minValue = hist[i];
			}
		}
	}
	if (maxValue == 0.0) {
		maxValue = 1.0;
	}
	if (minValue > 0) {
		minValue = 0;
	}

	for (int i = 1; i < number_bins; i++)
	{
		line(histImage,
			Point(bin_w * (i - 1), hist_h - cvRound((hist[i - 1]-minValue)/(maxValue-minValue)*hist_h)),
			Point(bin_w * (i), hist_h - cvRound((hist[i]-minValue)/(maxValue-minValue)*hist_h)),
			Scalar(255, 0, 0),
			2, 8, 0
		);
	}
	namedWindow(title, CV_WINDOW_AUTOSIZE);
	moveWindow(title, 120, 120);
	imshow(title, histImage);

	waitKey(0);
	destroyWindow(title);
}

inline double square(double a) {
	return a * a;
}

Mat changeToBinary(Mat& img) {
	Mat binary_img = img.clone();

	double H[256] = { 0., };
	for (int y = 0; y < img.rows; y++)
	{
		unsigned char* ptr1 = img.data + (img.cols * y);
		for (int x = 0; x < img.cols; x++)
		{
			H[ptr1[x]] += 1.0;
		}
	}

	double NH[256];
	for (int i = 0; i < 256; i++)
	{
		NH[i] = double(H[i]) / (double(img.rows) * double(img.cols));
	}

	double mT = 0;
	for (int i = 0; i < 256; i++)
	{
		mT += i * NH[i];
	}

	double weightedAvg[256];
	for (int t = 0; t < 256; t++)
	{
		double w0 = 0;
		double w1 = 0;

		double m0 = 0;
		double m1 = 0;

		for (int i = 0; i <= t; i++)
		{
			w0 += NH[i];
			m0 += i * NH[i];
		}

		for (int i = t + 1; i < 256; i++)
		{
			w1 += NH[i];
			m1 += i * NH[i];
		}

		if (w0 > 0) {
			m0 = m0 / w0;
		}
		if (w1 > 0) {
			m1 = m1 / w1;
		}

		weightedAvg[t] = w0 * square(m0 - mT) + w1 * square(m1 - mT);
	}

	int threshold = argmaxIndex(weightedAvg);

	for (int y = 0; y < binary_img.rows; y++)
	{
		unsigned char* ptr1 = img.data + (img.cols * y);
		unsigned char* resultptr = binary_img.data + (binary_img.cols * y);
		for (int x = 0; x < binary_img.cols; x++)
		{
			if (ptr1[x] <= threshold)
			{
				resultptr[x] = 0;
			}
			else
			{
				resultptr[x] = 255;
			}
		}
	}
	return binary_img;
}

void CRGBDlg::OnBnClickedBtnOtsu()
{	
	Mat binary_img = changeToBinary(img);

	DisplayImage(binary_img, 3);
	img = binary_img;
	strcpy(imgName, "otsu");
}

void reverseImage(Mat& img) {
	unsigned char* ptr = img.data;
	if (ptr[0] == 255) //reverse해야
	{
		for (int y = 0; y < img.rows; y++)
		{
			unsigned char* ptr1 = img.data + (img.cols * y);
			for (int x = 0; x < img.cols; x++)
			{
				if (ptr1[x] == 0)
				{
					ptr1[x] = 255;
				}
				else
				{
					ptr1[x] = 0;
				}
			}
		}
	}
}

void CRGBDlg::OnBnClickedBtnReverse()
{
	reverseImage(img);
	DisplayImage(img, 3);
}

void chageColor(Mat& img, Mat &copy, int i)
{
	for (int y = 0; y < copy.rows; y++)
	{
		unsigned char* ptr1 = img.data + 3 * (img.cols*y);
		unsigned char* resultptr = copy.data + 3 * (copy.cols*y);
		for (int x = 0; x<copy.cols; x++)
		{
			// 이렇게 RGB값을 조정하여 그 범위 안에 있는 Rgb 픽셀값에 단색을 넣었다.
			//200 -> 160 -> 110
			//그림자
			//배경색을 초록으로 해도 결과가 바뀌므로 함부로 손대지 말자
			resultptr[3 * x + 0] = 0;
			resultptr[3 * x + 1] = 0;
			resultptr[3 * x + 2] = 0;
			resultptr[3 * x + i] = ptr1[3 * x + i];
		}
		
	}
	
}

Mat changeToGray(const Mat& img)
{
	Mat gray(img.rows, img.cols, CV_8U);
	for (int y = 0; y < img.rows; y++)
	{
		unsigned const char* ptr1 = img.data + 3 * (img.cols * y);
		unsigned char* resultptr = gray.data + (gray.cols * y);
		for (int x = 0; x < img.cols; x++)
		{
			resultptr[x] = (ptr1[3 * x + 0] + ptr1[3 * x + 1] + ptr1[3 * x + 2])/3;
		}
	}
	return gray;
}

int argmaxIndex(double* array)
{
	int maxIndex = 1;
	double maxElement = array[1];
	for (int i = 2; i < 255; i++)
	{
		if (maxElement < array[i])
		{
			maxIndex = i;
			maxElement = array[i];
		}

	}

	return maxIndex;
}

int max(int* array)
{
	int max = array[0];
	for (int i = 1; i < 9; i++)
	{
		if (max < array[i])
		{
			max = array[i];
		}
	}
	
	return max;
}

int min(int* array)
{
	int min = array[0];
	for (int i = 1; i < 9; i++)
	{
		if (min > array[i])
		{
			min = array[i];
		}
	}
	
	return min;
}

Mat dilation(Mat img)
{
	Mat img_copy = img.clone();

	int structElem[9] = { 1,1,1,1,1,1,1,1,1 };

	for (int y = 1; y < (img_copy.rows - 1); y++)
	{
		unsigned char* ptr1 = img.data + (img.cols * y);
		unsigned char* resultptr = img_copy.data + (img_copy.cols * y);
		int m = img.cols * sizeof(unsigned char);
		for (int x = 1; x < (img_copy.cols - 1); x++)
		{
			int g[9] = {
				(ptr1 - m)[x - 1] * structElem[0], (ptr1 - m)[x] * structElem[1], (ptr1 - m)[x + 1] * structElem[2],
				(ptr1)[x - 1] * structElem[3], (ptr1)[x] * structElem[4], (ptr1)[x + 1] * structElem[5],
				(ptr1 + m)[x - 1] * structElem[6], (ptr1 + m)[x] * structElem[7], (ptr1 + m)[x + 1] * structElem[8]
			};
			resultptr[x] = max(g);
		}
	}

	img = img_copy;
	return img;
}

Mat erosion(Mat img)
{
	Mat img_copy = img.clone();

	int structElem[9] = { 1,1,1,1,1,1,1,1,1 };

	for (int y = 1; y < (img_copy.rows - 1); y++)
	{
		unsigned char* ptr1 = img.data + (img.cols * y);
		unsigned char* resultptr = img_copy.data + (img_copy.cols * y);
		int m = img.cols * sizeof(unsigned char);
		for (int x = 1; x < (img_copy.cols-1); x++)
		{
			int g[9] = {
				(ptr1 - m)[x-1] * structElem[0], (ptr1 - m)[x] * structElem[1], (ptr1 - m)[x+1] * structElem[2],
				(ptr1)[x-1] * structElem[3], (ptr1)[x] * structElem[4], (ptr1)[x+1] * structElem[5],
				(ptr1 + m)[x-1] * structElem[6], (ptr1 + m)[x] * structElem[7], (ptr1 + m)[x+1] * structElem[8]
			};
			resultptr[x] = min(g);
		}
	}

	img = img_copy;
	return img;
}

Mat opening(Mat img)
{
	return dilation(erosion(img));
}

Mat closing(Mat img)
{
	return erosion(dilation(img));
}

void CRGBDlg::OnBnClickedBtnOpening()
{
	Mat img_copy = opening(img);
	DisplayImage(img_copy, 3);
	img = img_copy;
	strcpy(imgName, "opening");
}


void CRGBDlg::OnBnClickedBtnClosing()
{
	Mat img_copy = closing(img);
	DisplayImage(img_copy, 3);
	img = img_copy;
	strcpy(imgName, "closing");
}

void calCoord(int i, int *y, int *x) //x,y를 새로운 위치의 x,y로 이동시키는 함수
{
	switch (i)
	{
	case 0: 
		*x = *x + 1;
		break;
	case 1: 
		*y = *y + 1; 
		*x = *x + 1;
		break;
	case 2: 
		*y = *y + 1;
		break;
	case 3: 
		*y = *y + 1;
		*x = *x - 1;
		break;
	case 4: 
		*x = *x - 1;
		break;
	case 5: 
		*y = *y - 1; 
		*x = *x - 1;
		break;
	case 6: 
		*y = *y - 1;
		break;
	case 7: 
		*y = *y - 1;
		*x = *x + 1;
		break;
	}
}

void read_neighbor(int y, int x, int neighbor[8], Mat& img) //해당 픽셀을 가운데 두고 이를 둘러싼 주변의 8화소 Direction code부여. 8 connectivity
{
	neighbor[0] = img.at<uchar>(y, x + 1);
	neighbor[1] = img.at<uchar>(y + 1, x + 1);
	neighbor[2] = img.at<uchar>(y + 1, x);
	neighbor[3] = img.at<uchar>(y + 1, x - 1);
	neighbor[4] = img.at<uchar>(y, x - 1);
	neighbor[5] = img.at<uchar>(y - 1, x - 1);
	neighbor[6] = img.at<uchar>(y - 1, x);
	neighbor[7] = img.at<uchar>(y - 1, x + 1);
}

void BTracing(int y, int x, unsigned int label, int tag, Mat& labImage, Mat& img)
{
	int cur_orient, pre_orient, end_x, pre_x, end_y, pre_y, start_o, add_o;
	int neighbor[8];
	int LUT_BLabeling[8][8] = { {0,0,0,0,0,0,0,0},
	{0,0,0,0,0,1,0,0},
	{0,0,0,0,0,1,1,0},
	{0,0,0,0,0,1,1,1},
	{1,0,0,0,0,1,1,1},
	{1,1,0,0,0,1,1,1},
	{1,1,1,0,0,1,1,1},
	{1,1,1,1,0,1,1,1} };


	if (tag==FORWARD)
	{
		cur_orient = 0;
		pre_orient = 0;
	}
	else
	{
		cur_orient = 6;
		pre_orient = 6;

	}
	end_x = x;
	pre_x = x;
	end_y = y;
	pre_y = y;

	do {
		read_neighbor(y, x, neighbor, img); //해당 픽셀을 기준으로 8-neighborhood
		start_o = (8 + cur_orient - 2) % 8; //시작 인덱스 계산

		int i;
		for (i = 0; i < 8; i++) //시작 인덱스부터 차례로 검사
		{
			add_o = (start_o + i) % 8;
			if (neighbor[add_o] != 0) //시작 인덱스부터 차례로 검사해서 오브젝트 만나면 for문 벗어나
			{
				break;
			}
		}

		if (i < 8)
		{
			calCoord(add_o, &y, &x); //x,y 새로운 위치로 옮겨가. 차례로 검사해서 오브젝트 만난 위치로
			cur_orient = add_o;
		}

		if (LUT_BLabeling[pre_orient][cur_orient]) //Lookup Table. Boundary Tracking에서 올라가는 경우에 해당label부여
		{
			num_region[label]++;
			labImage.at<uchar>(pre_y,pre_x) = label;

		}
		pre_x = x;
		pre_y = y;
		pre_orient = cur_orient;

	} while ((y != end_y) || (x != end_x)); //같은 작업 반복. 처음 시작한 화소위치에 도달할 때까지 Boundary Tracking 진행
}

void LabelingwithBT(Mat& img, Mat& labImage)
{

	unsigned int labelnumber = 1; //영역을 표시, 구분해줌(2,3,...)
	unsigned char cur_p; //원래 영상(img)에서 오브젝트만 관심이 있으니 해당 화소가 오브젝트인지 확인 위한 변수
	unsigned int ref_p1 = 0; //8-connectivity에서 해당 픽셀 기준 왼쪽과 왼쪽위 화소의 label값으로 region start, propagation, hole을 판단한다
	unsigned int ref_p2 = 0;

	for (int i = 1; i < (img.rows - 1); i++)
	{
		for (int j = 1; j < (img.cols - 1); j++)
		{
			cur_p = img.at<uchar>(i, j);
			if (cur_p == 255) //object
			{
				ref_p1 = labImage.at<uchar>(i,j - 1);
				ref_p2 = labImage.at<uchar>(i - 1,j - 1);
				if (ref_p1 > 1) //propagation
				{
					num_region[ref_p1]++;
					labImage.at<uchar>(i,j) = ref_p1; //ref_p1 label값 그대로 전파받음
				}
				else if ((ref_p1 == 0) && (ref_p2 >= 2)) //hole
				{
					num_region[ref_p2]++;
					labImage.at<uchar>(i,j) = ref_p2;
					BTracing(i, j, ref_p2, BACKWARD, labImage, img);  //Boundary Tracing 진행
				}
				else if ((ref_p1 == 0) && (ref_p2 == 0)) //region start
				{
					if (labImage.at<uchar>(i, j) == 0) //해당 픽셀이 label을 부여받은 적이 없어야 labelnumber를 올린다
					{
						labelnumber++;
					}

					num_region[labelnumber]++;
					labImage.at<uchar>(i,j) = labelnumber; //labImage에 새로 부여한 영역번호 기록
					BTracing(i, j, labelnumber, FORWARD, labImage, img); //Boundary Tracing 진행
				}
			}
			else {
				labImage.at<uchar>(i,j) = 0; //background
			}
		}
	}

}

Contour* getContourPoints(int y, int x, Mat& img)
{
	Contour* contour = new Contour;
	int cur_orient, end_x, end_y, start_o, add_o;
	int neighbor[8];

	cur_orient = 0;

	end_x = x;
	end_y = y;

	contour->count = 0;
	contour->points = new ContourPoint[MAX_SIZE/50];

	do {
		read_neighbor(y, x, neighbor, img); //해당 픽셀을 기준으로 8-neighborhood
		contour->points[contour->count].x = x; //경계화소 위치값 contour배열에 저장
		contour->points[contour->count].y = y;
		contour->count++;
		start_o = (8 + cur_orient - 2) % 8; //시작 인덱스 계산

		int i;
		for (i = 0; i < 8; i++) //시작 인덱스부터 차례로 검사
		{
			add_o = (start_o + i) % 8;
			if (neighbor[add_o] != 0) //시작 인덱스부터 차례로 검사해서 오브젝트 만나면 for문 벗어나
			{
				break;
			}
		}

		if (i < 8)
		{
			calCoord(add_o, &y, &x); //x,y 새로운 위치로 옮겨가. 차례로 검사해서 오브젝트 만난 위치로
			cur_orient = add_o;
		}

	} while ((y != end_y) || (x != end_x)); //같은 작업 반복. 처음 시작한 화소위치에 도달할 때까지 Boundary Tracking 진행
	return contour;
}

Contour* contourTracing(Mat& img, Mat& copy)
{
	for (int i = 0; i < img.cols; i++) //img의 테두리 픽셀값 0으로 만듦. 이래야 boundary tracing을 할 때 문제가 안생김
	{
		img.at<uchar>(0, i) = 0;
		img.at<uchar>(img.rows - 1, i) = 0;
	}

	for (int i = 1; i < (img.rows - 1); i++)
	{
		img.at<uchar>(i, 0) = 0;
		img.at<uchar>(i, img.cols - 1) = 0;
	}


	Mat labImage(img.rows, img.cols, CV_8UC1, Scalar(0)); //영역을 숫자로 표시하고 구분한 영상
	LabelingwithBT(img, labImage); //픽셀을 하나씩 처리하면서 영역label 부여(labImage에 표시)

	int structElem[9] = { 1,1,1,1,1,1,1,1,1 }; //경계화소출력 위한 배열

	bool contourMeet = false;
	int contourStart_y = 0;
	int contourStart_x = 0;

	for (int y = 1; y < (copy.rows - 1); y++)
	{
		unsigned char* ptr1 = labImage.data + (labImage.cols * y);
		unsigned char* resultptr = copy.data + (copy.cols * y);
		int m = labImage.cols * sizeof(unsigned char);
		for (int x = 1; x < (copy.cols - 1); x++)
		{
			if (ptr1[x] > 1) //label이 부여된 화소만 관심있다. label이 부여된 화소 중 경계화소만을 찾는 것이므로
			{
				int g[9] = {
				(ptr1 - m)[x - 1] * structElem[0], (ptr1 - m)[x] * structElem[1], (ptr1 - m)[x + 1] * structElem[2],
				(ptr1)[x - 1] * structElem[3], (ptr1)[x] * structElem[4], (ptr1)[x + 1] * structElem[5],
				(ptr1 + m)[x - 1] * structElem[6], (ptr1 + m)[x] * structElem[7], (ptr1 + m)[x + 1] * structElem[8]
				};

				if (min(g) == 0) //structElem 요소들을 각각 곱했을 때 주변 화소 중 하나라도 0이 된다면 해당 화소는 경계에 있는 것이 된다. 그러므로 흰색으로 경계화소 출력
				{
					resultptr[x] = 255;

					if (contourMeet == false)
					{
						contourStart_y = y;
						contourStart_x = x;
						contourMeet = true;
					}
					
				}
				else
				{
					resultptr[x] = 0;
				}
			}
			
		}
	}

	if (contourMeet)
	{
		return getContourPoints(contourStart_y, contourStart_x, copy);
	}
	return NULL;
}

Mat makeContourImage(Mat& img, Contour* contour) {
	Mat c_img(img.rows, img.cols, CV_8UC1, Scalar(0));

	for (int i = 0; i < contour->count; i++)
	{
		unsigned char* ptr1 = c_img.data + (c_img.cols * contour->points[i].y);
		ptr1[contour->points[i].x] = 255;
	}
	return c_img;
}

void CRGBDlg::OnBnClickedBtnContourtracing()
{
	Mat img_copy(img.rows, img.cols, CV_8UC1, Scalar(0)); //img와 같은 크기로 픽셀값 0으로 채워진 unsigned char matrix생성
	g_contour = contourTracing(img, img_copy); //Boundary Tracing으로 영역구분 후 경계화소 출력함수
	//DisplayImage(img_copy, 3);
	img = img_copy;

	Mat temp = makeContourImage(img, g_contour);
	
	//printf("now displaying image made up of contour points\n");
	DisplayImage(temp, 3);

	if (strcmp(imgName, "opening") == 0 || strcmp(imgName, "closing") == 0) //imgName이 opening 또는 closing으로 생성된 이미지는
																			//Contour Tracing을 적용한 뒤 저장버튼을 누르면 각각 opening_contour.jpg, closing_contour.jpg로 저장됨
	{
		strcpy(imgName, strcat(imgName, "_contour"));
	}
	else
	{
		strcpy(imgName, "contour");
	}
}

void normalizeLCS(Contour* contour) //LCS 값 정규화. 표준편차로 나누기
{
	double avg = 0.0;
	double stddev = 0.0;

	for (int i = 0; i < contour->count; i++)
	{
		avg += contour->LCS[i];
	}
	avg = avg / contour->count;

	for (int i = 0; i < contour->count; i++) //편차제곱의 합
	{
		stddev += pow(contour->LCS[i] - avg, 2);
	}
	stddev = sqrt(stddev / contour->count); //표준편차

	for (int i = 0; i < contour->count; i++)
	{
		//contour->LCS[i] /= stddev;
		contour->LCS[i] = (contour->LCS[i] - avg) / stddev;
	}
}

void calcLCS(Contour* contour) {
	int leftIndex, rightIndex;
	double a, b, c; //두 점을 지나는 직선의 방정식. ax+by+c=0에서 계수
	double distance;
	ContourPoint* pts = contour->points;
	contour->LCS = new double[contour->count];

	for (int i = 0; i < contour->count; i++)
	{
		leftIndex = ((i + contour->count) - (windowSize - 1) / 2) % contour->count;
		rightIndex = (i + (windowSize - 1) / 2) % contour->count;

		if (pts[leftIndex].x == pts[rightIndex].x)
		{
			a = 1;
			b = 0;
			c = -(pts[leftIndex].x);
		}
		else
		{
			a = ((double)pts[rightIndex].y - (double)pts[leftIndex].y) / ((double)pts[rightIndex].x - (double)pts[leftIndex].x);
			b = -1;
			c = (double)pts[leftIndex].y - ((double)pts[leftIndex].x * a);
		}

		distance = fabs(a * pts[i].x + b * pts[i].y + c) / sqrt(pow(a, 2) + pow(b, 2));

		contour->LCS[i] = distance;
	}

	delete[] contour->points;
	normalizeLCS(contour); //LCS 정규화
}

bool readImages(const char* path, const char *shapename, Mat* images) {
	char filepath[100];
	wchar_t wpath[100];

	sprintf(filepath, "%s\\%s*", path, shapename);
	mbstowcs(wpath, filepath, strlen(filepath) + 1);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(wpath, &ffd);

	int i = 0;
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			_tprintf(_T("  %s   <DIR>\n"), ffd.cFileName);
			//printf("  %s   <DIR>\n", (char*)ffd.cFileName);
		}
		else
		{
			std::wstring ws(ffd.cFileName);
			std::string filename(ws.begin(), ws.end());
			std::string pathstr = std::string(path) + std::string("\\") + filename;
			std::cout << "Reading " << pathstr << "...\n";
			
			Mat img = cv::imread(pathstr);
			images[i++] = img;
			if (i == NUM_TRAIN_SAMPLES + NUM_TEST_SAMPLES) {
				return true;
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);
	return false;
}

bool readClassificationSamples() {
	char path[100];
	wchar_t wpath[100];

	for (int i = 0; i < NUM_CLASS; i++) {
		readImages("polygons", shapes[i], shapeImages[i]);
	}
	return true;
}

Contour* processImage(Mat& src) {
	Mat gray = changeToGray(src);
	Mat binary = changeToBinary(gray);
	reverseImage(binary);

	Mat c1_img(binary.rows, binary.cols, CV_8UC1, Scalar(0)); //img와 같은 크기로 픽셀값 0으로 채워진 unsigned char matrix생성
	Contour* contour = contourTracing(binary, c1_img); //Boundary Tracing으로 영역구분 후 경계화소 출력함수

	calcLCS(contour);
	return contour;
}

void trainClassification() {
	for (int c = 0; c < NUM_CLASS; c++) {
		for (int i = 0; i < NUM_TRAIN_SAMPLES; i++) {
			Mat& img = shapeImages[c][i];
			trainContours[c][i] = processImage(img);
			img.release(); // free memory
		}
	}
}

double distance(double a, double b)
{
	return fabs(a - b);
}

double minDis(double a, double b, double c, int* argMin)
{
	double min = a;
	*argMin = 1;
	if (b < min)
	{
		min = b;
		*argMin = 2;
	}
	if (c < min)
	{
		min = c;
		*argMin = 3;
	}
	return min;
}

int p[MAX_SIZE];

double match(Contour* a, Contour* b) { //dtw
	double dis;
	double minDissimilarity = 99999999.0;

	double** D = (double**)malloc(sizeof(double*) * a->count);
	int** G = (int**)malloc(sizeof(int*) * a->count);

	for (int i = 0; i < a->count; i++)
	{
		D[i] = (double*)malloc(sizeof(double) * b->count);
		G[i] = (int*)malloc(sizeof(int) * b->count);
	}

	for (int s = 0; s < 1/*a->count*/; s++) //shift
	{
		D[0][0] = distance(a->LCS[(s + 0) % a->count], b->LCS[0]); //initialization
		G[0][0] = 0;
		for (int j = 1; j < b->count; j++)
		{
			D[0][j] = D[0][j - 1] + distance(a->LCS[(s + 0) % a->count], b->LCS[j]);
			G[0][j] = 2; //방향코드. 위에서 오면 1, 오른쪽방향 2, 대각선방향 3
		}
		for (int i = 1; i < a->count; i++)
		{
			D[i][0] = D[i - 1][0] + distance(a->LCS[(s + i) % a->count], b->LCS[0]);
			G[i][0] = 1;
		}

		int argMin = 0;
		for (int i = 1; i < a->count; i++) //forward
		{
			for (int j = 1; j < b->count; j++)
			{
				D[i][j] = distance(a->LCS[(s + i) % a->count], b->LCS[j]) + minDis(D[i - 1][j], D[i][j - 1], D[i - 1][j - 1], &argMin);
				G[i][j] = argMin;
			}
		}

		int i = a->count - 1;
		int j = b->count - 1;
		int k = 0;
		while ((i != 0) && (j != 0)) {
			p[k] = G[i][j];
			switch (G[i][j])
			{
			case 1:
				i--;
				k++;
				break;
			case 2:
				j--;
				k++;
				break;
			case 3:
				i--;
				j--;
				k++;
				break;
			}
		}

		dis = D[a->count - 1][b->count - 1]; // / k;
		if (dis < minDissimilarity)
		{
			minDissimilarity = dis;
		}
	}

	for (int i = 0; i < a->count; i++)
	{
		free(D[i]);
		free(G[i]);
	}
	free(D);
	free(G);

	return minDissimilarity;
}

int testClassification(Mat& img) {
	Contour* contour = processImage(img);
	int min_c = 0;
	double min = 99999999.0;

	for (int c = 0; c < NUM_CLASS; c++) {
		for (int i = 0; i < NUM_TRAIN_SAMPLES; i++) {
			double dis = match(trainContours[c][i], contour);
			//printf("test: %d.%d => %f\n", c, i, dis);
			if (dis < min) {
				min = dis;
				min_c = c;
			}
		}
	}
	return min_c;
}

/*
void CRGBDlg::OnBnClickedBtnTrain()
{
	if (!readClassificationSamples("train", trainSamples, NUM_TRAIN_SAMPLES)) {
		return;
	}
	for (int c = 0; c < NUM_CLASS; c++) {
		trainClassification(c);
	}
}
*/

void CRGBDlg::OnBnClickedBtnClassify()
{
	printf("onBtnClickedClassify\n");
	if (!readClassificationSamples()) {
		return;
	}

	/*
	Contour *contour = processImage(img);
	Mat c_img = makeContourImage(img, contour);
	DisplayImage(c_img, 3);
	drawHistogram("LCS", contour->LCS, 0, contour->count, 1024);
	delete contour;
	return;
	*/

	trainClassification();

	int predictCount[NUM_CLASS][NUM_CLASS] = { 0, };
	for (int c = 0; c < NUM_CLASS; c++) {
		printf("--- testing for class %d ---\n", c);
		
		for (int i = 0; i < NUM_TEST_SAMPLES; i++) {
			int predict = testClassification(shapeImages[c][i+NUM_TRAIN_SAMPLES]);
			predictCount[c][predict]++;
		}
		for (int cp = 0; cp < NUM_CLASS; cp++) {
			printf("prediction to class %d = %d / %d\n", cp, predictCount[c][cp], NUM_TEST_SAMPLES);
		}
	}

	int testNum = NUM_CLASS * NUM_TEST_SAMPLES;
	int correct = 0;
	int reject = 0;
	for (int c = 0; c < NUM_CLASS; c++)
	{
		correct += predictCount[c][c];
	}
	int error = testNum - correct - reject;

	printf("\n--- 성능 분석 ---\n");
	printf("정인식률 = %lf\n", (double)correct/(double)testNum);
	printf("기각률 = %lf\n", (double)reject / (double)testNum);
	printf("오류율 = %lf\n", (double)error / (double)testNum);
}

double LCS[MAX_SIZE] = { 0, };
void CRGBDlg::OnBnClickedBtnLcs()
{
	calcLCS(g_contour);
	drawHistogram("LCS", g_contour->LCS, 0, g_contour->count, 1024);
}


void CRGBDlg::OnBnClickedBtnDetectshape()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int predict = testClassification(img);
	printf("\n해당 도형은 %s 입니다!!\n", shapes[predict]);
}
