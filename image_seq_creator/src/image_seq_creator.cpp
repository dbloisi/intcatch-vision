/*
 * image_seq_creator.cpp
 *
 * Author: Domenico Daniele Bloisi
 * email: domenico.bloisi@gmail.com
 */

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace cv;
using namespace std;

//input data
VideoCapture *cap;
bool is_live =false;
//output data
string out_dir = "images/";
int out_frame_n = -1;
//gui
bool is_gui =false;

int width = -1, height = -1;

/**
*
* function headers
*
**/
void help();
void process();
bool connect(string cap_file);

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
        else if(strcmp(argv[i], "-gui") == 0) {
            is_gui = true;
        }
        else if(strcmp(argv[i], "-w") == 0) {
            istringstream iss(argv[++i]);
            iss >> width;
        }
        else if(strcmp(argv[i], "-h") == 0) {
            istringstream iss(argv[++i]);
            iss >> height;
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
       
    connect(cap_file);

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

    //while(true) {
        process();
      //  connect(cap_file);
    //}

    return EXIT_SUCCESS;
}

void process() {
    
    cout << "PROCESS" << endl;
    
    Mat frame;

    if (!cap->read(frame))
    {
        cout << "Unable to read frame from input stream" << endl;
        return;
    }

    out_frame_n = 0;

    bool run = true;
    while (run)
    {
        if (!cap->read(frame))
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

        stringstream ss;
        ss << setfill('0') << setw(5) << out_frame_n;
        string imagename = out_dir + ss.str() + ".jpg";

        cout << "saving image: " << imagename << " ..." << endl;
        cout.flush();

        if(width > 0 && height > 0) {
            Mat out_frame(Size(width, height), CV_8UC3);
            resize(frame, out_frame, out_frame.size());
            Mat gray_frame;
            cvtColor(out_frame, gray_frame, CV_BGR2GRAY);
            frame = gray_frame.clone();
        }
        if(imwrite(imagename, frame)) {
            cout << "[OK]" << endl;
            out_frame_n++;
        }
        else {
            cout << endl;
            cout << "Unable to save the image" << endl;
            exit(-1);
        }
        
        if (is_gui && waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
        }
    }
}

void help()
{
    cout
    << endl
    << "----------------------------------------------------------------------------"           << endl
    << "This program create an image sequence from a camera "                                   << endl
    << "or a video."                                                                            << endl
    << endl
    << "Usage:"                                                                                 << endl
    << "./image_seq_creator -in <source>"                                                       << endl
    << endl
    << "Examples:"                                                                              << endl
    << "  ./image_seq_creator -in video.mp4"                                                    << endl
    << endl
    << "----------------------------------------------------------------------------"           << endl
    << endl;
}

bool connect(string cap_file) {
    //input stream connection
    cout << "Connecting to the input stream...";
    cout.flush();

    do {
        if(strcmp(cap_file.c_str(), "0") == 0) {
            cap = new VideoCapture(0);
        }
        else {
            cap = new VideoCapture(cap_file);
        }
    
    } while (!cap->isOpened());

    cout << "[OK]" << endl;
    return true;
}

