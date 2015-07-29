#if 0
#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include "iostream"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "objdetect\objdetect.hpp"
#include "video\background_segm.hpp"
#include "vector"
using namespace std;
using namespace cv;
//++++++++++++++++++++++++++++++++++++++++++++
const int IMAGE_WIDTH = 500;
const int IMAGE_HEIGHT = 300;
Size scale(IMAGE_WIDTH, IMAGE_HEIGHT);
const int shift_xy[9] = { -1, 0, 1, -1, 1, -1, 0, 1, 0 };
const int defaultRadius = 20;//ǰ���ж��뾶
const int defaultReqMatches = 2;	//#minָ��
const int defaultfactor = 16; //����p
const int NB = 50; //ǰ��������
const int defaultsamples = 20;//ÿ�����ص㶼��һ������Ϊ20�����������ı���������
float backmodel[IMAGE_HEIGHT][IMAGE_WIDTH][defaultsamples + 1];
//********************************************
void initialbm(Mat &M){
	if (M.empty())
	{
		cout << "initial failed" << endl;
		return;
	}
	RNG rng;
	//ÿ�����ص㶼Ҫ��ʼ��
	int x, y, k;
	for (y = 0; y<M.rows; y++)
	{
		for (x = 0; x<M.cols; x++)
		{
			backmodel[y][x][defaultsamples] = 0;//ǰ������������������ص����ͳ�ƣ�����������ص�����K�α����Ϊǰ�����������Ϊ�����㡣
			for (k = 0; k < defaultsamples; k++)//��ͼ�����������г�ʼ�����������ھ������ѡ��һ������г�ʼ�����ظ�defaultsamples = 20��
			{
				int c;
				int r;
				int s_x = rng.uniform(0, 9);
				int s_y = rng.uniform(0, 9);
				if (x>0)
				{
					if (x < M.cols - 1)
					{
						c = x + shift_xy[s_x];
					}
					else
					{
						c = M.cols - 1;
					}
				}
				else
				{
					c = 0;
				}

				if (y>0)
				{
					if (y<M.rows - 1)
					{
						r = y + shift_xy[s_y];
					}
					else
					{
						r = M.rows - 1;
					}
				}
				else
				{
					r = 0;
				}
				backmodel[y][x][k] = M.at<uchar>(r, c);
			}
		}
	}
}
void updatebm(Mat dst, Mat fgmask)
{
	//��ÿ�����ؽ��и��±���ģ��
	int hitcount;//��¼�뱳��ģ������ĸ���
	RNG rng;
	int a, b;
	for (int y = 0; y<dst.rows; y++)
	{
		for (int x = 0; x<dst.cols; x++)
		{
			int k = 0;
			hitcount = 0;
			while (k < defaultsamples && hitcount<defaultReqMatches)
			{
				b = dst.at<uchar>(y, x);
				if (abs(backmodel[y][x][k] - b) < defaultRadius)//defaultRadius�ǵ�ǰ����������ľ������ֵ�����ж�������(��defaultsamples = 20��)���ж��ٸ����뵱ǰ��������
					hitcount++;
				k++;
			}
			if (hitcount >= defaultReqMatches)//����뵱ǰ���������ĵ����������defaultReqMatches�����ʾ��ǰ��Ӧ��Ϊ����
			{//���ж�Ϊ�������أ�
				//���� �� ����ֵ��Ϊ0��
				backmodel[y][x][defaultsamples] = 0;
				//��ʱ����p�ĸ���ȥ���� ģ���� �����һ��������
				fgmask.at<uchar>(y, x) = 0;
				a = rng.uniform(0, defaultfactor);//����һ�������0-defaultfactor=16�����ȸ��ʲ���0-15��16������
				if (a == 0)//����������Ϊ0(��0-15�����ԣ�ֻ����1/16�ĸ��ʸ��µ�ǰ���������)����ѡ��һ������ĵ���и���
				{
					a = rng.uniform(0, defaultsamples);
					backmodel[y][x][a] = b;
					//cout<<"x="<<x<<",y="<<y<<",�����Լ�����b="<<b<<endl;
				}
				a = rng.uniform(0, defaultfactor);//��1/16�ĸ��ʸ����ھӵ��������
				if (a == 0){//�У�ȥ�����ھӱ���
					//cout<<"x="<<x<<",y="<<y<<",�����ھӱ���b="<<b<<endl;
					int s_x = rng.uniform(0, 9);
					int s_y = rng.uniform(0, 9);

					if (x>0){
						if (x < dst.cols - 1){
							s_x = x + shift_xy[s_x];
						}
						else{
							s_x = dst.cols - 1;
						}
					}
					else{
						s_x = 0;
					}
					if (y>0){
						if (y<dst.rows - 1){
							s_y = y + shift_xy[s_y];
						}
						else{
							s_y = dst.rows - 1;
						}
					}
					else{
						s_y = 0;
					}
					a = rng.uniform(0, defaultsamples);
					backmodel[s_y][s_x][a] = b;//ѡ��һ���ھӸ����������һ��������
				}
			}
			else{//�ж�Ϊǰ������
				fgmask.at<uchar>(y, x) = 255;
				backmodel[y][x][defaultsamples]++;//ǰ������
				if (backmodel[y][x][defaultsamples]>NB){//����õ㱻��Ϊǰ���Ĵ������࣬˵�����¶��������˸õ㣬���Ҹ����峤ʱ�侲ֹ���������Ϊ����
					//������������NB �� �뽫������Ϊ������
					fgmask.at<uchar>(y, x) = 0;
					a = rng.uniform(0, defaultfactor);
					if (a == 0){//���µ�ǰ���������
						a = rng.uniform(0, defaultsamples);
						backmodel[y][x][a] = b;
					}
				}
			}
		}

	}
}
void findRect(Mat mask,Mat &src)
{

	vector<vector<Point> > vecRect;//ǰ�������ľ��ο�
	findContours(mask, vecRect, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);//�����������صĿ򱣴���vecRect��
	if (vecRect.empty())return;
	double area = 0;
	const double MIN_AREA = 50;
	for (int i = 0; i < vecRect.size(); ++i)
	{
		if (contourArea(vecRect[i]) < MIN_AREA)//������ο�����С��MIN_AREA��������ʾ����һ��ϸС�ĸ��ŵ�
			continue;
		else
		{
			Rect roiRect = boundingRect(vecRect[i]);
			Mat objectImg = src(roiRect);
			rectangle(src, roiRect, Scalar(0, 0, 255), 3);
			//imshow("objectImg",objectImg);
			//waitKey(0);
		}
	}
}

void main(){
	char *path = "d:\\dgq.avi";
	VideoCapture cap;
	cap.open(path);
	if (!cap.isOpened())
	{
		cout << "no file" << endl;
	}
	Mat img, dst;
	dst.create(scale, CV_32FC1);
	cap >> img;
	resize(img, img, scale);//��ԭͼ���ߴ�ת����������ԭͼ������̫��
	cvtColor(img, dst, CV_BGR2GRAY);//ǰ��ͼ�񣬱�ʾǰ������
	Mat fgmask(dst.size(), CV_8UC1, Scalar(0));
	clock_t t1 = clock();
	initialbm(dst);//��ʼ������ģ��backmodel
	clock_t t2 = clock();
	cout << t2 - t1 << endl;//��ģʱ��

	float fps = cap.get(CV_CAP_PROP_FPS);
	float vps = 1000 / fps;
	do
	{
		cap >> img;
		if (img.empty())
		{
			break;
		}
		resize(img, img, scale);
		cvtColor(img, dst, CV_BGR2GRAY);
		updatebm(dst, fgmask);
		erode(fgmask, fgmask, Mat(), Point(), 1);//��ʴ
		dilate(fgmask, fgmask, Mat(), Point(), 5);//����
		erode(fgmask, fgmask, Mat(), Point(), 4);
		Mat tempFgmask = fgmask.clone();

		findRect(tempFgmask, img);
		imshow("dst", dst);
		imshow("fgmask", fgmask);
		imshow("img", img);
		if (waitKey(vps) > 0)
		{
			break;
		}
	} while (!img.empty());

}
#endif
