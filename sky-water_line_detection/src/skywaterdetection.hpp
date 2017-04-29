/*
 * skywaterdetection.hpp
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


using namespace cv;
using namespace std;

class SkyWaterDetector
{
private:

    //acquisition
    std::mutex mu;
    std::condition_variable c_var;
    bool ready;
    bool processed;
    bool quit;
    
    std::thread acquisitionThread;

    //input data
    VideoCapture *cap;
    bool is_live;
    string cap_file;
    int in_frame_n;

    string calib_file;

    //output data
    bool out_set;
    string outvideo_filename;
    int MAX_LENGTH;
    int out_cnt;
    int out_frame_n;

    //gui
    bool is_gui;

    //trackbar min brightness
    const int min_brightness_slider_max = 255;
    int min_brightness_slider;
    double min_brightness;

    //trackbar max brightness
    const int max_brightness_slider_max = 255;
    int max_brightness_slider;
    double max_brightness;

    //trackbar min saturation
    const int min_saturation_slider_max = 255;
    int min_saturation_slider;
    double min_saturation;

    //trackbar max saturation
    const int max_saturation_slider_max = 255;
    int max_saturation_slider;
    double max_saturation;

    int horizonline;
    float alpha;

    //opticalflow
    TermCriteria termcrit;
    Size subPixWinSize;
    Size winSize;
    const int MAX_COUNT = 500;
    bool needToInit;
    bool nightMode;

    Mat gray, prevGray, image;
    vector<Point2f> points[2];
    Point2f point;

    Mat cameraMatrix, distCoeffs;

public:
    SkyWaterDetector(string cap_file,
                     float alpha,
                     string outvideo_filename,
                     int max_length,
                     bool is_live,
                     bool is_gui,
                     string calib_file);

    void detect();
    int getHorizonline();
 
private:
    void acquisition();
    void on_line();
    void off_line();

    void on_min_b_trackbar(int value);
    static void on_min_b_trackbar(int value, void* userdata);
    void on_max_b_trackbar(int value);
    static void on_max_b_trackbar(int value, void* userdata);
    void on_min_s_trackbar(int value);
    static void on_min_s_trackbar(int value, void* userdata);
    void on_max_s_trackbar(int value);
    static void on_max_s_trackbar(int value, void* userdata);    
    
    Mat computeMask(const Mat& frame, const Mat& I, const Mat& S);

    std::string get_current_time_and_date();

    void readCalibData(string calib_file);

    Mat colorAnalysis(Mat& frame);

    void opticalFlow(Mat& frame);

};

