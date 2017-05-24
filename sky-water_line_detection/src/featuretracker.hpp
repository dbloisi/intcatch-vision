/*
 * featuretracker.hpp
 *
 * Author: Domenico Daniele Bloisi
 * email: domenico.bloisi@gmail.com
 */

#include <iostream>
#include <string>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/features2d/features2d.hpp"

#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"

#include <chrono>

#include <iomanip>

#include "stationarypoint.hpp"

using namespace cv;
using namespace std;

class FeatureTracker
{
private:

    TermCriteria termcrit;
    Size subPixWinSize;
    Size winSize;
    int max_count;
    vector<bool> needToInit;

    Mat gray, prevGray, image;
    vector< vector<Point2f> > prev_points;
    vector< vector<Point2f> > points;

    vector< vector<Point2f> > hist_prev_points;
    vector< vector<Point2f> > hist_points;

    vector< vector<StationaryPoint> > history;
    //Point2f point;

    int slices;

    int init_counter;
    const int INIT_FRAMES = 60;

public:
    FeatureTracker();
    FeatureTracker(TermCriteria termcrit,
                   Size subPixWinSize,
                   Size winSize,
                   int max_count,
                   int slices);
    void process(cv::Mat& frame);
private:
    void computeSlice(cv::Mat& prev_slice, cv::Mat& slice, int slide_idx);
};
