//
//  transform.hpp
//  OMR-BubbleSheet
//
//  Created by Cheling Lim on 6/2/18.
//  Copyright © 2018 Cheling Lim. All rights reserved.
//

#ifndef transform_hpp
#define transform_hpp

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <string>

using namespace cv;

// Function header
bool compareContourAreas (std::vector<Point> contour1, std::vector<Point> contour2);
void four_point_transform(Mat &image, Mat &imageOut, std::vector<Point> docCnt);
void sort_contour(std::vector<std::vector<Point>> &contours, int from, int to, std::string sortOrder);
#endif /* transform_hpp */
