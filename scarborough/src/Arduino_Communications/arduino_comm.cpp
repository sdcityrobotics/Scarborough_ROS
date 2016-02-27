/*
 * arduino_comm.cpp
 *
 *  Created on: Jan 24, 2016
 *      Author: sdcr
 */
#include "arduino_comm.h"
#define DEBUG

//serial stream is used for opening and using comm ports
SerialStream port;
//ArdComm is used to communicate with the arduino
ArdComm ardcomm;
//ArdI2C is for communicating with the arduino over the i2c bus. TODO
//ArdI2C ardi2c;


int main(int argc, char **argv){

	ros::init(argc, argv, "arduino_chatter");

	ros::NodeHandle n;

	ros::Publisher ard_pub = n.advertise<scarborough::Motor_Speed>("arduino_chatter", 1000);

	//initialize the port for sending and receiving.
	ardcomm.init();

	//check ros to see if it is still active
	while(ros::ok()){

		/*TODO
		 * populate the data types from the arduino comm port
		 * Determine future things to pull from the arduino over the comm port
		 */
		ardcomm.interperet_message(ardcomm.motor.motor);

		ard_pub.publish(ardcomm.motor);

		ros::spinOnce();
	}

}

ArdComm::ArdComm(){
	handshake = false;
	message_count = 0;
	path = "/dev/ttyACM";
	delimiter = ";";
	parse = "";
	pos = 0;

}
//listen listens to the comm port and returns the message when the end character
//is hit. '\'
string ArdComm::listen(){
	char next_char;
	string message;
	 message_count = 0;
	cout << "Reading Message" << endl;
	//while the end line character isn't hit record the message.
	while(next_char != '\\'){

		port >> next_char; //get the next char
		message += next_char; //add char to message

		//check if a message is too long. If it is then throw away the message and break.
		/*
		 * This is nice for checking if there's waaay to many characters, but we need a better way to
		 * detect a long message. Maybe we should detect if it has changed the same variable in the same
		 * line. TODO
		 */
		 message_count++;
		if ( message_count > 200){
			cout << "ERROR MESSAGE OVER 200 CHARACTERS!!!";
			message = "";
			break;
		}
	}
	return message;
}

void ArdComm::init(){

	//find an open comm port. 0 - 10 is just because I've never seen more than 3 comm ports
	//TODO do a handshake with the arduino so we can determine if it is the correct comm port even further
	for(int i = 0 ; i < 10 ; i++){
		//bool equals true or false depending on whether a path exists. path is defined in the constructor.
		handshake = pathExists(path + boost::lexical_cast<std::string>(i) );
		//if it returns true then construct the full path and break the loop.
		if(handshake == true){
			path += boost::lexical_cast<std::string>(i);
			break;
		}
	}

	//output a statement that tells which comm port the program connected to
	cout << "connected on Comm Port: "
			<< path
			<< endl;

	//open comm port
	port.Open(path);
	//set baud rate. This also needs to be set on the arduino
	port.SetBaudRate(SerialStreamBuf::BAUD_115200);
	//set char size.
	port.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 );
}

/*
 * This function takes the message from the arduino then parses the statement.
 * a standard statement is M0:123.235;M4:544.23523;M1:665;\ this will break apart this statement
 * into an array. the individual statements are broken up by the ";" delimiter
 * the '\' is for the listen() function which uses it to find the end of a line.
 * TODO (Matt Pedler) Find other things that need to be pulled from the comm port
 */
void ArdComm::interperet_message(boost::array<double, 6ul>& motor){
	message = listen();

	while( message != ""){
		pos = message.find(delimiter);
		parse = message.substr(0, pos);
		for(unsigned int i = 0 ; i < parse.length() ; i++){

			switch(parse[i]){

			case 'M':
				cout << "Case M Found!" << endl;

				if(atoi(parse.substr(1,2).c_str()) >= 0 && atoi(parse.substr(1,2).c_str()) <=5 ){

					//populate the correct motor with the correct value
					motor[atoi(parse.substr(1,2).c_str())] = atof(message.substr(3,pos).c_str());

					//set cursor to next variable location.
					i = pos;
					//erase parsed values
					message.erase(0 , pos + delimiter.length());
				}//end if
				else{
					cout << "number is not within the bounds of the array" << endl;
				}

				break;

			default:
				cout << "default hit \n";
				break;
			}//end switch
		}//end for
	}//end while
}//end function


//This function checks whether a path exists or not. will return true or false
inline bool ArdComm::pathExists (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}





