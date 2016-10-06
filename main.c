/* 
 * File:   main.c
 * Author: pc
 *
 
 */

#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
#include "i2c.h"


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

#include <xc.h>

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG1
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#define _XTAL_FREQ 1000000

/*
 * 
 */
float ADCvalue;
float voltage;

unsigned char set, count, mode, quit, alarm;
unsigned char second, minute, hour, hour_display, date, day, month, year, day_alarm, hour_alarm, hour_alarm_display, minute_alarm, temperature;

void Fix_Time() {
    //fix thu
    if (day == 0) day = 7;
    else if (day == 8) day = 1;
    //fix thang
    if (month == 0) month = 12;
    else if (month == 13) month = 1;
    //fix nam
    if (year == 255) year = 99;
    else if (year == 100) year = 0;
    //fix ngay
    switch (month) {
        case 2:
        {
            if (year & 3) {
                if (date == 0) date = 28;
                else if (date == 29 || date == 30 || date == 31) date = 1;
            } else {
                if (date == 0) date = 29;
                else if (date == 30 || date == 31) date = 1;
            }
            break;
        }
        case 4: case 6: case 9: case 11:
        {
            if (date == 0) date = 30;
            else if (date == 31) date = 1;
            break;
        }
        default:
        {
            if (date == 0) date = 31;
            else if (date == 32) date = 1;
            break;
        }
    }
}

void Fix_Alarm() {
    if (minute_alarm == 255) minute_alarm = 59;
    else if (minute_alarm == 60) minute_alarm = 0;

    if (hour_alarm < 40 || hour_alarm == 255) { //24h        
        if (hour_alarm == 255) hour_alarm = 23;
        else if (hour_alarm == 24) hour_alarm = 0;
    } else if (hour_alarm < 60) { // 12h AM
        if (hour_alarm == 40) hour_alarm = 52;
        else if (hour_alarm == 51 && RB3 == 0) hour_alarm = 71;
        else if (hour_alarm == 52 && RB2 == 0) hour_alarm = 72;
        else if (hour_alarm == 53 && RB2 == 0) hour_alarm = 41;
    } else { //12h PM 
        if (hour_alarm == 60) hour_alarm = 72;
        else if (hour_alarm == 71 && RB3 == 0) hour_alarm = 51;
        else if (hour_alarm == 72 && RB2 == 0) hour_alarm = 52;
        else if (hour_alarm == 73 && RB2 == 0) hour_alarm = 61;
    }
}

void Read_FullTime() {
    second = DS1307_Read(0x00);
    minute = DS1307_Read(0x01);
    hour = DS1307_Read(0x02);
    day = DS1307_Read(0x03);
    date = DS1307_Read(0x04);
    month = DS1307_Read(0x05);
    year = DS1307_Read(0x06);
}

void Read_Time() {
    second = DS1307_Read(0x00);
    if (second == 0) minute = DS1307_Read(0x01);
    if (minute == 0) hour = DS1307_Read(0x02);
    if (hour == 0 || hour == 52) {
        day = DS1307_Read(0x03);
        date = DS1307_Read(0x04);
    }
    if (date == 1) month = DS1307_Read(0x05);
    if (month == 1) year = DS1307_Read(0x06);
}

void Read_Tem() {
    ADCON0bits.GO_nDONE = 1;
    while (ADCON0bits.GO_nDONE);
    ADCvalue = (ADRESH << 8) | ADRESL;
    voltage = ADCvalue * 5000.0f / 1023;
    temperature = voltage / 10;
}

void Write_Min() {
    if (second == 255) second = 59;
    else if (second == 60) second = 0;

    if (minute == 255) minute = 59;
    else if (minute == 60) minute = 0;
    DS1307_Write(0x00, 0);
    DS1307_Write(0x01, minute);
}

void Write_Hour() {

    if (hour < 40 || hour == 255) { //24h        
        if (hour == 255) hour = 23;
        else if (hour == 24) hour = 0;
    } else if (hour < 60) { // 12h AM
        if (hour == 40) hour = 52;
        else if (hour == 51 && RB3 == 0) hour = 71;
        else if (hour == 52 && RB2 == 0) hour = 72;
        else if (hour == 53 && RB2 == 0) hour = 41;
    } else { //12h PM 
        if (hour == 60) hour = 72;
        else if (hour == 71 && RB3 == 0) hour = 51;
        else if (hour == 72 && RB2 == 0) hour = 52;
        else if (hour == 73 && RB2 == 0) hour = 61;
    }
    DS1307_Write(0x02, hour);
}

void Write_Time() {
    Fix_Time();
    DS1307_Write(0x03, day);
    DS1307_Write(0x04, date);
    DS1307_Write(0x05, month);
    DS1307_Write(0x06, year);
}

void Change_Mode12h() {
    if (hour < 40) {
        //thay doi gio hien thi
        if (hour == 0) hour = 52;
        else if (hour == 12) hour = 72;
        else if (hour > 12) hour = hour + 48;
        else hour = hour + 40;
        // thay doi gio bao thuc
        if (hour_alarm == 0) hour_alarm = 52;
        else if (hour_alarm == 12) hour_alarm = 72;
        else if (hour_alarm > 12) hour_alarm = hour_alarm + 48;
        else hour_alarm = hour_alarm + 40;
    } else {
        //thay doi gio hien thi
        if (hour == 52) hour = 0;
        else if (hour == 72) hour = 12;
        else if (hour < 60) hour = hour - 40;
        else hour = hour - 48;
        //thay doi gio bao thuc
        if (hour_alarm == 52) hour_alarm = 0;
        else if (hour_alarm == 72) hour_alarm = 12;
        else if (hour_alarm < 60) hour_alarm = hour_alarm - 40;
        else hour_alarm = hour_alarm - 48;
    }
}
//////////////////////////////////////////////////////////////////////
//////////////////////    KHOI HIEN THI    ///////////////////////////
//////////////////////////////////////////////////////////////////////

void Display(unsigned char One) {
    Lcd_Write_Char(One / 10 + 0x30);
    Lcd_Write_Char(One % 10 + 0x30);
}

void Display_Hour() {
    if (hour < 40) hour_display = hour; //24h
    else if (hour < 60) hour_display = hour - 40; // 12h AM
    else if (hour > 60) hour_display = hour - 60; //12h PM
}

void Display_HourAlarm() {
    if (hour_alarm < 40) hour_alarm_display = hour_alarm; //24h
    else if (hour_alarm < 60) hour_alarm_display = hour_alarm - 40; // 12h AM
    else if (hour_alarm > 60) hour_alarm_display = hour_alarm - 60; //12h PM
}

//void Display_Day() {
//    switch (day % 10) {
//        case 1: Lcd_Write_String("SUN");
//            break;
//        case 2: Lcd_Write_String("MON");
//            break;
//        case 3: Lcd_Write_String("TUE");
//            break;
//        case 4: Lcd_Write_String("WES");
//            break;
//        case 5: Lcd_Write_String("THU");
//            break;
//        case 6: Lcd_Write_String("FRI");
//            break;
//        case 7: Lcd_Write_String("SAT");
//            break;
//        default: Lcd_Write_String("   ");
//            break;
//    }
//}

void Display_AM_PM() {
    if (hour < 40) Lcd_Write_String("   ");
    else if (hour < 60) Lcd_Write_String(" AM");
    else Lcd_Write_String(" PM");
}

void Display_Alarm() {
    if (alarm) {
        Lcd_Set_Cursor(1, 1);
        Lcd_Write_String("XOA HEN GIO?    ");

        Lcd_Set_Cursor(2, 1);
        Display_HourAlarm();
        Display(hour_alarm_display);

        Lcd_Write_Char(':');
        Display(minute_alarm);
        if (hour < 40) Lcd_Write_String("   ");
        else if (hour < 60) Lcd_Write_String(" AM");
        else Lcd_Write_String(" PM");
        Lcd_Write_String("           ");
    } else {
        Lcd_Set_Cursor(1, 1);
        Lcd_Write_String("HEN GIO         ");
        Lcd_Set_Cursor(2, 1);

        switch (set) {
                //chon gio
            case 1:
            {
                Lcd_Write_String("GIO: ");
                Lcd_Set_Cursor(2, 6);
                Display_HourAlarm();
                Display(hour_alarm_display);
                //Display_AM_PM();
                if (hour < 40) Lcd_Write_String("   ");
                else if (hour < 60) Lcd_Write_String(" AM");
                else Lcd_Write_String(" PM");
                break;
            }
                //chon phut
            case 2:
            {
                Lcd_Write_String("PHUT: ");
                Display(minute_alarm);
                Lcd_Write_String("  ");
                break;
            }
        }
        Lcd_Set_Cursor(2, 11);
        Lcd_Write_String("      ");

    }
}

void Display_Time() {

    Lcd_Set_Cursor(1, 1);

    if (alarm)
        Lcd_Write_String("ALM ");
    else Lcd_Write_String("    ");

    //hien thi gio
    Display_Hour();
    Display(hour_display);

    //hien thi phut
    Lcd_Write_Char(':');
    Display(minute);

    //hien thi giay
    Lcd_Write_Char(':');
    Display(second);

    Lcd_Set_Cursor(1, 13);
    //Display_AM_PM();
    if (hour < 40) Lcd_Write_String("   ");
    else if (hour < 60) Lcd_Write_String(" AM");
    else Lcd_Write_String(" PM");

    // hien thi thu
    Lcd_Set_Cursor(2, 1);
    //Display_Day();
    switch (day % 10) {
        case 1: Lcd_Write_String("SUN");
            break;
        case 2: Lcd_Write_String("MON");
            break;
        case 3: Lcd_Write_String("TUE");
            break;
        case 4: Lcd_Write_String("WES");
            break;
        case 5: Lcd_Write_String("THU");
            break;
        case 6: Lcd_Write_String("FRI");
            break;
        case 7: Lcd_Write_String("SAT");
            break;
        default: Lcd_Write_String("   ");
            break;
    }

    //hien thi ngay
    Lcd_Set_Cursor(2, 4);
    Lcd_Write_Char(' ');
    Display(date);

    //hien thi thang
    Lcd_Write_Char('-');
    Display(month);

    //hien thi nam
    Lcd_Write_Char('-');
    Display(year);

    //hien thi nhiet do
    Lcd_Set_Cursor(2, 13);
    Lcd_Write_Char(' ');
    Display(temperature);
    Lcd_Write_Char(67);
}

void Display_SetTime() {
    //chon che do gio
    if (quit == 1 && hour < 40) {
        Lcd_Set_Cursor(1, 1);
        Lcd_Write_String("CHE DO 12h?     ");
        Lcd_Set_Cursor(2, 1);
        Lcd_Write_String("                ");
    } else if (quit == 1 && hour >= 40) {
        Lcd_Set_Cursor(1, 1);
        Lcd_Write_String("CHE DO 24h?     ");
        Lcd_Set_Cursor(2, 1);
        Lcd_Write_String("                ");

    } else if (quit == 2) {
        Lcd_Set_Cursor(1, 1);
        switch (set) {
            case 0:
            {
                Lcd_Write_String("CHINH SUA?      ");
                Lcd_Set_Cursor(2, 1);
                Lcd_Write_String("                ");
                break;
            }
            case 1:
            {
                Lcd_Write_String("CHINH SUA      ");
                Lcd_Set_Cursor(2, 1);
                Lcd_Write_String("PHUT: ");
                Display(minute);
                break;
            }
            case 2:
            {
                Lcd_Write_String("CHINH SUA      ");
                Lcd_Set_Cursor(2, 1);
                Lcd_Write_String("GIO: ");
                Display_Hour();
                Display(hour_display);
                //Display_AM_PM();
                if (hour < 40) Lcd_Write_String("   ");
                else if (hour < 60) Lcd_Write_String(" AM");
                else Lcd_Write_String(" PM");
                break;
            }
            case 3:
            {
                Lcd_Write_String("CHINH SUA      ");
                Lcd_Set_Cursor(2, 1);
                Lcd_Write_String("THU: ");
                //Display_Day();
                switch (day % 10) {
                    case 1: Lcd_Write_String("SUN");
                        break;
                    case 2: Lcd_Write_String("MON");
                        break;
                    case 3: Lcd_Write_String("TUE");
                        break;
                    case 4: Lcd_Write_String("WES");
                        break;
                    case 5: Lcd_Write_String("THU");
                        break;
                    case 6: Lcd_Write_String("FRI");
                        break;
                    case 7: Lcd_Write_String("SAT");
                        break;
                    default: Lcd_Write_String("   ");
                        break;
                }
                Lcd_Write_String("   ");
                break;
            }
            case 4:
            {
                Lcd_Write_String("CHINH SUA      ");
                Lcd_Set_Cursor(2, 1);
                Lcd_Write_String("NGAY: ");
                Display(date);
                break;
            }
            case 5:
            {
                Lcd_Write_String("CHINH SUA      ");
                Lcd_Set_Cursor(2, 1);
                Lcd_Write_String("THANG: ");
                Display(month);
                break;
            }
            case 6:
            {
                Lcd_Write_String("CHINH SUA      ");
                Lcd_Set_Cursor(2, 1);
                Lcd_Write_String("NAM: ");
                Lcd_Write_Char(0x32);
                Lcd_Write_Char(0x30);
                Display(year);

                break;
            }
        }

    }
}
//////////////////////////////////////////////////////////////////
///////////////// KHOI NGAT NGOAI XU LY NUT BAM //////////////////
//////////////////////////////////////////////////////////////////

void interrupt ISR() {

    if (RBIF) {
        //nut hen gio
        if (RB1 == 0 && mode != 1) {
            mode = 2;
            quit++;
            set = 1;
            if (minute == minute_alarm && hour == hour_alarm && alarm) {
                alarm = 0;
                mode = 0;
                set = 0;
                quit = 0;
            } else if (quit == 2) {
                set = 0;
                quit = 0;
                mode = 0;
            }
            if (mode == 0) Read_FullTime();
        }
        //nut OK
        if (RB4 == 0 && mode != 0) {
            set++;
            if (mode == 2) {
                if (alarm) {
                    quit = 0;
                    alarm = 0;
                    mode = 0;
                    set = 0;
                } else if (set == 3) {
                    quit = 0;
                    set = 0;
                    mode = 0;
                    alarm = 1;
                }
            } else if (quit == 2 && set == 7) {
                set = 0;
                mode = 0;
                quit = 0;
            } else if (quit == 1) { //doi che do 24h - 12h
                set = 0;
                Change_Mode12h();
                DS1307_Write(0x02, hour);
            }
            if (mode == 0) Read_FullTime();
        }
        //nut chinh thoi gian
        if (RB5 == 0 && mode != 2) {
            mode = 1;
            quit++;
            set = 0;
            if (quit == 3) {
                quit = 0;
                mode = 0;
            }
            if (mode == 0) Read_FullTime();
        }
        //nut tang
        if (RB2 == 0) {
            if (mode == 1 && quit == 2) {
                switch (set) {
                    case 1: minute++;
                        Write_Min();
                        break;
                    case 2: hour++;
                        Write_Hour();
                        break;
                    case 3: day++;
                        break;
                    case 4: date++;
                        break;
                    case 5: month++;
                        break;
                    case 6: year++;
                        break;
                }
                Write_Time();
            } else if (mode == 2) {
                switch (set) {
                    case 1: hour_alarm++;
                        break;
                    case 2: minute_alarm++;
                        break;
                }
                Fix_Alarm();
            }
        }
        //nut giam
        if (RB3 == 0) {
            if (mode == 1 && quit == 2) {
                switch (set) {
                    case 1: minute--;
                        Write_Min();
                        break;
                    case 2: hour--;
                        Write_Hour();
                        break;
                    case 3: day--;
                        break;
                    case 4: date--;
                        break;
                    case 5: month--;
                        break;
                    case 6: year--;
                        break;
                }
                Write_Time();
            } else if (mode == 2) {
                switch (set) {
                    case 1: hour_alarm--;
                        break;
                    case 2: minute_alarm--;
                        break;
                }
                Fix_Alarm();
            }
        }
        INTCONbits.RBIF = 0;
    }
    if (INTCONbits.T0IF) {
        count++;
        if (count > 120 && mode == 0) {
            Read_Time();
            Read_Tem();
            count = 0;
        }
        INTCONbits.T0IF = 0;
        TMR0 = 6;
    }
}

int main() {
    TRISD = 0;
    PORTD = 0;
    TRISB = 0xFF;
    ANSEL = 0x00;
    ANSELH = 0x00;
    ///////////////////
    TRISA1 = 1;
    ANSELbits.ANS1 = 1;
    ADCON1 = 0x80;
    ADCON0 = 0x45;
    ///////////////////
    count = 0;
    set = 0;
    mode = 0;
    quit = 0;
    voltage = 0;
    ADCvalue = 0;
    day_alarm = 2;
    hour_alarm = 00;
    minute_alarm = 55;
    //////////////////
    OPTION_REGbits.nRBPU = 0; //Cho phep dung tro keo cua PORTB
    WPUBbits.WPUB = 0xFF; //Keo tro noi cho chan RB1, RB2
    INTCONbits.GIE = 1;
    INTCONbits.RBIE = 1;
    IOCBbits.IOCB = 0xFF;
    INTCONbits.RBIF = 0;
    /////////////////
    OPTION_REGbits.T0CS = 0;
    INTCONbits.T0IF = 0;
    PSA = 0;
    OPTION_REGbits.PS = 0b010;
    INTCONbits.T0IE = 1;
    TMR0 = 6;
    /////////////////
    i2c_Init();
    Lcd_Init();
    /////////////////
    Read_FullTime();
    /////////////////////
    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String("     HELLO      ");
    Lcd_Set_Cursor(2, 1);
    Lcd_Write_String(" GROUP 01 VXL!  ");
    __delay_ms(1000);
    //////////////////

    while (1) {

        __delay_ms(20);
        if (minute == minute_alarm && hour == hour_alarm && alarm)
            RD0 = 1;
        else RD0 = 0;
        if (mode == 1) {
            Display_SetTime();
        } else if (mode == 2) {
            Display_Alarm();
        } else Display_Time();
    }
    return (EXIT_SUCCESS);
}