/*!
 * \file
 * \brief
 */

#include <memory>
#include <string>

#include "SquareDetection.hpp"
#include "Common/Logger.hpp"

namespace Processors {
namespace SquareDetection {

using namespace cv;

SquareDetection::SquareDetection(const std::string & name) : Base::Component(name)
{
	LOG(LTRACE) << "Hello SquareDetection\n";
}

SquareDetection::~SquareDetection()
{
	LOG(LTRACE) << "Good bye SquareDetection\n";
}

bool SquareDetection::onInit()
{
	LOG(LTRACE) << "SquareDetection::initialize\n";

	// Register data streams, events and event handlers HERE!

	    // Register handler.
	    h_onNewImage.setup(this, &SquareDetection::onNewImage);
	    registerHandler("onNewImage", &h_onNewImage);
	    // Register event.
	    newImage = registerEvent("newImage");
	    newContours = registerEvent("newContours");
	    // Register data streams.
	    registerStream("in_img", &in_img);
	    registerStream("out_img", &out_img);
	    registerStream("out_contours", &out_contours);
	    registerStream("out_centers", &out_centers);

	return true;
}

bool SquareDetection::onFinish()
{
	LOG(LTRACE) << "SquareDetection::finish\n";

	return true;
}

bool SquareDetection::onStep()
{
	LOG(LTRACE) << "SquareDetection::step\n";
	return true;
}

bool SquareDetection::onStop()
{
	return true;
}

bool SquareDetection::onStart()
{
	return true;
}

void SquareDetection::onNewImage()
{

	Mat img = in_img.read();
	Mat cpy = img.clone();
	Mat drawing = Mat::zeros(img.size(), CV_8UC3);
	Mat temp = Mat::zeros(img.size(), CV_8UC1);

	// ContoursDetection {
	vector<vector<Point> > contours;
	vector<vector<Point> > selected;
	vector<vector<Point> > out;
	vector<Vec4i> hierarchy;
	vector<Point> approx;
	vector<Point> square;

	// Find Contours
	cv::findContours(cpy, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	// Draw square

	for(int i=0; i<=50; i++)
	{	
		square.push_back(Point(0,i));
	}
	for(int i=1; i<=50; i++)
	{	
		square.push_back(Point(i,50));
	}
	for(int i=49; i>=0; i--)
	{	
		square.push_back(Point(50,i));
	}
	for(int i=49; i>=1; i--)
	{	
		square.push_back(Point(i,0));
	}	


	double match;

	float radius;
	Point2f center;
	Moments mm;
	bool noticed = false;

	vector< vector<Point> > faces;

	//zwracane:
	vector<Point> detected; // = new vector<Point>();
	vector< vector<Point> > out_aproxLines; // = new vector< vector<Point> >();
	detected.clear();
	out_aproxLines.clear();

	//vector<Point> out_centers;

	//convexityDefects!

	// Select contours
	for(int i = 0; i< contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.08, true);
		mm = moments(Mat(approx), true);
		double M1 = (mm.mu20 + mm.mu02)/mm.m00*mm.m00;
		double M2 = ((mm.mu20 - mm.mu02)*(mm.mu20 - mm.mu02)+(4*mm.mu11*mm.mu11))/mm.m00*mm.m00*mm.m00*mm.m00;
		double M3 = ((mm.mu30 - 3*mm.mu12)*(mm.mu30 - 3*mm.mu12)+(3*mm.mu21 - mm.mu03)*(3*mm.mu21 - mm.mu03))/mm.m00*mm.m00*mm.m00*mm.m00*mm.m00;
		noticed = false;
		for (int k = 0; k < detected.size(); k++)
		{
			//Detect doubles
			if (mm.m10/mm.m00 < detected[k].x + 20 && mm.m10/mm.m00 > detected[k].x - 20 &&
			mm.m01/mm.m00 < detected[k].y + 20 && mm.m01/mm.m00 > detected[k].y - 20)
			{
				noticed = true;
			}
		}

		vector<Point> choosen;

		match = matchShapes(Mat(approx), Mat(square), CV_CONTOURS_MATCH_I1, 0);
	  	if (//fabs(contourArea(Mat(approx))) > 1000 
			//&& fabs(contourArea(Mat(approx))) < 10000 
			//&& 
			//match<0.8
			noticed == false && 
			M1 >= 0 && 
			//M2 <= 2.5 &&
			//M3 <= 0.5 &&
			 contourArea(Mat(approx)) > 4*arcLength(Mat(contours[i]), true) && 
			isContourConvex(approx) == 1
		   )
		{
			detected.push_back(Point(mm.m10/mm.m00, mm.m01/mm.m00));			
			drawContours(drawing, contours, i, cv::Scalar(255.0,0.0,0.0), 0, 8, hierarchy, 0, Point());
			//minEnclosingCircle(Mat(approx), center, radius);
			//line(drawing, center, center, cv::Scalar(0,255,0), 1, 8, 0);
			//rectangle(drawing, boundingRect(approx), cv::Scalar(0,255,0), 1, 8, 0);
			
			drawContours(temp, contours, i, cv::Scalar(255.0,255.0,255.0), 0, 8, hierarchy, 0, Point());
			vector<Vec2f> lines;
			HoughLines(temp, lines, 1, CV_PI/180, 20, 0, 0 );

			vector<Point2f> intersections;
			vector<Point2f> face;
			    for( size_t il = 0; il < lines.size(); il++ )
			    {
				for(size_t jl = 0; jl < lines.size(); jl++)
				{
				    Vec2f line1 = lines[il];
				    Vec2f line2 = lines[jl];

					    if(acceptLinePair(line1, line2, CV_PI / 32))
					    {
						Point2f intersection = computeIntersect(line1,line2);			
						intersections.push_back(intersection);
					    }
				}

			    }

			    if(intersections.size() > 0)
			    {
				vector<Point>::iterator point = contours[i].begin();
					for(; point != contours[i].end(); ++point)
				{				

				vector<Point2f>::iterator it;
				for(it = intersections.begin(); it != intersections.end(); ++it)
				{
					
						if (point->x < it->x + 2 && 
						point->x > it->x - 2 && 
						point->y < it->y + 2 &&
						point->y > it->y - 2) {
							face.push_back(*it);
				    			//circle(drawing, *it, 1, Scalar(0, 255, 0), 3);
						}

					}		    
					
				}
			    }

			//minEnclosingCircle(Mat(approx), center, radius);
			//circle(drawing, center, radius, cv::Scalar(0,255,255), 1, 8, 0);
			
			out.push_back(contours[i]);

			vector<Point2f>::iterator it2;
			
			bool exist = false;

			it2 = face.begin();
			if(!face.empty()){			

				for(it2 = face.begin(); it2 != face.end(); ++it2)
				{
					exist = false;					
					if(choosen.empty()){
						choosen.push_back(Point(it2->x,it2->y));					
					} else {
						vector<Point>::iterator point = choosen.begin();
						for(; point != choosen.end(); ++point){
							if (point->x < it2->x + 15 && 
							point->x > it2->x - 15 && 
							point->y < it2->y + 15 &&
							point->y > it2->y - 15) {
								exist = true;
							} 

							
						}
					}
					if(!exist){
						choosen.push_back(Point(it2->x,it2->y));
						circle(drawing, *it2, 1, Scalar(0, 255, 0), 3);					
					}

				}
			}
		}
		else
		{
			drawContours(drawing, contours, i, cv::Scalar(128.0,128.0,128.0), 0, 8, hierarchy, 0, Point());	
		}
	
		if (!choosen.empty())
		{
			vector<Point>::iterator ptr = choosen.begin();
			vector<Point>::iterator ptr2 = choosen.begin();	
			ptr2++;

			for(; ptr2 != choosen.end();)
			{
				//if (ptr2 == choosen.end()) ptr2 == choosen.begin();			
				line(drawing, *ptr, *ptr2, cv::Scalar(0,255,0), 1, 8, 0);
				ptr++;
				ptr2++;
			
			}

			ptr = choosen.begin();
			ptr2 = choosen.end();
			ptr2--;	
			line(drawing, *ptr, *ptr2, cv::Scalar(0,255,0), 1, 8, 0);
		}
		
		out_aproxLines.push_back(choosen);
	}
	



	// Write image to output data stream.
	//out_centers(detedted);
	out_contours.write(out_aproxLines);
	out_centers.write(detected);
	out_img.write(drawing);

	// Raise event.
	newContours->raise();
	newImage->raise();


}

bool SquareDetection::acceptLinePair(Vec2f line1, Vec2f line2, float minTheta)
{
    float theta1 = line1[1], theta2 = line2[1];

    if(theta1 < minTheta)
    {
        theta1 += CV_PI; // dealing with 0 and 180 ambiguities...
    }

    if(theta2 < minTheta)
    {
        theta2 += CV_PI; // dealing with 0 and 180 ambiguities...
    }

    return abs(theta1 - theta2) > minTheta;
}

Point2f SquareDetection::computeIntersect(Vec2f line1, Vec2f line2)
{
    vector<Point2f> p1 = lineToPointPair(line1);
    vector<Point2f> p2 = lineToPointPair(line2);

    float denom = (p1[0].x - p1[1].x)*(p2[0].y - p2[1].y) - (p1[0].y - p1[1].y)*(p2[0].x - p2[1].x);
    Point2f intersect(((p1[0].x*p1[1].y - p1[0].y*p1[1].x)*(p2[0].x - p2[1].x) -
                       (p1[0].x - p1[1].x)*(p2[0].x*p2[1].y - p2[0].y*p2[1].x)) / denom,
                      ((p1[0].x*p1[1].y - p1[0].y*p1[1].x)*(p2[0].y - p2[1].y) -
                       (p1[0].y - p1[1].y)*(p2[0].x*p2[1].y - p2[0].y*p2[1].x)) / denom);

    return intersect;
}

vector<Point2f> SquareDetection::lineToPointPair(Vec2f line)
{
    vector<Point2f> points;

    float r = line[0], t = line[1];
    double cos_t = cos(t), sin_t = sin(t);
    double x0 = r*cos_t, y0 = r*sin_t;
    double alpha = 1000;

    points.push_back(Point2f(x0 + alpha*(-sin_t), y0 + alpha*cos_t));
    points.push_back(Point2f(x0 - alpha*(-sin_t), y0 - alpha*cos_t));

    return points;
}

}//: namespace SquareDetection
}//: namespace Processors
