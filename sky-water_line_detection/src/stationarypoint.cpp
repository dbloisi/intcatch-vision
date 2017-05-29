/*
 * stationarypoint.cpp
 *
 * Author: Domenico Daniele Bloisi
 * email: domenico.bloisi@gmail.com
 */

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/features2d/features2d.hpp"

#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"

#include "stationarypoint.hpp"

StationaryPoint::StationaryPoint(Point2f point, int history)
{
    this->point = point;
    this->history = history;
    this->hit = true;
}

Point2f StationaryPoint::getPoint() {
    return point;
}

void StationaryPoint::setPoint(Point2f p) {
    point = p;
}

int StationaryPoint::getHistory() {
    return history;
}

void StationaryPoint::setHistory(int h) {
    history = h;
}

bool StationaryPoint::getHit() {
    return hit;
}

void StationaryPoint::setHit(bool value) {
    hit = value;
}
