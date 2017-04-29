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
                                   int max_length,
                                   bool is_live,
                                   bool is_gui,
                                   string calib_file)
{
    out_frame_n = 0;
   
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
        if(max_length > 0) {
            MAX_LENGTH = max_length;
        }
        else {
            MAX_LENGTH = -1;
            out_cnt = -1;
            out_frame_n = -1;
        }
    }
    this->is_gui = is_gui;

    this->calib_file.assign(calib_file);

    cap = new VideoCapture(cap_file);
    
    if (!cap->isOpened())  // if not success, exit program
    {
        cout << "Cannot open the video file " << cap_file << endl;
        exit(EXIT_FAILURE);
    }

    cout << "input stream open" << endl;

    double dWidth = cap->get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    double dHeight = cap->get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

    cout << "Frame size: " << dWidth << " x " << dHeight << endl;

    double dFPS = cap->get(CV_CAP_PROP_FPS);
    
    
    if(dFPS != dFPS || dFPS > 30) { //check for nan value
        dFPS = 25.;
    }

    cout << "FPS: " << dFPS << endl;

    min_saturation_slider = 0;
    max_saturation_slider = 50;
    min_brightness_slider = 0;
    max_brightness_slider = 150;

    if(is_gui) {

        namedWindow("video", CV_WINDOW_AUTOSIZE);

        namedWindow("Saturation", CV_WINDOW_AUTOSIZE);   
        char min_s_trackbarName[50];
        sprintf( min_s_trackbarName, "min S");
        createTrackbar( min_s_trackbarName, "Saturation", &min_saturation_slider, min_saturation_slider_max, on_min_s_trackbar, this );
        on_min_s_trackbar(min_saturation_slider, this);
        char max_s_trackbarName[50];
        sprintf( max_s_trackbarName, "max S");
        createTrackbar( max_s_trackbarName, "Saturation", &max_saturation_slider, max_saturation_slider_max, on_max_s_trackbar, this );
        on_max_s_trackbar(max_saturation_slider, this);
        
        namedWindow("Brightness", CV_WINDOW_AUTOSIZE);
        char min_b_trackbarName[50];
        sprintf( min_b_trackbarName, "min B");
        createTrackbar( min_b_trackbarName, "Brightness", &min_brightness_slider, min_brightness_slider_max, on_min_b_trackbar, this );
        on_min_b_trackbar(min_brightness_slider, this);
        char max_b_trackbarName[50];
        sprintf( max_b_trackbarName, "max B");
        createTrackbar( max_b_trackbarName, "Brightness", &max_brightness_slider, max_brightness_slider_max, on_max_b_trackbar, this );
        on_max_b_trackbar(max_brightness_slider, this);
    } //is_gui

    //optical flow
    termcrit.type = TermCriteria::COUNT|TermCriteria::EPS;
    termcrit.maxCount = 20;
    termcrit.epsilon = 0.03;
    subPixWinSize.width = 10;
    subPixWinSize.height = 10;
    winSize.width = 31;
    winSize.height = 31;
    needToInit = true;
    nightMode = false;
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

#if 0
    bool run = true;
    while(run) {
        std::unique_lock<std::mutex> lk(mu);
	c_var.wait(lk, []{return ready;});

	   
		    bool bSuccess = true;
		    //std::cout<<"grabbing a frame\n";
		    bSuccess = cap->grab(); // grab a new frame from video

		    if (!bSuccess) //if not success, break loop
		    {
		        cout << "Cannot read a frame from video stream" << endl;
		    }
	   
	    
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
#endif
       
}

void SkyWaterDetector::on_line() {
    
    cout << "LIVE ACQUISITION" << endl;
    VideoWriter _outputVideo;

#if 0
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
#endif
}


void SkyWaterDetector::off_line() {

    cout << "OFF LINE ACQUISITION" << endl;

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

    if(!calib_file.empty()) {
        cout << "input images will be UNDISTORTED" << endl;
        readCalibData(calib_file);
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

        if(!calib_file.empty()) {
            Mat temp;
            undistort(frame, temp, cameraMatrix, distCoeffs);
            frame = temp.clone();
        }

        in_frame_n++;

        //Mat mask = colorAnalysis(frame);
            
        opticalFlow(frame);

  

        if(is_gui) {
            //get the frame number and write it on the current frame
            stringstream ss;
            rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
            ss << cap->get(1); //CV_CAP_PROP_POS_FRAMES
            string frameNumberString = ss.str();
            putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        
            imshow("video", frame); 
        }
        else {
            cout << ".";
            cout.flush();
        }
        
        
        if (out_set) {
            _outputVideo.write(frame);

            out_frame_n++; 
        
            cout << "*";
            cout.flush();

            if(MAX_LENGTH > 0 && out_frame_n > MAX_LENGTH) {
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

/**
 * @function on_min_s_trackbar
 * @brief Callback for trackbar on min saturation
 */
void SkyWaterDetector::on_min_s_trackbar(int value, void* userdata)
{
    SkyWaterDetector* swd = reinterpret_cast<SkyWaterDetector*>(userdata);
    swd->on_min_s_trackbar(value);
}

void SkyWaterDetector::on_min_s_trackbar(int value) {
    min_saturation = min_saturation_slider;
} 

/**
 * @function on_max_s_trackbar
 * @brief Callback for trackbar on max saturation
 */
void SkyWaterDetector::on_max_s_trackbar(int value, void* userdata)
{
    SkyWaterDetector* swd = reinterpret_cast<SkyWaterDetector*>(userdata);
    swd->on_max_s_trackbar(value);
}

void SkyWaterDetector::on_max_s_trackbar(int value) {
    max_saturation = max_saturation_slider;
} 

/**
 * @function on_min_b_trackbar
 * @brief Callback for trackbar on min brightness
 */
void SkyWaterDetector::on_min_b_trackbar(int value, void* userdata)
{
    SkyWaterDetector* swd = reinterpret_cast<SkyWaterDetector*>(userdata);
    swd->on_min_b_trackbar(value);
}

void SkyWaterDetector::on_min_b_trackbar(int value) {
    min_brightness = min_brightness_slider;
}

/**
 * @function on_max_b_trackbar
 * @brief Callback for trackbar on max brightness
 */
void SkyWaterDetector::on_max_b_trackbar(int value, void* userdata)
{
    SkyWaterDetector* swd = reinterpret_cast<SkyWaterDetector*>(userdata);
    swd->on_max_b_trackbar(value);
}

void SkyWaterDetector::on_max_b_trackbar(int value) {
    max_brightness = max_brightness_slider;
} 

Mat SkyWaterDetector::computeMask(const Mat& frame, const Mat& I, const Mat& S) {

    Mat frame_gray;
    cvtColor(frame, frame_gray, CV_BGR2GRAY);
    Mat mask_otsu;
    cv::threshold(frame_gray, mask_otsu, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

    imshow("otsu", mask_otsu);

    Mat mask(frame.rows, frame.cols, CV_8UC3);
    Mat mask_gray(frame.rows, frame.cols, CV_8UC1);
    for(int i = 0; i < frame.rows; ++i) {
        for(int j = 0; j < frame.cols; ++j) {
            if(I.at<uchar>(i,j) > 0 &&
               S.at<uchar>(i,j) > 0 &&
               mask_otsu.at<uchar>(i,j) == 0)
            {
                mask.at<Vec3b>(i,j) = frame.at<Vec3b>(i,j);
                mask_gray.at<uchar>(i,j) = 255;
            }
            else {
                mask.at<Vec3b>(i,j) = Vec3b(0,0,0);
                mask_gray.at<uchar>(i,j) = 0;
            }
        }
    }

    


    //find biggest area
    std::vector < std::vector<Point> > contours;

    Mat tmpBinaryImage = mask_gray.clone();
    findContours(tmpBinaryImage, contours, RETR_LIST, CHAIN_APPROX_NONE);
    double max_area = 0;
    size_t max_contourIdx = 0;
    for (size_t contourIdx = 0; contourIdx < contours.size(); ++contourIdx)
    {
        Moments moms = moments(Mat(contours[contourIdx]));
        double area = moms.m00;
        if (area > max_area)
        {
            max_area = area;
            max_contourIdx = contourIdx;
        }
    }
    Mat mask_black = Mat::zeros(frame.rows, frame.cols, CV_8UC1);
    drawContours( mask_black, contours, max_contourIdx, Scalar(120, 120, 120), CV_FILLED );

    return mask_black;
}

std::string SkyWaterDetector::get_current_time_and_date()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "Y%Y-M%m-D%d-H%H-M%M-S%S");
    return ss.str();
}

void SkyWaterDetector::readCalibData(string calib_file)
{
    FileStorage fs(calib_file, FileStorage::READ);
    fs["Camera_Matrix"] >> cameraMatrix;
    fs["Distortion_Coefficients"] >> distCoeffs;
    cout << "camera matrix: " << cameraMatrix << endl
         << "distortion coeffs: " << distCoeffs << endl;
    fs.release();
}

Mat SkyWaterDetector::colorAnalysis(Mat &frame)
{   
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
    
    if(is_gui){
        imshow("Brightness", cm_I); 
        imshow("Saturation", cm_S);
        
        imshow("mask", mask);
    }
    return mask;
}

void SkyWaterDetector::opticalFlow(Mat& frame) {
        frame.copyTo(image);
        cvtColor(image, gray, COLOR_BGR2GRAY);  
        if( needToInit )
        {
            // automatic initialization
            goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
            cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit);
        }
        else if( !points[0].empty() )
        {
            vector<uchar> status;
            vector<float> err;
            if(prevGray.empty())
                gray.copyTo(prevGray);
            calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize,
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

        
        if(in_frame_n % 100 == 0) {
            needToInit = true;
        }
        else {
            needToInit = false;
        }
        imshow("LK Demo", image);

        image.copyTo(frame);
        
    //opticalflow
        std::swap(points[1], points[0]);
        cv::swap(prevGray, gray);
        
}
