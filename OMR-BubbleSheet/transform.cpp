//
//  transform.cpp
//  OMR-BubbleSheet
//
//  Created by Cheling Lim on 6/2/18.
//  Copyright © 2018 Cheling Lim. All rights reserved.
//
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <string>

#include "transform.hpp"

using namespace cv;

const int TOPLEFT = 0;
const int TOPRIGHT = 1;
const int BOTTOMRIGHT = 2;
const int BOTTOMLEFT = 3;

void order_rectangle_points(std::vector<Point> &pts){
    //order into sequence of TopLeft, TopRight, bottomRigh, bottomLeft
    int minSum = pts[0].x + pts[0].y;
    int maxSum = minSum;
    //topRight has the biggest sum of x & y, bottomLeft has the minimum
    auto topRight(pts[0]), bottomLeft(pts[0]);
    
    for(int i = 1; i < pts.size(); ++i)
    {
        if((pts[i].x + pts[i].y) > maxSum)
        {
            maxSum = pts[i].x + pts[i].y;
            topRight = pts[i];
        }
        else if((pts[i].x + pts[i].y) < minSum)
        {
            minSum = pts[i].x + pts[i].y;
            bottomLeft = pts[i];
        }
    }
    
    //topLeft's x is closer to bottomLeft's in X value
    auto topLeft(topRight), bottomRight(bottomLeft);
    for(int i = 0; i < pts.size(); ++i)
    {
        if (((pts[i].x == topRight.x) && (pts[i].y == topRight.y)) || ((pts[i].x == bottomLeft.x) && (pts[i].y == bottomLeft.y)))
        {
            //Do nothing
        }
        else if(abs(pts[i].x - bottomLeft.x) < abs(pts[i].x - topRight.x))
        {
            topLeft = pts[i];
        }
        else
        {
            bottomRight = pts[i];
        }
    }
    
    pts[TOPLEFT] = topLeft;
    pts[TOPRIGHT] = topRight;
    pts[BOTTOMRIGHT] = bottomRight;
    pts[BOTTOMLEFT] = bottomLeft;
}

int max(const int &v1, const int &v2){
    return (v1 > v2)? v1: v2;
}

void four_point_transform(Mat &imageIn, Mat &imageOut, std::vector<Point> src){
    
    //order_rectangle_points(src);
    //Mat rect_src = Mat::Mat(4, 2, CV_32F, src.data());
    order_rectangle_points(src);
    
    Point2f src_vertices[4];
    src_vertices[0] = src[0];
    src_vertices[1] = src[1];
    src_vertices[2] = src[2];
    src_vertices[3] = src[3];
    
    //const Point *point = &src[0];
    //int n = (int)src.size();
    
    //polylines(imageIn, &point, &n, 1, true, Scalar(0, 255, 0), 3, CV_AA);
    //imshow("draw", imageIn);
    
    int maxWidth = max((src[TOPRIGHT].x - src[TOPLEFT].x), (src[BOTTOMRIGHT].x - src[BOTTOMLEFT].x));
    int maxHeight = max((src[TOPLEFT].y - src[BOTTOMLEFT].y), (src[TOPRIGHT].y - src[BOTTOMRIGHT].y));
    
    Point2f dst_vertices[4];
    dst_vertices[0] = Point(0, maxHeight-1);
    dst_vertices[1] = Point(maxWidth-1, maxHeight-1);
    dst_vertices[2] = Point(maxWidth-1, 0);
    dst_vertices[3] = Point(0, 0);
   
    Mat warpMatrix = getPerspectiveTransform(src_vertices, dst_vertices);
    //std::cout << "imageOut.size()  " << imageOut.size();
    warpPerspective(imageIn, imageOut, warpMatrix, Size(maxWidth, maxHeight), INTER_LINEAR, BORDER_CONSTANT);
}

// comparison function object
bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 ) {
    double i = fabs(contourArea(cv::Mat(contour1)));
    double j = fabs(contourArea(cv::Mat(contour2)));
    return ( i > j );
}

bool sortTopToBottom(const std::vector<Point> &lhs, const std::vector<Point> &rhs)
{
    Rect rectLhs = boundingRect(Mat(lhs));
    Rect rectRhs = boundingRect(Mat(rhs));
    
    return rectLhs.y < rectRhs.y;
}

bool sortLeftToRight(const std::vector<Point> &lhs, const std::vector<Point> &rhs)
{
    Rect rectLhs = boundingRect(Mat(lhs));
    Rect rectRhs = boundingRect(Mat(rhs));
    
    return rectLhs.x < rectRhs.x;
}

void sort_contour(std::vector<std::vector<Point>> &contours, int from, int to, std::string sortOrder)
{
    if (!sortOrder.compare("top-to-bottom"))
    {
        sort(contours.begin() + from, contours.begin() + to, sortTopToBottom);
    }
    else if (!sortOrder.compare("left-to-right"))
    {
        sort(contours.begin() + from, contours.begin() + to, sortLeftToRight);
    }
}


