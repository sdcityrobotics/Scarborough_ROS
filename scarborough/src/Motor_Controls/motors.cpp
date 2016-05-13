/*
 * motors.cpp
 *
 *  Created on: Mar 9, 2016
 *      Author: sdcr
 */

#include "motors.h"

I2Cdev i2cdev;
Motors motors;


/*
 * getdata is for ROS to collect variables from publishers
 */
void getdata(const scarborough::Motor_Speed& msg){

	//convert from boost int to c++ int
	for(int i = 0; i < 6 ; i++){

		motors.motor_val[i] = msg.motor[i]; //boost::lexical_cast<int>(msg.motor[i]);

	}
	//send values to class variable for further analysis

}

int main(int argc, char **argv){

	//initialize ROS
	ros::init(argc, argv, "MOTORS");

	//Make node handler
	ros::NodeHandle motor_handle;
	ros::Subscriber motor_subscriber;
	motors.init();

	//main loop
	while(ros::ok()){


		motor_subscriber = motor_handle.subscribe("ARD_I2C", 200, getdata);
		for(int i = 0; i < 6 ; i++){
			cout << i << " : "<< motors.motor_val[i] << endl;
		}
		motors.set_motor_speed();

		ros::spinOnce();
	}
}

/*
 * init should be used for braking here until the killswitch is pushed
 */
void Motors::init(){

	//Motors need to be set to zero before use
//	for(int i= 0; i<6;i++){
//		i2cdev.writeWord(i2cConvert(i+33), 0, (uint16_t)motor_val[i] );
//	}
	i2cdev.writeWord(i2cConvert(38), 0, (uint16_t) 0);


}

/*
 * Set motor speed will write the motor values to their individual motors over i2c
 *
 * motor addresses will be:
 * bus: 1
 * M1: 33
 * M2: 34
 * M3: 35
 * M4: 36
 * M5: 37
 * M6: 38
 */
void Motors::set_motor_speed(){
//	for(int i = 0 ; i < 6 ; i++){
//		//write number to the motor over i2c
//		i2cdev.writeWord(i2cConvert(i+33), 0, (uint16_t)motor_val[i] );
//
//	}
	if(motor_val[0] > 5000){
		motor_val[0] = 5000;
	}
	i2cdev.writeWord(0x38 , 0x00 ,(uint16_t)motor_val[0]);
}

/*
 * Get motor values will grab the motor values from the ROS system and set them to some class variables
 */
void Motors::get_motor_values(int motor_raw[6]){
	for(int i = 0 ; i < 6 ; i++){

		//TODO Matt Pedler: do an actual conversion of what needs to be written. For right now this will suffice
		motor_val[i] = (int)motor_raw[i];
	}
}

int Motors::i2cConvert(int address){

	//load int into string stream
	stringstream ss;
	ss << address;
	string ss_return;
	//load ss into string
	ss >> ss_return;

	int length;

	//get length of the string
	length = ss_return.length();
	int temp = 0;
	int _length = length;

	//this will convert the number from hex to decimal
	//this only works for hex numbers that do not contain a letter in them.

	for(int i = 0; i < length ;i++){

		//add a modifier based upon where it is at in the string
		int moder = (16 * (_length -1));
		//if the modifier is zero make it 1
		moder = max(moder, 1);

		//convert the string from the hex number to the decimal int
		temp += atoi(ss_return.substr(i,1).c_str()) * moder;
		_length--;

		//cout << temp << " / " << atoi(ss_return.substr(i,1).c_str())  << " / " << moder<< endl;

	}
	//return the decimal int number we can use to access the device on the i2c
	return temp;

}


