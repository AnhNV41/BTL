/* 
 * File:   i2c.h
 * Author: pc
 *
 * Created on Ngày 17 tháng 4 n?m 2016, 16:05
 */

#ifndef I2C_H
#define	I2C_H
void i2c_Init();
void i2c_Start();
void i2c_Restart();
void i2c_Stop();
void i2c_Ack();
void i2c_Nack();
void i2c_Write(unsigned char data);
unsigned char i2c_Read();
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void DS1307_Write(unsigned char add, unsigned char dat);
unsigned char DS1307_Read(unsigned char add);
#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* I2C_H */

