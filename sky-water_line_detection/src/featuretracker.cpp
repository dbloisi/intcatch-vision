/*
 * featuretracker.cpp
 *
 * Author: Domenico Daniele Bloisi
 * email: domenico.bloisi@gmail.com
 */

#include "featuretracker.hpp"

FeatureTracker::FeatureTracker()
{
    termcrit.type = TermCriteria::COUNT|TermCriteria::EPS;
    termcrit.maxCount = 20;
    termcrit.epsilon = 0.03;
    subPixWinSize.width = 10;
    subPixWinSize.height = 10;
    winSize.width = 31;
    winSize.height = 31;
    max_count = 500;
    needToInit = true;
    slices = 1;
}

FeatureTracker::FeatureTracker(TermCriteria termcrit,
                               Size subPixWinSize,
                               Size winSize,
                               int max_count,
                               bool needToInit)
{
    this->termcrit.type = termcrit.type;
    this->termcrit.maxCount = termcrit.maxCount;
    this->termcrit.epsilon = termcrit.epsilon;
    this->subPixWinSize.width = subPixWinSize.width;
    this->subPixWinSize.height = subPixWinSize.height;
    this->winSize.width = winSize.width;
    this->winSize.height = winSize.height;
    this->max_count = max_count;
    this->needToInit = needToInit;
    slices = 1; 
}

void FeatureTracker::process(cv::Mat& frame) {
    int w = frame.cols / slices;
    for(int i = 0; i < slices; ++i) {

        cout << "sono qui" << endl;
        cout << Rect(w * i, 0, w, frame.rows) << endl;

        Mat slice(frame, Rect(w * i, 0, w * (i+1), frame.rows));

        computeSlice(slice);
	
    }//for slices
}

void FeatureTracker::setSlices(int n) {
    slices = n;
}

void FeatureTracker::computeSlice(cv::Mat& slice) {
    slice.copyTo(image);
    cvtColor(image, gray, COLOR_BGR2GRAY);  
    if( needToInit )
    {
	// automatic initialization
	goodFeaturesToTrack(gray, points[1], max_count, 0.01, 10, Mat(), 3, 0, 0.04);
	cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit);
    }
    else if( !points[0].empty() )
    {
	vector<uchar> status;
	vector<float> err;
	if(prevGray.empty()) {
	    gray.copyTo(prevGray);
	}
	calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1],
	                     status, err, winSize,
	                     3, termcrit, 0, 0.001);
	size_t i, k;
	for( i = k = 0; i < points[1].size(); i++ )
	{
	    if( !status[i] )
	        continue;

	    points[1][k++] = points[1][i];
	    circle( image, points[1][i], 3, Scalar(0,255,0), -1, 8);
	}
	points[1].resize(k);
    }

	
    if(points[1].size() < 10) {
	needToInit = true;
    }
    else {
	needToInit = false;
    }
    imshow("LK Demo", image);

    //image.copyTo(frame);
	
    std::swap(points[1], points[0]);
    cv::swap(prevGray, gray);
}

