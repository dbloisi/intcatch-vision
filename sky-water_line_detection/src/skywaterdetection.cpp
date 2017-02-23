/*
 * skywaterdetection.cpp
 *
 * Author: Domenico Daniele Bloisi
 * email: domenico.bloisi@gmail.com
 */

#include "skywaterdetection.hpp"

SkyWaterDetector::SkyWaterDetector(string cap_file,
                                   float alpha,
                                   string outvideo_filename,
                                   bool is_live,
                                   bool is_gui)
{
    ready = false;
    processed = false;
    quit = false;
    this->cap_file.assign(cap_file);
    this->alpha = alpha;
    this->is_live = is_live;
    if(outvideo_filename.empty()) {
        out_set = false;
    }
    else {
        this->outvideo_filename.assign(outvideo_filename);
        out_set = true;
    }
    this->is_gui = is_gui;

    VideoCapture _cap;     
    _cap.open(cap_file);
    
    if (!_cap.isOpened())  // if not success, exit program
    {
        cout << "Cannot open the video file " << cap_file << endl;
        exit(EXIT_FAILURE);
    }

    cout << "input stream open" << endl;
    cap = &_cap;

    double dWidth = cap->get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    double dHeight = cap->get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

    cout << "Frame size: " << dWidth << " x " << dHeight << endl;

    double dFPS = cap->get(CV_CAP_PROP_FPS);
    
    
    if(dFPS != dFPS || dFPS > 30) { //check for nan value
        dFPS = 25.;
    }

    cout << "FPS: " << dFPS << endl;

    min_saturation_slider = 10;
    max_saturation_slider = 50;
    min_brightness_slider = 30;
    max_brightness_slider = 100;
    min_hue_slider = 0;
    max_hue_slider = 255;

    if(is_gui) {

        namedWindow("video", CV_WINDOW_AUTOSIZE);

        namedWindow("Saturation", CV_WINDOW_AUTOSIZE);
   
        char min_s_trackbarName[50];
        sprintf( min_s_trackbarName, "min S");
        createTrackbar( min_s_trackbarName, "Saturation", &min_saturation_slider, min_saturation_slider_max, on_min_s_trackbar );
        on_min_s_trackbar( min_saturation_slider, 0 );

        char max_s_trackbarName[50];
        sprintf( max_s_trackbarName, "max S");
        createTrackbar( max_s_trackbarName, "Saturation", &max_saturation_slider, max_saturation_slider_max, on_max_s_trackbar );
        on_max_s_trackbar( max_saturation_slider, 0 );

        namedWindow("Brightness", CV_WINDOW_AUTOSIZE);
    
        char min_b_trackbarName[50];
        sprintf( min_b_trackbarName, "min B");
        createTrackbar( min_b_trackbarName, "Brightness", &min_brightness_slider, min_brightness_slider_max, on_min_b_trackbar );
        on_min_b_trackbar( min_brightness_slider, 0 );

        char max_b_trackbarName[50];
        sprintf( max_b_trackbarName, "max B");
        createTrackbar( max_b_trackbarName, "Brightness", &max_brightness_slider, max_brightness_slider_max, on_max_b_trackbar );
        on_max_b_trackbar( max_brightness_slider, 0 );

        namedWindow("Hue", CV_WINDOW_AUTOSIZE);
    
        char min_h_trackbarName[50];
        sprintf( min_h_trackbarName, "min H");
        createTrackbar( min_h_trackbarName, "Hue", &min_hue_slider, min_hue_slider_max, on_min_h_trackbar );
        on_min_h_trackbar( min_hue_slider, 0 );
    
        char max_h_trackbarName[50];
        sprintf( max_h_trackbarName, "max H");
        createTrackbar( max_h_trackbarName, "Hue", &max_hue_slider, max_hue_slider_max, on_max_h_trackbar );
        on_max_h_trackbar( max_hue_slider, 0 );
    } //is_gui


}


void SkyWaterDetector::detect()
{
    if(is_live) {
        on_line();
    }
    else {
        off_line();
    }
}

int SkyWaterDetector::getHorizonline() {
    return horizonline;
}

void SkyWaterDetector::acquisition() {
    bool run = true;
    while(run) {
        std::unique_lock<std::mutex> lk(mu);
	c_var.wait(lk, []{return ready;});

	    //for(int i = 0; i < 5; ++i) {
		    bool bSuccess = true;
		    //std::cout<<"grabbing a frame\n";
		    bSuccess = cap->grab(); // grab a new frame from video

		    if (!bSuccess) //if not success, break loop
		    {
		        cout << "Cannot read a frame from video stream" << endl;
		    }
	    //}
	    
	    processed = true;
	    
	    lk.unlock();
	    c_var.notify_one();
        
        std:chrono::milliseconds ms(1);
        std::chrono::duration<double, std::milli> ms3 = ms;
        std::this_thread::sleep_for(ms3);

        if (quit) {
            run = false;
        }
    }       
}

void SkyWaterDetector::on_line() {
    
    cout << "LIVE ACQUISITION" << endl;
    VideoWriter _outputVideo;

    std::thread t(acquisition);

    Mat frame;     //current frame


    if(out_set) {
        
        _outputVideo.open(outvideo_filename,
                     CV_FOURCC('D','I','V','X'),
                     10,
                     frame.size(),
                     true);
        if (!_outputVideo.isOpened())
        {
            cout  << "Could not open the output video for writing: " << outvideo_filename << endl;
            exit(EXIT_FAILURE);
        }
        
        cout << "OUTPUT DATA will be written to: " << outvideo_filename << endl;		
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
        		
        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        ss << cap->get(1); //CV_CAP_PROP_POS_FRAMES
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks

        imshow("video", frame); //show the frame in "MyVideo" window

        if (out_set) {
            _outputVideo.write(frame);
        }
        
        if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
            quit = true;
            run = false;
        }

    }
    t.join();
}


void SkyWaterDetector::off_line() {

}

#if 0




void off_line() {
    
    cout << "SINGLE THREAD ACQUISITION" << endl;

    VideoWriter _outputVideo;
    
    Mat frame;     //current frame

    cap->read(frame); // get a new frame from camera
    if (!frame.data)
    {
        cout << "Unable to read frame from input stream" << endl;
        return;
    }

    if(out_set) {
        
        _outputVideo.open(outvideo_filename,
                     CV_FOURCC('D','I','V','X'),
                     10,
                     frame.size(),
                     true);
        if (!_outputVideo.isOpened())
        {
            cout  << "Could not open the output video for writing: " << outvideo_filename << endl;
            exit(EXIT_FAILURE);
        }
        
        cout << "OUTPUT DATA will be written to: " << outvideo_filename << endl;		
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
                
	Mat I(frame.rows, frame.cols, CV_8UC1);
        Mat S(frame.rows, frame.cols, CV_8UC1);

        Mat H(frame.rows, frame.cols, CV_8UC1);
        Mat hsv;
        cvtColor( frame, hsv, CV_BGR2HSV );
        
        for(int i = 0; i < frame.rows; ++i) {
            for(int j = 0; j < frame.cols; ++j) {

                uchar b = cvRound((frame.at<Vec3b>(i,j)[0] +
                           frame.at<Vec3b>(i,j)[1] +
                           frame.at<Vec3b>(i,j)[2] ) / 3.f);


                float s = 255.f * (1.f - (std::min(frame.at<Vec3b>(i,j)[0],
                                         std::min(frame.at<Vec3b>(i,j)[1], frame.at<Vec3b>(i,j)[2]))
                                     / (float)b));

                if(b >= min_brightness && b <= max_brightness) { 
                    I.at<uchar>(i,j) = b;
                }
                else {
                    I.at<uchar>(i,j) = 0;
                }
                if(s >= min_saturation && s <= max_saturation) { 
                    S.at<uchar>(i,j) = (uchar)s;
                }
                else {
                    S.at<uchar>(i,j) = 0;
                }

#if HUE
                uchar h = hsv.at<Vec3b>(i,j)[0];
                if(h >= min_hue && h <= max_hue) { 
                    H.at<uchar>(i,j) = h;
                }
                else {
                    H.at<uchar>(i,j) = 0;
                }
#endif

            }
            

        }

        // Apply the colormap
        Mat cm_I;
        applyColorMap(I, cm_I, COLORMAP_HOT);
        Mat cm_S;
        applyColorMap(S, cm_S, COLORMAP_HOT);
        Mat cm_H;
        applyColorMap(H, cm_H, COLORMAP_HOT);

        Mat mask = computeMask(frame,I,S);
        Mat mask_gray;
        cvtColor( mask, mask_gray, CV_BGR2GRAY );

#if FEATURES
        ///Parameters for Shi-Tomasi algorithm
        vector<Point2f> corners;
        double qualityLevel = 0.1;
        double minDistance = 10;
        int blockSize = 3;
        bool useHarrisDetector = false;
        double k = 0.04;
        int maxCorners = 200;
        
        goodFeaturesToTrack( frame_gray,
               corners,
               maxCorners,
               qualityLevel,
               minDistance,
               Mat(),
               blockSize,
               useHarrisDetector,
               k );

        cout<<"** Number of corners detected: "<<corners.size()<<endl;
        int r = 4;
        for( int i = 0; i < corners.size(); i++ )
        {
            circle( frame, corners[i], r, Scalar(0,255,0), -1, 8, 0 );
            circle( mask, corners[i], 20, Scalar(0), -1, 8, 0 );
        }
    
#endif
    
        Mat dst, cdst;
        Canny(mask_gray, dst, 50, 200, 3);
        cvtColor(dst, cdst, CV_GRAY2BGR);

        vector<Vec4i> lines;
        HoughLinesP(dst, lines, 1, CV_PI/180, 150, 150, 30 );

        for( size_t i = 0; i < lines.size(); i++ )
        {
            Vec4i l = lines[i];
            if(abs(l[1] - l[3]) < 20) {
                //line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
                int y = (l[1] + l[3]) / 2;
                //line( cdst, Point(0, y), Point(cdst.cols-1, y), Scalar(255,0,0), 1, CV_AA);
                int diff = 0;
                for(int i = y - 30, cnt = 0; i < y - 9; i+=10, ++cnt) {
                    for(int j = 20; j < cdst.cols - 21; j+=20) {
                        uchar b_1 = mask.at<Vec3b>(i,j)[0];
                        uchar g_1 = mask.at<Vec3b>(i,j)[1];
                        uchar r_1 = mask.at<Vec3b>(i,j)[2];
                        //circle(cdst, Point(j,i), 3, Scalar(0,255,0));

                        int ii;
                        switch(cnt) {
                            case 0:
                                ii = y+10;
                                //circle(cdst, Point(j,y+10), 3, Scalar(0,255,255));
                                break;
                            case 1:
                                ii = y+20;
                                //circle(cdst, Point(j,y+20), 3, Scalar(0,255,255));
                                break;
                            case 2:
                                ii = y+30;
                                //circle(cdst, Point(j,y+30), 3, Scalar(0,255,255));
                                break;
                        }
                        uchar b_2 = mask.at<Vec3b>(ii,j)[0];
                        uchar g_2 = mask.at<Vec3b>(ii,j)[1];
                        uchar r_2 = mask.at<Vec3b>(ii,j)[2];
                        if(abs(b_1 - b_2) > 20 ||
                           abs(g_1 - g_2) > 20 ||
                           abs(r_1 - r_2) > 20)
                        {
                            diff++;
                        }

                    }//for j
                }//for i

                //cout << "diff: " << diff << endl;
                if(diff > 30) {
                    //line( cdst, Point(0, y), Point(cdst.cols-1, y), Scalar(255,0,0), 3, CV_AA);

                    if(horizonline < 0) {
                        horizonline = y;
                    }
                    else {
                        horizonline = cvRound((alpha * y) + (1.0f - alpha) * horizonline);
                    }

                    break;
                }
            }//if

        }//for

        //imshow("detected lines", cdst);

        line( frame, Point(0, horizonline), Point(frame.cols-1, horizonline), Scalar(255,0,0), 3, CV_AA);
        
        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        ss << cap->get(1); //CV_CAP_PROP_POS_FRAMES
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));


        imshow("video", frame); 

        imshow("Brightness", cm_I); 
        imshow("Saturation", cm_S);

#if HUE
        imshow("Hue", cm_H);
#endif
        

        imshow("mask", mask); 

        if (out_set) {
            _outputVideo.write(frame);
        }
        
        if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed by user" << endl;
            quit = true;
            run = false;
        }

    }
}


#endif



/**
 * @function on_min_s_trackbar
 * @brief Callback for trackbar on min saturation
 */
void SkyWaterDetector::on_min_s_trackbar( int, void* )
{
    min_saturation = min_saturation_slider;
}

/**
 * @function on_max_s_trackbar
 * @brief Callback for trackbar on max saturation
 */
void SkyWaterDetector::on_max_s_trackbar( int, void* )
{
    max_saturation = max_saturation_slider;
}

/**
 * @function on_min_b_trackbar
 * @brief Callback for trackbar on min brightness
 */
void SkyWaterDetector::on_min_b_trackbar( int, void* )
{
    min_brightness = min_brightness_slider;
}

/**
 * @function on_max_b_trackbar
 * @brief Callback for trackbar on max brightness
 */
void SkyWaterDetector::on_max_b_trackbar( int, void* )
{
    max_brightness = max_brightness_slider;
}

/**
 * @function on_min_h_trackbar
 * @brief Callback for trackbar on min hue
 */
void SkyWaterDetector::on_min_h_trackbar( int, void* )
{
    min_hue = min_hue_slider;
}

/**
 * @function on_max_h_trackbar
 * @brief Callback for trackbar on max hue
 */
void SkyWaterDetector::on_max_h_trackbar( int, void* )
{
    max_hue = max_hue_slider;
}


Mat SkyWaterDetector::computeMask(const Mat& frame, const Mat& I, const Mat& S) {
    Mat mask(frame.rows, frame.cols, CV_8UC3);
    for(int i = 0; i < frame.rows; ++i) {
        for(int j = 0; j < frame.cols; ++j) {
            if(I.at<uchar>(i,j) > 0 && S.at<uchar>(i,j) > 0) {
                mask.at<Vec3b>(i,j) = frame.at<Vec3b>(i,j);
            }
            else {
                mask.at<Vec3b>(i,j) = Vec3b(0,0,0);
            }
        }
    }
    return mask;
}

