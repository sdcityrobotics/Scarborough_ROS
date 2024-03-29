/*
 * anchor.cpp
 *
 *  Created on: May 30, 2016
 *      Author: sdcr
 */

#include "anchor.h"

#define ANCHOR_ADDR 0x76
#define ANCHOR_RESET 0x1E
#define ANCHOR_ADC_READ 0x00
#define ANCHOR_PROM_READ 0xA0
#define ANCHOR_CONVERT_D1_8192 0x4A
#define ANCHOR_CONVERT_D2_8192 0x5A

I2Cdev i2cdev;





void Anchor::init(){

	i2cdev.writeByte(ANCHOR_ADDR, ANCHOR_RESET, 0);
	usleep(1000);

	for(uint8_t i = 0 ; i < 8; i++){
		uint8_t temp;
		i2cdev.readByte(ANCHOR_ADDR, ANCHOR_PROM_READ + i * 2, &temp);

		//load C
		C[i] = (temp << 8 | temp);
	}

	uint8_t crcRead = C[0] >> 12;

	uint8_t crcCalculated = crc4(C);

	//this is dumb. Blue robotics programming guy is dumb
	if(crcCalculated == crcRead){
		//success
	}
	else{
		//failure
	}
}



void Anchor::read(){

	i2cdev.writeWord(ANCHOR_ADDR, ANCHOR_CONVERT_D1_8192, 0 );
	usleep(20);

	uint8_t tempStore[3];

	D1 = 0;
	for(int i = 0; i < 3 ; i++){

		i2cdev.readByte(ANCHOR_ADDR, ANCHOR_ADC_READ, &tempStore[i]);


	}
	//TODO Matt Pedler - Verify that this is actually making the right number
	//change D1 to right value

	D1 = tempStore[1] + (tempStore[2] * 256);

	i2cdev.writeWord(ANCHOR_ADDR, ANCHOR_CONVERT_D2_8192, 0 );
	usleep(20);

	D2 = 0;

	for(int i = 0; i < 3 ; i++){

		i2cdev.readByte(ANCHOR_ADDR, ANCHOR_ADC_READ, &tempStore[i]);


	}


	D2 = tempStore[1] + (tempStore[2] * 256);
	calculate();




}

void Anchor::readTestCase(){

	C[0] = 0;
	C[1] = 34982;
	C[2] = 36352;
	C[3] = 20328;
	C[4] = 22354;
	C[5] = 26646;
	C[6] = 26146;
	C[7] = 0;

	D1 = 4958179;
	D2 = 6815414;

	calculate();

}

float Anchor::altitude(){
	return (1-pow((pressure()/1013.25),.190284))*145366.45*.3048;
}

float Anchor::depth(){
	return (pressure(Anchor::Pa)-101300)/(fluidDensity*9.80665);
}

float Anchor::pressure(float conversion){
	return P/10.0f*conversion;
}

void Anchor::setFluidDensity(float density){
	fluidDensity = density;

}

float Anchor::temperature(){
	return TEMP/100.0f;
}

uint8_t Anchor::crc4(uint16_t n_prom[]) {
	uint16_t n_rem = 0;

	n_prom[0] = ((n_prom[0]) & 0x0FFF);
	n_prom[7] = 0;

	for ( uint8_t i = 0 ; i < 16; i++ ) {
		if ( i%2 == 1 ) {
			n_rem ^= (uint16_t)((n_prom[i>>1]) & 0x00FF0);
		} else {
			n_rem ^= (uint16_t)(n_prom[i>>1] >> 8);
		}
		for ( uint8_t n_bit = 8 ; n_bit > 0 ; n_bit-- ) {
			if ( n_rem & 0x8000 ) {
				n_rem = (n_rem << 1) ^ 0x3000;
			} else {
				n_rem = (n_rem << 1);
			}
		}
	}

	n_rem = ((n_rem >> 12) & 0x000F);

	return n_rem ^ 0x00;
}

void Anchor::calculate(){
	// Given C1-C6 and D1, D2, calculated TEMP and P
		// Do conversion first and then second order temp compensation

		int32_t dT;
		int64_t SENS;
		int64_t OFF;
		int32_t SENSi;
		int32_t OFFi;
		int32_t Ti;
		int64_t OFF2;
		int64_t SENS2;

		// Terms called
		dT = D2-uint32_t(C[5])*256l;
		SENS = int64_t(C[1])*32768l+(int64_t(C[3])*dT)/256l;
		OFF = int64_t(C[2])*65536l+(int64_t(C[4])*dT)/128l;


		//Temp and P conversion
		TEMP = 2000l+int64_t(dT)*C[6]/8388608LL;
		P = (D1*SENS/(2097152l)-OFF)/(8192l);

		//Second order compensation
		if((TEMP/100)<20){         //Low temp
			Ti = (3*int64_t(dT)*int64_t(dT))/(8589934592LL);
			OFFi = (3*(TEMP-2000)*(TEMP-2000))/2;
			SENSi = (5*(TEMP-2000)*(TEMP-2000))/8;
			if((TEMP/100)<-15){    //Very low temp
				OFFi = OFFi+7*(TEMP+1500l)*(TEMP+1500l);
				SENSi = SENSi+4*(TEMP+1500l)*(TEMP+1500l);
			}
		}
		else if((TEMP/100)>=20){    //High temp
			Ti = 2*(dT*dT)/(137438953472LL);
			OFFi = (1*(TEMP-2000)*(TEMP-2000))/16;
			SENSi = 0;
		}

		OFF2 = OFF-OFFi;           //Calculate pressure and temp second order
		SENS2 = SENS-SENSi;

		TEMP = (TEMP-Ti);
		P = (((D1*SENS2)/2097152l-OFF2)/8192l);
}
