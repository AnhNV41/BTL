#include "i2c.h"
#include <xc.h>
#define _XTAL_FREQ 1000000

void i2c_Init(){
    SSPCON = 0x28;   // SSPCON = 0b00101000      
    SSPADD = 9;
    SMP = 1;
}

void i2c_Start(){
    SEN = 1;
    while(SSPIF == 0); //do nothing
    SSPIF = 0;
}

void i2c_Restart(){
    RSEN = 1;
    while(SSPIF == 0); //do nothing
    SSPIF = 0;
}

void i2c_Stop(){
    PEN = 1;
    while(SSPIF == 0); //do nothing
    SSPIF = 0;
}

void i2c_Ack(){
    ACKDT = 0;      //confirm
    ACKEN = 1;      //enable
    while(SSPIF == 0); //do nothing
    SSPIF = 0;
}

void i2c_Nack(){
    ACKDT = 1;
    ACKEN = 1;      //enable
    while(SSPIF == 0); //do nothing
    SSPIF = 0;
}

void i2c_Write(unsigned char data){
    SSPBUF = data;
    while(SSPIF == 0); //do nothing
    SSPIF = 0;
}

unsigned char i2c_Read(){
    RCEN = 1;               //order read buffer
    while(SSPIF == 0);      //do nothing wait buffer full
    SSPIF = 0;              
    return SSPBUF;          //return 
}

//----------------------------------------
//--------------DS1307--------------------
//----------------------------------------

void DS1307_Write(unsigned char add, unsigned char dat){
    unsigned char tmp;
	tmp = ((dat / 10) << 4) | (dat % 10); 
    i2c_Start();
    i2c_Write(0xD0); // want to write to DS1307 
    i2c_Write(add);  // want to write to addREG
    i2c_Write(tmp); // write data to addREG
    i2c_Stop();
}

unsigned char DS1307_Read(unsigned char add){
    unsigned char data;
    i2c_Start();
    i2c_Write(0xD0); 
    i2c_Write(add);
    
    i2c_Restart();
    i2c_Write(0xD1);    //want to read from DS1307
    data = i2c_Read(); //read data from add
    i2c_Nack();
    i2c_Stop();
    unsigned char temp = (data >> 4) * 10 + (data & 0x0f);
	return temp;
}
