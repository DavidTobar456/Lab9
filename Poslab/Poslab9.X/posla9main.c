/*
 * File:   posla9main.c
 * Author: david
 *
 * Created on 13 de octubre de 2021, 08:43 PM
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 8000000

void configuracion(void);

uint8_t conteo, limite;

void __interrupt()isr(){
    if(PIR1bits.ADIF){
        if(ADCON0bits.CHS==0b1100){
            CCPR1L=(ADRESH>>1)+124;
            CCP1CONbits.DC1B1 = ADRESH & 0b01;
            CCP1CONbits.DC1B0 = (ADRESL>>7);
            ADCON0bits.CHS=0b1010;
        }else if(ADCON0bits.CHS==0b1010){
            CCPR2L=(ADRESH>>1)+124;
            CCP2CONbits.DC2B1 = ADRESH & 0b01;
            CCP2CONbits.DC2B0 = (ADRESL>>7);
            ADCON0bits.CHS=0b0111;
        }else if(ADCON0bits.CHS==0b0111){
            limite=(char)(ADRESH/5);
            ADCON0bits.CHS=0b1100;
        }
        PIR1bits.ADIF=0;
    }
    if(INTCONbits.T0IF){
        conteo=conteo+1;
        if(conteo==0){
            conteo=205;
        }
        if(conteo<(limite+205)){
            PORTCbits.RC0=1;
        }else{
            PORTCbits.RC0=0;
        }
        INTCONbits.T0IF=0;
        TMR0=230;
    }
}

void main(void) {
    configuracion();
    conteo=205;
    while(1){
        if(!ADCON0bits.GO){
            __delay_us(50);
            ADCON0bits.GO=1;
        }
    }
}

void configuracion(void){
    // Configuración de los puertos como digitales o analógicos
    ANSEL=0;
    ANSELH=0;
    ANSELHbits.ANS12=1;
    ANSELHbits.ANS10=1;
    
    // Se configuran los puertos pertinentes como entradas o como salidas
    TRISCbits.TRISC1=1;
    TRISCbits.TRISC2=1; // Se desactivan los pines correspondientes a CCP1, CCP2
    TRISBbits.TRISB0=1;
    TRISBbits.TRISB1=1; // RB0 y RB1 serán entradas analógicas
    TRISEbits.TRISE2=1; // RE2 será una entrada analógica
    
    // Se limpia el registro PORTB
    PORTB=0;
    
    // Configuración del oscilador
    OSCCONbits.SCS=1;
    OSCCONbits.IRCF=0b111; // Se configura el oscilador interno a 8MHz
    
    // Configuración de las interrupciones
    INTCONbits.GIE=1;
    INTCONbits.PEIE=1;
    INTCONbits.T0IE=1;
    INTCONbits.T0IF=0;
    PIE1bits.ADIE=1;
    PIR1bits.ADIF=0;
    
    // Configuración de ADCON
    ADCON0bits.ADON=1;
    ADCON0bits.CHS=0b1100;
    ADCON0bits.ADCS=0b01;
    __delay_us(50);
    ADCON0bits.GO=1;
    
    ADCON1bits.ADFM=0;
    ADCON1bits.VCFG0=0;
    ADCON1bits.VCFG1=0;
    
    // Configuramos el periodocon PR2
    PR2=249;
    
    // Configuración del CCP1
    CCP1CONbits.P1M=0;
    CCP1CONbits.CCP1M=0b1100;
    CCPR1L=0x0F;
    CCP1CONbits.DC1B=0b11;
    
    // Configuración del CCP2
    CCP2CONbits.CCP2M=0b1100;
    CCPR2L=0x0F;
    CCP2CONbits.DC2B1=1;
    CCP2CONbits.DC2B0=1;
    
    // Configuración de TMR2
    PIR1bits.TMR2IF=0;
    T2CONbits.T2CKPS=0b11;
    T2CONbits.TMR2ON=1;
    
    while(!PIR1bits.TMR2IF);
    PIR1bits.TMR2IF=0;
    
    // Activación del PWM
    TRISC=0;
    
    
    // Configuración de Timer0
    OPTION_REGbits.T0CS=0;
    OPTION_REGbits.PSA=0;
    OPTION_REGbits.PS=0b000;
    TMR0=230;
}
