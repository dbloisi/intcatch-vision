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
    slices = 4;
    needToInit.resize(slices);
    for(int i = 0; i < slices; ++i) {
        needToInit[i] = true;
    }    

    prev_points.resize(slices);
    points.resize(slices);
    history.resize(slices);
}

FeatureTracker::FeatureTracker(TermCriteria termcrit,
                               Size subPixWinSize,
                               Size winSize,
                               int max_count,
                               int slices)
{
    this->termcrit.type = termcrit.type;
    this->termcrit.maxCount = termcrit.maxCount;
    this->termcrit.epsilon = termcrit.epsilon;
    this->subPixWinSize.width = subPixWinSize.width;
    this->subPixWinSize.height = subPixWinSize.height;
    this->winSize.width = winSize.width;
    this->winSize.height = winSize.height;
    this->max_count = max_count;
    this->slices = slices;

    needToInit.resize(slices);
    for(int i = 0; i < slices; ++i) {
        needToInit[i] = true;
    }

    points.resize(slices);
    history.resize(slices);
}

void FeatureTracker::process(cv::Mat& frame) {
    int w = frame.cols / slices;

    frame.copyTo(image);
    cvtColor(image, gray, COLOR_BGR2GRAY);
    if(prevGray.empty()) {
        gray.copyTo(prevGray);
        return;
    }

    for(int i = 0; i < slices; ++i) {
        cout << Rect(w * i, 0, w, gray.rows) << endl;

        Mat slice(gray, Rect(w * i, 0, w, gray.rows));

        Mat prev_slice(prevGray, Rect(w * i, 0, w, prevGray.rows));

        computeSlice(prev_slice, slice, i);
	
    }//for slices
    cv::swap(prevGray, gray);
}

void FeatureTracker::computeSlice(cv::Mat& prev_slice, cv::Mat& slice, int slice_idx) {
        
    if(needToInit[slice_idx])
    {
        goodFeaturesToTrack(slice, points[slice_idx], max_count, 0.01, 10, Mat(), 3, 0, 0.04);
	cornerSubPix(slice, points[slice_idx], subPixWinSize, Size(-1,-1), termcrit);

        /*
        for(int i = 0; i < points[slice_idx].size(); i++ )
	{
	    circle( image, points[slice_idx][i], 3, Scalar(0,255,0), -1, 8);
	}
	imshow("puntolini", image);
        waitKey(0);
        */
    }
    else if( !prev_points[slice_idx].empty() )
    {
	vector<uchar> status;
	vector<float> err;
	
	calcOpticalFlowPyrLK(prev_slice, slice, prev_points[slice_idx], points[slice_idx],
	                     status, err, winSize,
	                     3, termcrit, 0, 0.001);
	size_t i, k;
	for( i = k = 0; i < points[slice_idx].size(); i++ )
	{
	    if( !status[i] )
	        continue;

	    points[slice_idx][k++] = points[slice_idx][i];
	    circle(image,
                   Point(points[slice_idx][i].x + (slice_idx*slice.cols),
                         points[slice_idx][i].y), 
                   3, Scalar(0,255,0), -1, 8);
	}
	points[slice_idx].resize(k);
    }
	
    if(points[slice_idx].size() < 3) {
	needToInit[slice_idx] = true;
    }
    else {
	needToInit[slice_idx] = false;
    }
    imshow("LK Demo", image);

    //image.copyTo(frame);
	
    std::swap(points[slice_idx], prev_points[slice_idx]);
    
}

