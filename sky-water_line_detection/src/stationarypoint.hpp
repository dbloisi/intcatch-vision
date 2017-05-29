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
    bool hit;
public:
    StationaryPoint(Point2f point, int history);
    Point2f getPoint();
    void setPoint(Point2f p);
    int getHistory();
    void setHistory(int h);
    bool getHit();
    void setHit(bool value);
};
