//
//  main.cpp
//  OMR-BubbleSheet
//
//  Created by Cheling Lim on 2/2/18.
//  Copyright © 2018 Cheling Lim. All rights reserved.
//

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cstdlib>
#include <map>

#include "transform.hpp"

using namespace cv;

int main(int argc, char** argv)
{
    const int NoOfChoice = 5;
    const int NoOfQuestion = 5;
    std::map<int, int> standardAnswer;
    std::map<int, int> testerAnswer;
    standardAnswer.insert(std::make_pair(0, 1)); //Question 1: answer: B
    standardAnswer.insert(std::make_pair(1, 3)); //Question 2: answer: D
    standardAnswer.insert(std::make_pair(2, 0)); //Question 3: answer: A
    standardAnswer.insert(std::make_pair(3, 2)); //Question 4: answer: C
    standardAnswer.insert(std::make_pair(4, 1)); //Question 5: answer: B
    
    Mat image, gray, blurred, edge;
    
    //Step 1: Detect the exam in an image
    image = imread("omr_test_01.png", IMREAD_COLOR);
    
    if(image.empty())
    {
        std::cout << "Failed to read file\n";
        exit(1);
    }
    cvtColor(image, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, blurred,Size(5, 5), 0);
    Canny(blurred, edge, 75, 200);
    
    /*imshow("image", image);
    imshow("gray", gray);
    imshow("blurred", blurred);
    imshow("Canny edge", edge);*/

    Mat paper, warped, thresh;
    std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;
    std::vector<Point> approxCurve, docCnt;
    
    /// Find contours
    findContours(edge, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    
    std::sort(contours.begin(), contours.end(), compareContourAreas);
    
   //Find document countour
    for (const auto &element: contours)
    {
        double perimeter = arcLength(element, true);
        approxPolyDP(element, approxCurve, 0.05 * perimeter, true);
        
        if (approxCurve.size() == 4)
        {
            docCnt = approxCurve;
            break;
        }
    }

    
    //Step 2: Apply a perspective transform to extract the top-down, birds-eye-view of the exam
    
    four_point_transform(image, paper, docCnt);
    //imshow("paper", paper);
    four_point_transform(gray, warped, docCnt);
    //imshow("warped", warped);
    
    //Step 3: Extract the sets of bubbles (questionCnt)
    threshold(warped, thresh, 127, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
    //imshow("thresh", thresh);
    
    findContours(thresh, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    
    std::vector<std::vector<Point> > contours_poly(contours.size());
    std::vector<Rect> boundRect(contours.size());
    std::vector<std::vector<Point>> questionCnt;
    
    
    //Find document countour
    for( int i = 0; i< contours.size(); i++ )
    {
        approxPolyDP(Mat(contours[i]), contours_poly[i], 0.1, true);
        int w =  boundingRect(Mat(contours[i])).width;
        int h =  boundingRect(Mat(contours[i])).height;
        double ar = (double)w/h;
        
        if(hierarchy[i][3] == -1) //No parent
        if(w >=20 && h >=20 && ar < 1.1 && ar > 0.9)
            questionCnt.push_back(contours_poly[i]);
        
        
    }
    
    //std::cout << "\nquestionCnt " << questionCnt.size() << "\n";
    
    //Step 4: Sort the questions/bubbles into rows.
    sort_contour(questionCnt, 0, (int)questionCnt.size(), std::string("top-to-bottom"));
    
    for( int i = 0; i< questionCnt.size(); i = i + NoOfChoice)
    {
        sort_contour(questionCnt, i, i + 5, std::string("left-to-right"));
    }

    //Step 5: Determine the marked answer
    for( int i = 0; i< questionCnt.size();)
    {
        int maxPixel = 0;
        int answerKey = 0;
        for(int j = 0 ; j < NoOfChoice; ++i, ++j)
        {
            Mat mask = Mat::zeros(thresh.size(), CV_8U);
            drawContours(mask, questionCnt, i, 255, CV_FILLED, 8, hierarchy, 0, Point());
            bitwise_and(mask, thresh, mask);
            if (countNonZero(mask) > maxPixel)
            {
                maxPixel = countNonZero(mask);
                answerKey = j;
            }
        }
        testerAnswer.insert(std::make_pair(i/NoOfChoice - 1, answerKey));
        std::cout << "Question: " << i/NoOfChoice << " Tester's Answer: " << answerKey << "\n";
    }
    
    //Step 6: Lookup the correct answer in our answer key to determine if the user was correct in their choice.
    std::map<int, int>::const_iterator itStandardAnswer;
    std::map<int, int>::const_iterator itTesterAnswer;
    
    itStandardAnswer = standardAnswer.begin();
    itTesterAnswer = testerAnswer.begin();
    
    int correct = 0;
    int currentQuestion = 0;
    
    while(itStandardAnswer != standardAnswer.end())
    {
        if(itStandardAnswer->second == itTesterAnswer->second)
        {
            ++correct;
            //Circle in GREEN
            drawContours(paper, questionCnt,  (currentQuestion * NoOfChoice) + itStandardAnswer->second, Scalar(0, 255, 0), 2 , 8, hierarchy, 0, Point());
        }
        else //Circle in RED
        {
            drawContours(paper, questionCnt,  (currentQuestion * NoOfChoice) + itStandardAnswer->second, Scalar(0, 0, 255), 2 , 8, hierarchy, 0, Point());
        }

        ++currentQuestion;
        ++itTesterAnswer;
        ++itStandardAnswer;
    }
    
    //Step 7: Display the score
    double score = ((double)correct/NoOfQuestion * 100);

    Scalar color;
    if (score >= 60.0)
        color = Scalar(0, 255, 0);
    else
        color = Scalar(0, 0, 255);
    
    putText(paper, std::to_string((int)score) + "%" , Point(20, 30), FONT_HERSHEY_SIMPLEX, 0.9, color, 2);
    std::cout << "Total " << currentQuestion << "  correct " << correct << "\n";
    imshow("Marked Paper", paper);
    waitKey();
    return 0;
}

