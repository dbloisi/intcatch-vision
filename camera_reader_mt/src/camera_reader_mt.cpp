/*
 * camera_reader_MT.cpp
 *
 * Author: Domenico Daniele Bloisi
 * email: domenico.bloisi@gmail.com
 */

#include <string>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <chrono>

#include <iostream>
#include <iomanip>
#include <ctime>

#include <wiringPi.h>

using namespace cv;
using namespace std;

std::mutex mu;
std::condition_variable c_var;
bool ready = false;
bool processed = false;
bool quit = false;

//input data
VideoCapture *cap;
bool is_live =false;
//output data
bool out_set = false;
string outvideo_filename = "";
int MAX_LENGTH = -1;
int out_cnt = -1;
int out_frame_n = -1;
//gui
bool is_gui =false;
//raspberry pi
bool using_pi = false;

/**
*
* function headers
*
**/
void help();
void acquisition();
void on_line();
void off_line();

std::string get_current_time_and_date();

/**
*
* main function
*
**/
int main(int argc, char* argv[])
{
    bool in_set = false;	
    string cap_file = "";
    	
    //print help information
    help();

    //check for the input parameter correctness
    if(argc < 3) {
        cerr <<"Incorret input list" << endl;
        cerr <<"exiting..." << endl;
        return EXIT_FAILURE;
    }
    
    for (int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "-in") == 0) {
            cap_file.assign(argv[++i]);
            in_set = true;
        }
        else if(strcmp(argv[i], "-out") == 0) {
            out_set = true;
            out_cnt = 0;
        }
        else if(strcmp(argv[i], "-live") == 0) {
            is_live = true;
        }
        else if(strcmp(argv[i], "-gui") == 0) {
            is_gui = true;
        }
        else if(strcmp(argv[i], "-n") == 0) {
            std::istringstream iss(argv[++i]);
            iss >> MAX_LENGTH;
        }
        else if(strcmp(argv[i], "-pi") == 0) {
            using_pi = true;
        }
        else {
            //error in reading input parameters
            cerr <<"Please, check the input parameters." << endl;
            cerr <<"Exiting..." << endl;
            return EXIT_FAILURE;
        }
    }
	
    if(!in_set) {
        //error in reading input parameters
        cerr <<"Unable to set input source." << endl;
        cerr <<"Please, check the input parameters." << endl;
        cerr <<"Exiting..." << endl;
        return EXIT_FAILURE;
    }
    else if(MAX_LENGTH > 0 && !out_set) {
        //error in reading input parameters
        cerr <<"Parameter n set without defining an output source." << endl;
        cerr <<"Please, check the input parameters." << endl;
        cerr <<"Exiting..." << endl;
        return EXIT_FAILURE;
    }

    if(using_pi) {
        wiringPiSetup();
        pinMode(0, OUTPUT);
        pinMode (3, OUTPUT);
    }
    
    //input stream connection
    cout << "Connecting to the input stream...";
    cout.flush();
    VideoCapture _cap;
    do {
        if(using_pi) {
            digitalWrite(0, HIGH);
            delay(200);
            digitalWrite(0, LOW);    
            delay(200);
        }

        if(strcmp(cap_file.c_str(), "0") == 0) {
            _cap.open(0);
        }
        else {
            _cap.open(cap_file);
        }
    
    } while (!_cap.isOpened());

    cap = &_cap;
    cout << "[OK]" << endl;

    if(using_pi) {
        digitalWrite (3, HIGH);
    }
   
    double dWidth = cap->get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    double dHeight = cap->get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

    cout << "Input frame size: " << dWidth << " x " << dHeight << endl;

    double dFPS = cap->get(CV_CAP_PROP_FPS);
    
    
    if(dFPS != dFPS || dFPS > 30) { //check for nan value
        dFPS = 25.;
    }

    cout << "FPS: " << dFPS << endl;

    if(is_gui) {
        namedWindow("video", CV_WINDOW_AUTOSIZE);
    }

    if(is_live) {
        on_line();
    }
    else {
        off_line();
    }

    if (out_set) {
        cout << "Finished writing" << endl;
    }

    return EXIT_SUCCESS;
}

void acquisition()
{
    bool run = true;
    while(run) {
        std::unique_lock<std::mutex> lk(mu);
	c_var.wait(lk, []{return ready;});

        //std::cout<<"grabbing a frame\n";
        bool bSuccess = cap->grab(); // grab a new frame from video

        if (!bSuccess) //if not success, break loop
        {
            cout << "Cannot read a frame from video stream" << endl;
            quit = true;
        }
   
        processed = true;
	    
        lk.unlock();
        c_var.notify_one();
        
        std:chrono::microseconds mc(1);
        std::chrono::duration<double, std::micro> mc1 = mc;
        std::this_thread::sleep_for(mc1);

        if (quit) {
            run = false;
        }
    }       
}

void on_line() {
    
    cout << "LIVE ACQUISITION" << endl;
    VideoWriter _outputVideo;

    std::thread t(acquisition);

    Mat frame;

    //first acquisition
    cout << "Acquiring initial frame...";
    cout.flush();
    do
    {
        {
            std::lock_guard<std::mutex> lk(mu);
            ready = true;
        }        

        c_var.notify_one();

        {
            std::unique_lock<std::mutex> lk(mu);
            c_var.wait(lk, []{return processed;});
        }        

        cap->retrieve(frame);
    } while (!frame.data);

    cout << "[OK]" << endl;

    if(out_set) {

        cout << "Output video Filename: " << outvideo_filename << endl;
        cout << "codec (CV_FOURCC id): " << CV_FOURCC('D','I','V','X') << endl;
        cout << "fps: " << 25 << endl;
        cout << "frame size: " << frame.size() << endl;

        cout << "opening output video stream...";
        cout.flush();

        outvideo_filename = get_current_time_and_date();
        stringstream ss;
        ss << out_cnt;
        outvideo_filename.append("_"+ss.str()+".avi");
        
        _outputVideo.open(outvideo_filename,
                     CV_FOURCC('D','I','V','X'),
                     25,
                     frame.size(),
                     true);
        if (!_outputVideo.isOpened())
        {
            cout  << "Could not open the output video for writing: " << outvideo_filename << endl;
            exit(EXIT_FAILURE);
        }

        cout << "[OK]" << endl;        
        cout << "OUTPUT DATA will be written to: " << outvideo_filename << endl;
        out_frame_n = 0; 
        out_cnt++;		
    }

    bool run = true;
    while (run)
    {
        {
            std::lock_guard<std::mutex> lk(mu);
            ready = true;
        }        
   
        c_var.notify_one();
        
        {
            std::unique_lock<std::mutex> lk(mu);
            c_var.wait(lk, []{return processed;});
        }        

        cap->retrieve(frame);
        if (!frame.data)
        {
            cout << "Unable to read frame from input stream" << endl;
            break;
        }
        
        if(is_gui) {
            //get the frame number and write it on the current frame
            stringstream ss;
            rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
            ss << cap->get(1); //CV_CAP_PROP_POS_FRAMES
            string frameNumberString = ss.str();
            putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        }

        if(is_gui) {
            imshow("video", frame); //show the frame in "MyVideo" window
        }
        else {
            cout << ".";
            cout.flush();
        }

        if (out_set) {
            _outputVideo.write(frame);

            if(using_pi) {
                digitalWrite(3, HIGH);
                delay(20);
                digitalWrite(3, LOW);
            }

            out_frame_n++; 
        
            cout << "*";
            cout.flush();

            if(MAX_LENGTH > 0 && out_frame_n > MAX_LENGTH) {
                cout << endl;
                cout << "opening output video stream...";
                cout.flush();

                outvideo_filename = get_current_time_and_date();
                stringstream ss;
                ss << out_cnt;
                outvideo_filename.append("_"+ss.str()+".avi");
        
                _outputVideo.open(outvideo_filename,
                     CV_FOURCC('D','I','V','X'),
                     25,
                     frame.size(),
                     true);
                if (!_outputVideo.isOpened())
                {
                    cout  << "Could not open the output video for writing: " << outvideo_filename << endl;
                    exit(EXIT_FAILURE);
                }

                cout << "[OK]" << endl;        
                cout << "OUTPUT DATA will be written to: " << outvideo_filename << endl;
                out_frame_n = 0; 
                out_cnt++;
            }
        }
        
        if (is_gui && waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
            quit = true;
            run = false;
        }

    }
    t.join();
}

void off_line() {
    
    cout << "SINGLE THREAD ACQUISITION" << endl;

    VideoWriter _outputVideo;
    
    Mat frame;

    cap->read(frame);

    if (!frame.data)
    {
        cout << "Unable to read frame from input stream" << endl;
        return;
    }

    if(out_set) {

        cout << "Output video Filename: " << outvideo_filename << endl;
        cout << "codec (CV_FOURCC id): " << CV_FOURCC('D','I','V','X') << endl;
        cout << "fps: " << 25 << endl;
        cout << "frame size: " << frame.size() << endl;

        cout << "opening output video stream...";
        cout.flush();

        outvideo_filename = get_current_time_and_date();
        stringstream ss;
        ss << out_cnt;
        outvideo_filename.append("_"+ss.str()+".avi");
        
        _outputVideo.open(outvideo_filename,
                     CV_FOURCC('D','I','V','X'),
                     25,
                     frame.size(),
                     true);
        if (!_outputVideo.isOpened())
        {
            cout  << "Could not open the output video for writing: " << outvideo_filename << endl;
            exit(EXIT_FAILURE);
        }

        cout << "[OK]" << endl;        
        cout << "OUTPUT DATA will be written to: " << outvideo_filename << endl;
        out_frame_n = 0; 
        out_cnt++;		
    }
    
    bool run = true;
    while (run)
    {
        cap->read(frame); // get a new frame from camera
        if (!frame.data)
        {
            cout << "Unable to read frame from input stream" << endl;
            break;
        }
                
	if(is_gui) {
            //get the frame number and write it on the current frame
            stringstream ss;
            rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
            ss << cap->get(1); //CV_CAP_PROP_POS_FRAMES
            string frameNumberString = ss.str();
            putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        }

        if(is_gui) {
            imshow("video", frame);
        }
        else {
            cout << ".";
            cout.flush();
        }

        if (out_set) {
            _outputVideo.write(frame);

            if(using_pi) {
                digitalWrite(3, HIGH);
                delay(20);
                digitalWrite(3, LOW);
            }

            out_frame_n++; 
        
            cout << "*";
            cout.flush();

            if(MAX_LENGTH > 0 && out_frame_n > MAX_LENGTH) {
                cout << endl;
                cout << "opening output video stream...";
                cout.flush();

                outvideo_filename = get_current_time_and_date();
                stringstream ss;
                ss << out_cnt;
                outvideo_filename.append("_"+ss.str()+".avi");
        
                _outputVideo.open(outvideo_filename,
                     CV_FOURCC('D','I','V','X'),
                     25,
                     frame.size(),
                     true);
                if (!_outputVideo.isOpened())
                {
                    cout  << "Could not open the output video for writing: " << outvideo_filename << endl;
                    exit(EXIT_FAILURE);
                }

                cout << "[OK]" << endl;        
                cout << "OUTPUT DATA will be written to: " << outvideo_filename << endl;
                out_frame_n = 0; 
                out_cnt++;
            }
        }
        
        if (is_gui && waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
            quit = true;
            run = false;
        }
    }
}

void help()
{
    cout
    << endl
    << "----------------------------------------------------------------------------"           << endl
    << "This program capture images from a camera "                                             << endl
    << "showing and storing them."                                                              << endl
    << endl
    << "Usage:"                                                                                 << endl
    << "./camera_reader_mt -in <source> {-out | -gui | -live | -n <max frames> | -pi}"                << endl
    << endl
    << "Examples:"                                                                              << endl
    << "  ./camera_reader_mt -in http://10.5.5.9:8080/live/amba.m3u8 -out -live"                << endl
    << endl
    << "  ./camera_reader_mt -in video.mp4 -gui"                                                << endl
    << "----------------------------------------------------------------------------"           << endl
    << endl;
}

std::string get_current_time_and_date()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    char timeformatted[256];

    if(0 < strftime(timeformatted, sizeof(timeformatted), "Y%Y-M%m-D%d-H%H-M%M-S%S",
          std::localtime(&in_time_t)))

    cout << timeformatted << endl;


    std::stringstream ss;
    //ss << std::put_time(std::localtime(&in_time_t), "Y%Y-M%m-D%d-H%H-M%M-S%S");
    ss << timeformatted;
    return ss.str();
}


