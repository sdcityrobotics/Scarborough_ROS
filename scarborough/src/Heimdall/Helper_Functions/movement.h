/*
 * movement.h
 *
 *  Created on: Jul 24, 2016
 *      Author: ximidar
 */

#ifndef SRC_HEIMDALL_HELPER_FUNCTIONS_MOVEMENT_H_
#define SRC_HEIMDALL_HELPER_FUNCTIONS_MOVEMENT_H_

//
//  movement.hpp
//  openLOL
//
//  Created by Shubhankar Agarwal on 7/17/16.
//  Copyright © 2016 Shubhankar Agarwal. All rights reserved.
//
#include <stdio.h>

#include "opencv2/imgproc/imgproc.hpp"
#ifndef movement_hpp
#define movement_hpp


using namespace std;
using namespace cv;

/// Function header
void direction(vector<Point>  target , double yawO , vector<Point2i> cent_i  , int framecounter , vector<double> area1 , double curr_depth);

void changeaxis(Rect target , double yawO , vector<Point2i> cent_i  , int framecounter , vector<double> area1);


#endif /* movement_hpp */






#endif /* SRC_HEIMDALL_HELPER_FUNCTIONS_MOVEMENT_H_ */
