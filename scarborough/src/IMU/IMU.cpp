/*
 * IMU.cpp
 *
 *  Created on: Feb 12, 2016
 *      Author: ubuntu
 */
//#define NO_I2C


#include "IMU.h"

//TODO create a debug function that will override addressing the i2c bus since some computers don't have a usable one.
//declare imu object
IMU imu;
bool killer;

void getdata_KILL(const scarborough::Kill_Switch& msg){
	cout << msg << " GETDATA" << endl;
	killer = msg.killed;


}


Handler handler;
int main(int argc, char **argv) {
	cout << "initializing ROS" << endl;
	ros::init(argc, argv, handler.IMU_CHATTER);

	ros::NodeHandle n;
	ros::NodeHandle kill;
	ros::Subscriber sub;



	ros::Publisher imu_pub = n.advertise<scarborough::YPR>(handler.IMU, 200);
        sub = kill.subscribe(handler.KILL, 20, getdata_KILL);


	
    cout << "Initializing IMU" << endl;
    imu.IMU_init();
    //imu.set_zero(); // many offsets have calculated this number as YPR_OFFSET: -13.1352 -0.686341 -0.702521
    bool restart = false;
    while(ros::ok()){
    	
	
    	if(!killer){
		 

		if(restart){
			cout<< "INITIALIZING IMU" << endl;
			imu.IMU_init();
			restart = false;
		}

    		imu.read_IMU();

    		imu_pub.publish(imu.ypr_ROS.YPR);
		ros::spinOnce();
    	}
    	else{
		

		if(!restart){
    			restart = true;
		}
		//sit here while the switch is off
    		cout << "Waiting " << killer <<  endl;
		ros::spinOnce();
    	}


    	

    }




}


IMU::IMU(){
	dmpReady = false;

	ypr_offset[0] = -13.1352;
	ypr_offset[1] = -0.686341;
	ypr_offset[2] = -0.702521;

}

void IMU::IMU_init(){
	 // initialize device
	    printf("Initializing I2C devices...\n");
	    mpu.initialize();

	    // verify connection
	    printf("Testing device connections...\n");
	    //wait until we get a connection.
	   	while(mpu.testConnection() == false ){
	   		cout << ".";
	   	}
	    printf(mpu.testConnection() ? "MPU6050 connection successful\n" : "MPU6050 connection failed\n");


	    // load and configure the DMP
	    printf("Initializing DMP...\n");
	    devStatus = mpu.dmpInitialize();

	    // make sure it worked (returns 0 if so)
	    if (devStatus == 0) {
	        // turn on the DMP, now that it's ready
	        printf("Enabling DMP...\n");
	        mpu.setDMPEnabled(true);

	        // enable Arduino interrupt detection
	        //Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
	        //attachInterrupt(0, dmpDataReady, RISING);
	        mpuIntStatus = mpu.getIntStatus();

	        // set our DMP Ready flag so the main loop() function knows it's okay to use it
	        printf("DMP ready!\n");
	        dmpReady = true;

	        // get expected DMP packet size for later comparison
	        packetSize = mpu.dmpGetFIFOPacketSize();
	    } else {
	        // ERROR!
	        // 1 = initial memory load failed
	        // 2 = DMP configuration updates failed
	        // (if it's going to break, usually the code will be 1)
	        printf("DMP Initialization failed (code %d)\n", devStatus);
	    }
}

//TODO read_IMU needs to have an array passed into it to have the array populated with the YPR angles.
//TODO read_IMU needs to be split into different functions that return the different types of data instead of having things be defined. It just looks nicer.
void IMU::read_IMU(){
	// if programming failed, don't try to do anything
	    if (!dmpReady) return;
	    // get current FIFO count
	    fifoCount = mpu.getFIFOCount();

	    if (fifoCount == 1024) {
	        // reset so we can continue cleanly
	        mpu.resetFIFO();
	       // printf("FIFO overflow!\n");

	    // otherwise, check for DMP data ready interrupt (this should happen frequently)
	    } else if (fifoCount >= 42) {
	        // read a packet from FIFO
	        mpu.getFIFOBytes(fifoBuffer, packetSize);

	        #ifdef OUTPUT_READABLE_QUATERNION
	            // display quaternion values in easy matrix form: w x y z
	            mpu.dmpGetQuaternion(&q, fifoBuffer);
	            printf("quat %7.2f %7.2f %7.2f %7.2f    ", q.w,q.x,q.y,q.z);
	        #endif

	        #ifdef OUTPUT_READABLE_EULER
	            // display Euler angles in degrees
	            mpu.dmpGetQuaternion(&q, fifoBuffer);
	            mpu.dmpGetEuler(euler, &q);
	            printf("euler %7.2f %7.2f %7.2f    ", euler[0] * 180/M_PI, euler[1] * 180/M_PI, euler[2] * 180/M_PI);
	        #endif

	        #ifdef OUTPUT_READABLE_YAWPITCHROLL
	            // display Euler angles in degrees
	            mpu.dmpGetQuaternion(&q, fifoBuffer);
	            mpu.dmpGetGravity(&gravity, &q);
	            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
	            //printf("ypr  %7.2f %7.2f %7.2f    ", ypr[0] * 180/M_PI, ypr[1] * 180/M_PI, ypr[2] * 180/M_PI);
	            imu.ypr_ROS.YPR[0] =  (ypr[0] * 180/M_PI) - ypr_offset[0] * -1;
	            imu.ypr_ROS.YPR[2] =  (ypr[1] * 180/M_PI) - ypr_offset[1] * -1;
	            imu.ypr_ROS.YPR[1] =  (ypr[2] * 180/M_PI) - ypr_offset[2] * -1;

	            //output current rotation
	            cout << "YPR: " << imu.ypr_ROS.YPR[0]
	                            << " "
	                            << imu.ypr_ROS.YPR[1]
							    << " "
							    << imu.ypr_ROS.YPR[2];
	        #endif

	        #ifdef OUTPUT_READABLE_REALACCEL
	            // display real acceleration, adjusted to remove gravity
	            mpu.dmpGetQuaternion(&q, fifoBuffer);
	            mpu.dmpGetAccel(&aa, fifoBuffer);
	            mpu.dmpGetGravity(&gravity, &q);
	            mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
	            printf("areal %6d %6d %6d    ", aaReal.x, aaReal.y, aaReal.z);
	        #endif

	        #ifdef OUTPUT_READABLE_WORLDACCEL
	            // display initial world-frame acceleration, adjusted to remove gravity
	            // and rotated based on known orientation from quaternion
	            mpu.dmpGetQuaternion(&q, fifoBuffer);
	            mpu.dmpGetAccel(&aa, fifoBuffer);
	            mpu.dmpGetGravity(&gravity, &q);
	            mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
	            printf("aworld %6d %6d %6d    ", aaWorld.x, aaWorld.y, aaWorld.z);
	        #endif


	        printf("\n");
	    }
}
/*
 * This function waits for the IMU to settle then sets the zero point for the entire class.
 *
 */

void IMU::set_zero(){
	cout << "Setting Zero\n";
	for(int i = 0; i < 50000; i++){
		// if programming failed, don't try to do anything
		if (!dmpReady) return;
		// get current FIFO count
		fifoCount = mpu.getFIFOCount();

		if (fifoCount == 1024) {
			// reset so we can continue cleanly
			mpu.resetFIFO();
			printf("FIFO overflow!\n");

			// otherwise, check for DMP data ready interrupt (this should happen frequently)
		} else if (fifoCount >= 42) {
			// read a packet from FIFO
			mpu.getFIFOBytes(fifoBuffer, packetSize);

			// display Euler angles in degrees
			mpu.dmpGetQuaternion(&q, fifoBuffer);
			mpu.dmpGetGravity(&gravity, &q);
			mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
			//printf("ypr  %7.2f %7.2f %7.2f    ", ypr[0] * 180/M_PI, ypr[1] * 180/M_PI, ypr[2] * 180/M_PI);
			ypr_offset[0] =  ypr[0] * 180/M_PI;
			ypr_offset[1] =  ypr[1] * 180/M_PI;
			ypr_offset[2] =  ypr[2] * 180/M_PI;
			cout <<"\r" << i ;


		}
	}
	cout << "\nYPR_OFFSET: " <<  ypr_offset[0]
						   << " "
				           <<  ypr_offset[1]
						   << " "
						   <<  ypr_offset[2]
						   <<"\n";


}


