/*
 * stationarypoint.hpp
 *
 * Author: Domenico Daniele Bloisi
 * email: domenico.bloisi@gmail.com
 */

using namespace cv;
using namespace std;

class StationaryPoint
{
private:
    Point2f point;
    int history;
public:
    StationaryPoint(Point2f point, int history);
};

