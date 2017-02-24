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

#include <chrono>

using namespace cv;
using namespace std;

class SkyWaterDetector
{
private:

#if 0
    //acquisition
    static std::mutex mu;
    static std::condition_variable c_var;
    static bool ready;
    static bool processed;
    static bool quit;
#endif

    //input data
    VideoCapture *cap;
    bool is_live;
    string cap_file;
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

    //trackbar min hue
    const int min_hue_slider_max = 255;
    int min_hue_slider;
    double min_hue;

    //trackbar max hue
    const int max_hue_slider_max = 255;
    int max_hue_slider;
    double max_hue;

    int horizonline;
    float alpha;

public:
    SkyWaterDetector(string cap_file,
                     float alpha,
                     string outvideo_filename,
                     int max_length,
                     bool is_live,
                     bool is_gui);
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
    void on_min_h_trackbar(int value);
    static void on_min_h_trackbar(int value, void* userdata);
    void on_max_h_trackbar(int value);
    static void on_max_h_trackbar(int value, void* userdata);

    Mat computeMask(const Mat& frame, const Mat& I, const Mat& S);

    std::string get_current_time_and_date();

};

