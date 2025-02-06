/*
 * File:   Lab7T1.c
 * Author: hadia
 *
 * Created on November 3, 2024, 7:15 PM
 */


#include "xc.h"
#include "p24fj128ga010.h"

#pragma config FNOSC = FRCPLL

#define sysFreq 32000000UL
#define freqCY sysFreq/2
#define mode 16
#define baudRate 9600
#define baudVal (int) ((freqCY/baudRate)/mode)-1
unsigned char Rxdata[1024];

int16_t ADCread(void){
    AD1CON1bits.SAMP = 1;
    while(!AD1CON1bits.DONE);
    return ADC1BUF0;
}

void sendDataBuff (const char *buff, uint32_t size){
    while(size){
        while(!U1STAbits.TRMT);
        U1TXREG = *buff;
        buff++;
        size--;
    }
    while(!U1STAbits.TRMT);
}

int main(void) {
    int16_t ADCvalue;
    double f_ADCvalue;
    int temp;
    
    AD1PCFG = 0xFF00; //AD0..AD7 analog
    AD1CON1 = 0x81E0; //manual sample & auto convert with signed integer
    AD1CON2 = 0x0000;
    AD1CON3 = 0x1F18;
    AD1CHS = 0x0007; // positive at AN7
    AD1CSSL = 0;
    TRISB = 0x0080;
    TRISE = 0;
    PORTD = 0;
 
    T2CON = 0x8030;
    PR2 = 31249;
    U1MODEbits.UARTEN = 0;
    U1MODEbits.STSEL = 0; // 1-Stop bit
    U1MODEbits.PDSEL = 0; // No Parity, 8-Data bits
    U1MODEbits.ABAUD = 0; // Auto-Baud disabled
    U1MODEbits.BRGH = 0; // slow mode
    U1BRG = baudVal;
    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;
    while(1){
        ADCvalue = ADCread();
        
        if ((ADCvalue & 0x0200) == 0){
            ADCvalue = ADCvalue;
        }
        else{
            ADCvalue = ADCvalue + 0x0200;
        }
        //ssss sssd dddd dddd
        f_ADCvalue = (ADCvalue*5.0) / (1023.0); //volt
        temp = (int) (((f_ADCvalue*10000)-4985)/100); //temp convert
        //
        if (temp < 18 && temp > 14){
            _RE9 = 1;
            _RE0 = 0;
            sprintf(Rxdata, "\r\nTemperature is getting too low!\r\n");
            sendDataBuff(Rxdata, strlen(Rxdata));
        }
        else if (temp < 14){
            _RE9 = 0;
            _RE0 = 1;
            sprintf(Rxdata, "\r\nTemperature is getting too critically low! ALARM!!\r\n");
            sendDataBuff(Rxdata, strlen(Rxdata));
        }
        else if (temp > 24 && temp < 32){
            _RE9 = 1;
            _RE0 = 0;
            sprintf(Rxdata, "\r\nTemperature is getting too high!\r\n");
            sendDataBuff(Rxdata, strlen(Rxdata));
        }
        else if (temp > 32){
            _RE9 = 0;
            _RE0 = 1;
            sprintf(Rxdata, "\r\nTemperature is getting too critically high! ALARM!!\r\n");
            sendDataBuff(Rxdata, strlen(Rxdata));
        }
        else{
            _RE9 = 0;
            _RE0 = 0;
        }
        sprintf(Rxdata, "\r\nCurrent Temperature is: %d.0 Celsius\r\n", temp);
        sendDataBuff(Rxdata, strlen(Rxdata));
        while(TMR2);
    }
    U1MODEbits.UARTEN = 0;
    return 0;
}
