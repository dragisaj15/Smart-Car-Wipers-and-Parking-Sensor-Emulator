/*********************************************************************
 * Project Authors: Nikola Jevtic EE164/2019, Marija Bojic EE99/2019,
 * Milan Stanojevic EE100/2019 and Dragisa Jakisic EE32/2019.
 * Task: Emulate car wipers using an SG90 servo motor.
 * The system is turned on by sending "ON" from the serial terminal.
 * At room lighting (sunny), the servo motor is idle. 
 * Shining a flashlight at the photoresistor simulates a heavy downpour,
 * so the wipers run at maximum speed. If the photoresistor is covered
 * and ADC values become very low (~0010), it simulates light rain.
 * During constant servo operation (light or heavy rain), the board is 
 * under heavy load (especially due to ADC conversion), so other sensors 
 * are disabled. However, when it is sunny, all sensors operate normally.
 * Project Defense Date: Feb 17, 2023.
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <p30fxxxx.h>
#include "glcd_lib.h"
#include "adc.h"

_FOSC(CSW_FSCM_OFF & XT_PLL4); // Instruction clock is the same as the 10MHz crystal
_FWDT(WDT_OFF);                // Disable Watchdog Timer

unsigned int sirovi0,sirovi1,sirovi2,sirovi3,sirovi4;
unsigned int broj,broj1,broj2,tempRX, temp0, temp1, n, flag;
unsigned int brojac_ms,stoperica,ms,sekund,brojac_ms3,stoperica3,ms3,sekund3;
unsigned int X, Y,x_vrednost, y_vrednost;
//const unsigned int ADC_THRESHOLD = 900; 
const unsigned int AD_Xmin =220;
const unsigned int AD_Xmax =3642;
const unsigned int AD_Ymin =520;
const unsigned int AD_Ymax =3450;

int buf[1];

#define DRIVE_A PORTCbits.RC13
#define DRIVE_B PORTCbits.RC14

void ConfigureTSPins(void)
{
	//ADPCFGbits.PCFG10=1;
	//ADPCFGbits.PCFG7=digital;

	//TRISBbits.TRISB10=0;
	TRISCbits.TRISC13=0;
    TRISCbits.TRISC14=0;
	
	//LATCbits.LATC14=0;
	//LATCbits.LATC13=0;
}

// Delay for touch screen
void Delay(unsigned int N)
{
	unsigned int i;
	for(i=0;i<N;i++);
}

void Touch_Panel (void)
{
    // Horizontal transistors are conducting
	DRIVE_A = 1;  
	DRIVE_B = 0;
    
    LATCbits.LATC13=1;
    LATCbits.LATC14=0;

	Delay(500); // Wait for AD conversion to complete
				
	// Read X coordinate	
	x_vrednost = temp0; // temp0 is the AD converter value on the BOTTOM pin		

	// Vertical transistors are conducting
    LATCbits.LATC13=0;
    LATCbits.LATC14=1;
	DRIVE_A = 0;  
	DRIVE_B = 1;

	Delay(500); // Wait for AD conversion to complete
	
	// Read Y coordinate	
	y_vrednost = temp1; // temp1 is the AD converter value on the LEFT pin	
	
    // To match X and Y coordinates to the 128x64 screen resolution, 
    // scale x_vrednost and y_vrednost to the 0-128 and 0-64 ranges respectively.
    // Scaling the X coordinate
    X=(x_vrednost-161)*0.03629;

    // X = ((x_vrednost-AD_Xmin)/(AD_Xmax-AD_Xmin))*128;	
    // AD_Xmin and AD_Xmax are the min and max ADC values for the touch panel.

    // Scaling the Y coordinate
	Y= ((y_vrednost-500)*0.020725);
    // Y= ((y_vrednost-AD_Ymin)/(AD_Ymax-AD_Ymin))*64;
}

/***************************************************************************
* Function Name     : initUART1                                            *
* Description       : Initializes RS232 communication at 9600 baud rate    * 
* Parameters        : None                                                 *
* Return Value      : None                                                 *
***************************************************************************/
void initUART1(void)
{
    U1BRG=0x0040; // baud rate 9600
    U1MODEbits.ALTIO = 0; // Use UART1 with default pins - RF2 and RF3
    IEC0bits.U1RXIE = 1;
    U1STA&=0xfffc;
    U1MODEbits.UARTEN=1;
    U1STAbits.UTXEN=1;
}

void __attribute__((__interrupt__)) _U1RXInterrupt(void) 
{
    IFS0bits.U1RXIF = 0;
    tempRX=U1RXREG;
    if(tempRX!=0){
        buf[n] = tempRX;
        if(n<2) {
            n++;
        } 
        else {
            n = 0;
        }
    }
} 

void __attribute__((__interrupt__)) _ADCInterrupt(void) 
{
    sirovi0=ADCBUF0; // Touch screen
    sirovi1=ADCBUF1; // Touch screen
    sirovi2=ADCBUF2; // LM34
    sirovi3=ADCBUF3; // PIR
    sirovi4=ADCBUF4; // FOTO (Photoresistor)
    temp0=sirovi0;
    temp1=sirovi1;
										
    IFS0bits.ADIF = 0;
} 

/*********************************************************************
* Function Name     : WriteUART1                            		 *
* Description       : Writes data to the U1TXREG register            *
*                     to transmit data    						     *
* Parameters        : unsigned int data - data to be sent            *
* Return Value      : None                                           *
*********************************************************************/
void WriteUART1(unsigned int data)
{
	while (U1STAbits.TRMT==0);
    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else 
        U1TXREG = data & 0xFF;
}

/***********************************************************************
* Function Name     : WriteUART1dec2string                     		   *
* Description       : Sends 4-digit numbers (digit by digit)           *
* Parameters        : unsigned int data - data to be sent              *
* Return Value      : None                                             *
************************************************************************/
void WriteUART1dec2string(unsigned int data)
{
	unsigned char temp;

	temp=data/1000;
	WriteUART1(temp+'0');
	data=data-temp*1000;
	temp=data/100;
	WriteUART1(temp+'0');
	data=data-temp*100;
	temp=data/10;
	WriteUART1(temp+'0');
	data=data-temp*10;
	WriteUART1(data+'0');
}

/*******************************************************************
// Function for delay in milliseconds
*********************************************************************/
void Delay_ms (int vreme) 
{
    stoperica = 0;
    while(stoperica < vreme);
}

void Delay_ms3 (int vreme3) 
{
    stoperica3 = 0;
    while(stoperica3 < vreme3);
}

// TIMER 2 INTERRUPT ROUTINE
void __attribute__ ((__interrupt__)) _T2Interrupt(void) // Every 1ms
{
    TMR2 = 0;
    ms=1; // millisecond or interrupt flag; only needs to be reset in the function

    brojac_ms++; // millisecond counter
    stoperica++; // counter for Delay_ms function

    if (brojac_ms==1000) // 1 second reached
    {
        brojac_ms=0;
        sekund=1; // second flag
    } 
    IFS0bits.T2IF = 0; 
}

// TIMER 3 INTERRUPT ROUTINE
void __attribute__ ((__interrupt__)) _T3Interrupt(void) // Every 1ms
{
    TMR3 = 0;
    ms3=1; // millisecond or interrupt flag; only needs to be reset in the function

    brojac_ms3++; // millisecond counter
    stoperica3++; // counter for Delay_ms function

    if (brojac_ms3==1000) // 1 second reached
    {
        brojac_ms3=0;
        sekund3=1; // second flag
    } 
    IFS0bits.T3IF = 0; 
}

// Function to start the system - when "ON" is sent from the terminal, system starts
void start(){
    
    if(buf[0] == 'O' && buf[1] == 'N') {
            //GLCD_ClrScr();
            flag=1;
            GoToXY(1,3);
            GLCD_Printf("System is ON");
            Delay_ms3(500);
            //GLCD_ClrScr();
        } 
}

// Function that allows checking if the RD8 LED is ON when the screen is pressed
void pritisnut_ekran(){
    
    if(X >0  && X < 128 && Y > 0 && Y <64){         
            GLCD_ClrScr();
            GoToXY(1,3);
            GLCD_Printf("Screen pressed");
            GoToXY(1,4);
            GLCD_Printf("Check if RD8 LED is ON"); // Since LM35 works fine when motor is off, but ADC draws   
            Delay_ms(5000);                        // too much load otherwise, this stops everything to check 
            GLCD_ClrScr();                         // if sensor reached 30 deg. If so, RD8 LED turns on.
    }                                              // (5/4095=0.001220703125; 0.001220703125/0.01=0.1220703125;
                                                   // 30/0.1220703125=245.76)
}

// Function that prints a string to the serial terminal
void RS232_putst(register const char *str)
{
    while((*str!=0))
    {
        WriteUART1(*str);
        if(*str == 13) WriteUART1(10);
        if(*str == 10) WriteUART1(13);
        str++;
    }
}

// Print LM35 ADC values via UART to the terminal
void ispis_temp(){  
    
        RS232_putst("TEMP ");
		WriteUART1dec2string(sirovi2);
        for(broj2=0;broj2<100;broj2++);
		WriteUART1(13); // enter
}

// Function that turns on the RD8 LED if the desired temperature is reached on LM35
void pali_rd8(){
    
    if(sirovi2>250){
            LATDbits.LATD8=1;
        } 
        else {
            LATDbits.LATD8=0;
        }
}

// Print photoresistor ADC values via UART to the terminal
void ispis_foto(){
    
        RS232_putst("FOTO ");
		WriteUART1dec2string(sirovi4);
        for(broj2=0;broj2<100;broj2++);
		WriteUART1(13); // enter
}

// Print PIR sensor ADC values via UART to the terminal
void ispis_pir(){
    
        RS232_putst("PIR ");
		WriteUART1dec2string(sirovi3);
		for(broj2=0;broj2<100;broj2++);
		WriteUART1(13); // enter
}

// Simulates parking sensors - when movement is detected via PIR, piezo buzzer is activated
void parking_senzori(){
    
        if(sirovi3>2000){
                // Piezo Buzzer
                LATAbits.LATA11=1;
                Delay_ms(10);
                LATAbits.LATA11=0;
                Delay_ms(19);
            } 
}

// Simulates car wipers using the SG90 servo motor. 
// Wipers are activated based on photoresistor ADC values.
void brisaci(){
    
    if(sirovi4<=150){
                
            GLCD_ClrScr();
            GoToXY(0,0);
            GLCD_Printf("Heavy rain");
            
            for(broj2=0; broj2<25; broj2++){
                LATDbits.LATD9=1;
                Delay_ms(1);
                LATDbits.LATD9=0;
                Delay_ms(19);
            }
            
            for(broj2=0; broj2<10; broj2++){
                LATDbits.LATD9=1;
                Delay_ms(2);
                LATDbits.LATD9=0;
                Delay_ms(18);
            }
        } 
      
    if(sirovi4<=1800 && sirovi4>150){
            //GLCD_ClrScr();
            GoToXY(0,0);
            GLCD_Printf("Sunny");
        }
    
    if(sirovi4>1800){
                 
            GLCD_ClrScr(); 
            GoToXY(0,0);
            GLCD_Printf("Light rain");
            
            for(broj2=0; broj2<60; broj2++){
                LATDbits.LATD9=1;
                Delay_ms(1);
                LATDbits.LATD9=0;
                Delay_ms(19);
            }
            
            for(broj2=0; broj2<20; broj2++){
                LATDbits.LATD9=1;
                Delay_ms(2);
                LATDbits.LATD9=0;
                Delay_ms(18);
              }    
             }
}

int main(int argc, char** argv) {
    
    for(broj1=0;broj1<10000;broj1++);
    for(broj=0;broj<60000;broj++);

        flag = 0; // Initial flag value
        Init_T2(); // Timer 2 initialization
        Init_T3(); // Timer 3 initialization
        
		initUART1(); // UART initialization
 		ADCinit(); // ADC initialization
        ADCON1bits.ADON=1; // Start AD conversion
        
        // GLCD Initialization
        ConfigureAllPins();
        GLCD_LcdInit();
        GLCD_ClrScr();
        
        // Touch screen setup
        ConfigureTSPins();
        
        TRISDbits.TRISD9=0; // Output for servo
        TRISDbits.TRISD8=0; // Output for LM35 (turns on RD8 LED when specific temp is detected)
        TRISAbits.TRISA11=0; // Output for buzzer
        
        
	while(1)
		{
         
        start();
        
        if(flag==1){
            Touch_Panel();
            pritisnut_ekran();                                                                                                 
            ispis_temp();         
            pali_rd8();
            ispis_foto();
            ispis_pir();
            parking_senzori();
            brisaci();
        }
      
		} // end while  

    return (EXIT_SUCCESS);
} // end main
