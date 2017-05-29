/*
 * featuretracker.cpp
 *
 * Author: Domenico Daniele Bloisi
 * email: domenico.bloisi@gmail.com
 */

#include "featuretracker.hpp"

#define DEBUG

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
    slices = 1;
    needToInit.resize(slices);
    for(int i = 0; i < slices; ++i) {
        needToInit[i] = true;
    }

    prev_points.resize(slices);
    points.resize(slices);

    hist_prev_points.resize(slices);
    hist_points.resize(slices);

    history.resize(slices);

    init_counter = 0;
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

    prev_points.resize(slices);
    points.resize(slices);

    hist_prev_points.resize(slices);
    hist_points.resize(slices);

    history.resize(slices);

    init_counter = 0;
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

        Mat slice(gray, Rect(w * i, 0, w, gray.rows));

        Mat prev_slice(prevGray, Rect(w * i, 0, w, prevGray.rows));

        computeSlice(prev_slice, slice, i);

    }//for slices

    cv::swap(prevGray, gray);
    init_counter++;

    imshow("LK Demo", image);
}

void FeatureTracker::computeSlice(cv::Mat& prev_slice, cv::Mat& slice, int slice_idx) {

    if(needToInit[slice_idx])
    {
        cout << "NEED TO INIT slice " << slice_idx << endl;

        if( !prev_points[slice_idx].empty() )
        {
          for( int i = 0; i < prev_points[slice_idx].size(); i++ )
          {
              hist_prev_points[slice_idx].push_back(prev_points[slice_idx][i]);
          }
        }

        goodFeaturesToTrack(slice, points[slice_idx], max_count, 0.01, 10, Mat(), 3, 0, 0.04);
	      cornerSubPix(slice, points[slice_idx], subPixWinSize, Size(-1,-1), termcrit);

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
                   2, Scalar(0,255,0), -1, 8);
	      }
	      points[slice_idx].resize(k);
    }

    if( !hist_prev_points[slice_idx].empty() )
    {
        vector<uchar> status;
        vector<float> err;

        calcOpticalFlowPyrLK(prev_slice, slice, hist_prev_points[slice_idx], hist_points[slice_idx],
	                     status, err, winSize,
	                     3, termcrit, 0, 0.001);
        size_t i, k;
        for( i = k = 0; i < hist_points[slice_idx].size(); i++ )
        {
            if( !status[i] )
                continue;

	          hist_points[slice_idx][k++] = hist_points[slice_idx][i];
	          circle(image,
                   Point(hist_points[slice_idx][i].x + (slice_idx*slice.cols),
                         hist_points[slice_idx][i].y),
                   3, Scalar(0, 0, 255), -1, 8);
	      }
	      hist_points[slice_idx].resize(k);

        //if(history[slice_idx].empty()) {
        //if(history[slice_idx].size() < 20) {

        #ifdef DEBUG
                  cout << "----------------hist  points---------------------" << endl;
                  for( int i = 0; i < hist_points[slice_idx].size(); i++ )
                  {

                      Point2f p = hist_points[slice_idx][i];

                      cout << p << endl;
                  }
                  cout << "----------------------------------------" << endl;
        #endif

          for( int i = 0; i < hist_points[slice_idx].size(); i++ )
          {
              StationaryPoint sp(hist_points[slice_idx][i], 1);
              history[slice_idx].push_back(sp);
          }

        //}
        //else {

          //waitKey(0);

          for( int i = 0; i < history[slice_idx].size(); i++ )
          {
              bool to_erase = false;
              for( int n = 0; n < i; n++ )
              {
                  if(history[slice_idx][i].getPoint().x ==
                     history[slice_idx][n].getPoint().x &&
                     history[slice_idx][i].getPoint().y ==
                     history[slice_idx][n].getPoint().y )
                  {
                      to_erase = true;
                      break;
                  }
              }
              for( int k = i+1; k < history[slice_idx].size(); k++ )
              {
                  if(history[slice_idx][i].getPoint().x ==
                     history[slice_idx][k].getPoint().x &&
                     history[slice_idx][i].getPoint().y ==
                     history[slice_idx][k].getPoint().y )
                  {
                      to_erase = true;
                      break;
                  }
              }

              if(to_erase) {
                history[slice_idx][i].setHit(false);
                continue;
              }

              StationaryPoint sp = history[slice_idx][i];
              Point2f p = sp.getPoint();
              int h = sp.getHistory();
              bool found = false;
              int j = 0;
              for( ; j < hist_prev_points[slice_idx].size(); j++ )
              {
                  if(p.x == hist_prev_points[slice_idx][j].x &&
                     p.y == hist_prev_points[slice_idx][j].y) {
                        found = true;
                        break;
                  }
              }
              if(found) {
                history[slice_idx][i].setPoint(hist_points[slice_idx][j]);
                history[slice_idx][i].setHistory(++h);
                history[slice_idx][i].setHit(true);

                //circle(image,
                //     Point(history[slice_idx][i].getPoint().x + (slice_idx*slice.cols),
                //             history[slice_idx][i].getPoint().y),
                //     4, Scalar(0, 0, 0), 1, 8);
              }
          }

#ifdef DEBUG
          cout << "----------------history prima -------------------" << endl;
          for( int i = 0; i < history[slice_idx].size(); i++ )
          {
              StationaryPoint sp = history[slice_idx][i];
              Point2f p = sp.getPoint();
              int h = sp.getHistory();
              bool v = sp.getHit();

              cout << p << " history: " << h << " hit: " << v << endl;
          }
          cout << "----------------------------------------" << endl;
#endif

        vector<StationaryPoint> temp_history;

        for( int i = 0; i < history[slice_idx].size(); i++ )
        {
            if(history[slice_idx][i].getHit()) {
              StationaryPoint sp = history[slice_idx][i];
              sp.setHit(false);
              temp_history.push_back(sp);
            }
        }
        history[slice_idx].clear();
        for( int i = 0; i < temp_history.size(); i++ )
        {
            history[slice_idx].push_back(temp_history[i]);
            if(history[slice_idx][i].getHistory() > 100) {
                circle(image,
                   Point(history[slice_idx][i].getPoint().x + (slice_idx*slice.cols),
                         history[slice_idx][i].getPoint().y),
                   5, Scalar(255,255,255), 3, 8);
            }
            else if(history[slice_idx][i].getHistory() > 50) {
                circle(image,
                   Point(history[slice_idx][i].getPoint().x + (slice_idx*slice.cols),
                         history[slice_idx][i].getPoint().y),
                   5, Scalar(255,0,0), 3, 8);
            }
        }
    }

    #ifdef DEBUG
              cout << "----------------history dopo------------------" << endl;
              for( int i = 0; i < history[slice_idx].size(); i++ )
              {
                  StationaryPoint sp = history[slice_idx][i];
                  Point2f p = sp.getPoint();
                  int h = sp.getHistory();
                  bool v = sp.getHit();

                  cout << p << " history: " << h << " hit: " << v << endl;
              }
              cout << "----------------------------------------" << endl;
              waitKey(0);
    #endif


    if(init_counter > INIT_FRAMES) {
        needToInit[slice_idx] = true;
        init_counter = 0;
    }
    else if(hist_points[slice_idx].size() < 10) {
        needToInit[slice_idx] = true;
        init_counter = 0;
    }
    else {
        needToInit[slice_idx] = false;
    }



    //image.copyTo(frame);

    std::swap(points[slice_idx], prev_points[slice_idx]);

    std::swap(hist_points[slice_idx], hist_prev_points[slice_idx]);

}
