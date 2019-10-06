#include <iostream>
#include <fstream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp> 
using namespace cv;
using namespace std;
int main(int argc, char** argv)
{
	const string keys =
		"{ h help |      | print this help message }"
		"{ @image |<none>| path to image file }";
	CommandLineParser parser(argc, argv, keys);

	// file pointer 
	fstream fout;

	// opens an existing csv file or creates a new file. 
	fout.open("reportcard.csv", ios::out | ios::app);

	if (parser.has("help"))
	{
		parser.printMessage();
		return 0;
	}
	string filename = parser.get<string>("@image");
	if (!parser.check())
	{
		parser.printErrors();
		return 0;
	}
	VideoCapture capture(0);
	//VideoCapture capture(filename);
	if (!capture.isOpened()) {
		//error in opening the video input
		cerr << "Unable to open file!" << endl;
		return 0;
	}
	// Create some random colors
	vector<Scalar> colors;
	RNG rng;
	for (int i = 0; i < 100; i++)
	{
		int r = rng.uniform(0, 256);
		int g = rng.uniform(0, 256);
		int b = rng.uniform(0, 256);
		colors.push_back(Scalar(r, g, b));
	}
	Mat old_frame, old_gray;
	vector<Point2f> p0, p1;
	// Take first frame and find corners in it
	capture >> old_frame;
	cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
	goodFeaturesToTrack(old_gray, p0, 1, 0.3, 7, Mat(), 7, false, 0.04);
	// Create a mask image for drawing purposes
	Mat mask = Mat::zeros(old_frame.size(), old_frame.type());
	while (true) {
		//cout << "vedio is playing:" << endl;
		Mat frame, frame_gray;
		capture >> frame;
		if (frame.empty())
			break;
		cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
		// calculate optical flow
		vector<uchar> status;
		vector<float> err;
		TermCriteria criteria = TermCriteria((TermCriteria::COUNT) + (TermCriteria::EPS), 10, 0.03);
		calcOpticalFlowPyrLK(old_gray, frame_gray, p0, p1, status, err, Size(15, 15), 2, criteria);
		vector<Point2f> good_new;


		//cout << p0;
		//cout << p1 << endl;




		for (uint i = 0; i < p0.size(); i++)
		{
			// Select good points
			if (status[i] == 1) {
				good_new.push_back(p1[i]);

				//data save csv
				fout << "x" << i << "-" << p0[i].x << "   ";
				fout << "y" << i << "-" << p0[i].y << "   ";
				cout << p0[i];

				//check moving direction
				if ((p0[i].x <= 240) && (p1[i].x > 240) || (p0[i].x >= 1680) && (p1[i].x < 1680)) {
					fout << "move from back to front    ";
				}
				else if ((p0[i].x >= 240) && (p1[i].x < 240) || (p0[i].x <= 1680) && (p1[i].x > 1680)) {
					fout << "move from front to back    ";
				}

				// draw the tracks
				line(mask, p1[i], p0[i], colors[i], 2);
				circle(frame, p1[i], 5, colors[i], -1);
				//w return 0;
			}
		}
		fout << "Ok" << endl;
		cout << "ok" << endl;

		Mat img;
		add(frame, mask, img);
		line(img, Point(1680, 0), Point(1680, 960), Scalar(255, 0, 0), 10, CV_AA);
		line(img, Point(1200, 0), Point(1200, 960), Scalar(255, 0, 0), 10, CV_AA);
		line(img, Point(720, 0), Point(720, 960), Scalar(255, 0, 0), 10, CV_AA);
		line(img, Point(240, 0), Point(240, 960), Scalar(255, 0, 0), 10, CV_AA);

		imshow("Frame", img);

		int keyboard = waitKey(30);
		if (keyboard == 'q' || keyboard == 27)
			break;
		// Now update the previous frame and previous points
		old_gray = frame_gray.clone();
		p0 = good_new;
	}
}

