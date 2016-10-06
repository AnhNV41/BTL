/* 
 * File:   lcd.h
 * Author: pc
 *
 * Created on Ngày 15 tháng 4 n?m 2016, 14:42
 */

#ifndef LCD_H
#define	LCD_H

void Lcd_Port(char a);
void Lcd_Cmd(char a);
void Lcd_Set_Cursor(char a, char b);
void Lcd_Init();
void Lcd_Write_Char(char a);
void Lcd_Write_String(char *a);
void Lcd_Shift_Right();
void Lcd_Shift_Left();

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

