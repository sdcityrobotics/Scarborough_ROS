/*
 * hal.h
 *
 *  Created on: Feb 25, 2016
 *      Author: sdcr
 */

/*
 * Hal9000 was the futuristic machine off of the movie "2001:A Space Oddysey"
 * This Hal is the State machine that will take in all information from the other
 * processes and then decide what the entire machine should do. All debug messages
 * from this program should be "I Can't do that Dave" Hopefully a Dave will work on
 * this code in the future and get really freaked out by it.
 */
#ifndef HAL_H_
#define HAL_H_

// TODO include data types here.


class hal{
	public:
		hal();
		void collect(); //collect all information. This means all imu data, vision data, and depth data.
		void process(); //process what the information means, for example if we find an orange square and an obstacle we want to go to the obstacle
		void decide(); // Decide will actually change the state of the machine.
};



#endif /* HAL_H_ */
