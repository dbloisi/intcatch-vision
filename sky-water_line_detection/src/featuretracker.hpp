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
    bool needToInit;

    Mat gray, prevGray, image;
    vector<Point2f> points[2];
    vector<StationaryPoint> history;
    Point2f point;

    int slices;

public:
    FeatureTracker();
    FeatureTracker(TermCriteria termcrit,
                   Size subPixWinSize,
                   Size winSize,
                   int max_count,
                   bool needToInit);
    void process(cv::Mat& frame);
    void setSlices(int n);
private:
    void computeSlice(cv::Mat& slice);
};

