/*********************************************************************
 * Projekat radili: Nikola Jevtic EE164/2019, Marija Bojic EE99/2019,
 * Milan Stanojevic EE100/2019 i Dragisa Jakisic EE32/2019.
 * Zadatak je bio da se emulira rad brisaca na automobilu preko servo
 * motora SG90. Sistem se pali slanjem ON sa terminala. Pri sobnom osve-
 * tljenju, servo motor ne radi - sija sunce. Kada se uperi u fotootpornik
 * blic, simuliramo pljusak, stoga brisaci rade najvecom brzinom. Ukoliko 
 * prekrijemo fotootpornik i vrednosti AD konverzije sa fotootpornika postanu
 * veoma male ~ 0010, kisa sipi. Prilikom konstantnog rada servo mo-
 * tora(kisa sipi i kisa pljusti), cela ploca je opterecena,
 * pogotovo zbog AD konverzije, pa u tom slucaju
 * ostali senzori ne rade, ali dok sija sunce, svi senzori rade normalno.
 * Datum odbrane projekta: 17.2.2023.
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include<p30fxxxx.h>
#include "glcd_lib.h"
#include "adc.h"


_FOSC(CSW_FSCM_OFF & XT_PLL4);//instruction takt je isti kao i kristal 10MHz
_FWDT(WDT_OFF);


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

//Delay za tocuh screen
void Delay(unsigned int N)
{
	unsigned int i;
	for(i=0;i<N;i++);
}
void Touch_Panel (void)
{
// vode horizontalni tranzistori
	DRIVE_A = 1;  
	DRIVE_B = 0;
    
     LATCbits.LATC13=1;
     LATCbits.LATC14=0;

	Delay(500); //cekamo jedno vreme da se odradi AD konverzija
				
	// ocitavamo x	
	x_vrednost = temp0;//temp0 je vrednost koji nam daje AD konvertor na BOTTOM pinu		

	// vode vertikalni tranzistori
     LATCbits.LATC13=0;
     LATCbits.LATC14=1;
	DRIVE_A = 0;  
	DRIVE_B = 1;

	Delay(500); //cekamo jedno vreme da se odradi AD konverzija
	
	// ocitavamo y	
	y_vrednost = temp1;// temp1 je vrednost koji nam daje AD konvertor na LEFT pinu	
	
//Ako ?elimo da nam X i Y koordinate budu kao rezolucija ekrana 128x64 treba skalirati vrednosti x_vrednost i y_vrednost tako da budu u opsegu od 0-128 odnosno 0-64
//skaliranje x-koordinate

    X=(x_vrednost-161)*0.03629;



//X= ((x_vrednost-AD_Xmin)/(AD_Xmax-AD_Xmin))*128;	
//vrednosti AD_Xmin i AD_Xmax su minimalne i maksimalne vrednosti koje daje AD konvertor za touch panel.


//Skaliranje Y-koordinate
	Y= ((y_vrednost-500)*0.020725);

//	Y= ((y_vrednost-AD_Ymin)/(AD_Ymax-AD_Ymin))*64;
}

/***************************************************************************
* Ime funkcije      : initUART1                                            *
* Opis              : inicjalizuje RS232 komunikaciju s 9600bauda          * 
* Parameteri        : Nema                                                 *
* Povratna vrednost : Nema                                                 *
***************************************************************************/

void initUART1(void)
{
    U1BRG=0x0040;//baud rate 9600
    U1MODEbits.ALTIO = 0; //Koristimo UART1 sa osnovnim pinovima - RF2 i RF3
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
							

										sirovi0=ADCBUF0; //TAC
                                        sirovi1=ADCBUF1; //TAC
                                        sirovi2=ADCBUF2; //LM34
                                        sirovi3=ADCBUF3; //PIR
                                        sirovi4=ADCBUF4; //FOTO
                                        temp0=sirovi0;
                                        temp1=sirovi1;
										
										

    IFS0bits.ADIF = 0;

} 

/*********************************************************************
* Ime funkcije      : WriteUART1                            		 *
* Opis              : Funkcija upisuje podatke u registar U1TXREG,   *
*                     za slanje podataka    						 *
* Parameteri        : unsigned int data-podatak koji zelimo poslati  *
* Povratna vrednost : Nema                                           *
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
* Ime funkcije      : WriteUART1dec2string                     		   *
* Opis              : Funkcija salje 4-cifrene brojeve (cifru po cifru)*
* Parameteri        : unsigned int data-podatak koji zelimo poslati    *
* Povratna vrednost : Nema                                             *
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
//funkcija za kasnjenje u milisekundama
*********************************************************************/
void Delay_ms (int vreme)//funkcija za kasnjenje u milisekundama
	{
		stoperica = 0;
		while(stoperica < vreme);
	}
void Delay_ms3 (int vreme3)//funkcija za kasnjenje u milisekundama
	{
		stoperica3 = 0;
		while(stoperica3 < vreme3);
	}


//PREKIDNA RUTINA TAJMERA 2 
void __attribute__ ((__interrupt__)) _T2Interrupt(void) // svakih 1ms
{

		TMR2 =0;
     ms=1;//fleg za milisekundu ili prekid;potrebno ga je samo resetovati u funkciji

	brojac_ms++;//brojac milisekundi
    stoperica++;//brojac za funkciju Delay_ms

    if (brojac_ms==1000)//sek
        {
          brojac_ms=0;
          sekund=1;//fleg za sekundu
		 } 
	IFS0bits.T2IF = 0; 
       
}
//PREKIDNA RUTINA TAJMERA 3
void __attribute__ ((__interrupt__)) _T3Interrupt(void) // svakih 1ms
{

		TMR3 =0;
     ms3=1;//fleg za milisekundu ili prekid;potrebno ga je samo resetovati u funkciji

	brojac_ms3++;//brojac milisekundi
    stoperica3++;//brojac za funkciju Delay_ms

    if (brojac_ms3==1000)//sek
        {
          brojac_ms3=0;
          sekund3=1;//fleg za sekundu
		 } 
	IFS0bits.T3IF = 0; 
       
}

//Funkcija za pokretanje sistema - kada se sa terminala posalje ON, sistem se pokrece
void start(){
    
    if(buf[0] == 'O' && buf[1] == 'N') {
            //GLCD_ClrScr();
            flag=1;
            GoToXY(1,3);
            GLCD_Printf("Sistem je upaljen");
            Delay_ms3(500);
            //GLCD_ClrScr();
        } 
}

//Funkcija koja, kada je pritisnut ekran, omogucava da se proveri da li je RD8 upaljena ili ne
void pritisnut_ekran(){
    
    if(X >0  && X < 128 && Y > 0 && Y <64){         
            GLCD_ClrScr();
            GoToXY(1,3);
            GLCD_Printf("Ekran je pritisnut");
            GoToXY(1,4);
            GLCD_Printf("Proveri da li je dioda RD8 upaljena");//Posto LM35 radi okej u slucaju kada motor ne radi, vuce ad konverzija previse,    
            Delay_ms(5000);                                    //na ovaj nacin zaustavljamo sve i proveravamo da li se senzor zagrejao do temperature od 30
            GLCD_ClrScr();                                     //stepeni, ako jeste, pali se lampica RD8 (5/4095=0,001220703125; 0,001220703125/0,01=0,1220703125;
    }                                                          //                                         30/0,1220703125=245,76)
}

//Funkcija koja ispisuje string na terminalu
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

//Ispisivanje vrednosti AD konverzije sa LM35 preko UART-a na terminalu
void ispis_temp(){  
    
        RS232_putst("TEMP ");
		WriteUART1dec2string(sirovi2);
        for(broj2=0;broj2<100;broj2++);
		WriteUART1(13);//enter
}

//Funkcija koja pali diodu RD8 ukoliko je postignuta zeljena temperatura na LM35
void pali_rd8(){
    
    if(sirovi2>250){
            LATDbits.LATD8=1;
        } 
        else {
            LATDbits.LATD8=0;
        }
}

 //Ispisivanje vrednosti AD konverzije sa fotootpornika preko UART-a na terminalu
void ispis_foto(){
    
        RS232_putst("FOTO ");
		WriteUART1dec2string(sirovi4);
        for(broj2=0;broj2<100;broj2++);
		WriteUART1(13);//enter
}


//Ispisivanje vrednosti AD konverzije sa PIR senzora preko UART-a na terminalu
void ispis_pir(){
    
        RS232_putst("PIR ");
		WriteUART1dec2string(sirovi3);
		for(broj2=0;broj2<100;broj2++);
		WriteUART1(13);//enter
}

//Funkcija koja simulira parking senzore - kada se detektuje pokret preko PIR senzora, pali se piezo buzzer
void parking_senzori(){
    
        if(sirovi3>2000){
                //Piezo Buzzer
                LATAbits.LATA11=1;
                Delay_ms(10);
                LATAbits.LATA11=0;
                Delay_ms(19);
            } 
}


//Simuliranje rada brisaca na autu preko servo motora SG90, u zavisnosti od vrednosti konverzije sa fotootpornika,
//pale se brisaci 
void brisaci(){
    
    if(sirovi4<=150){
                
            GLCD_ClrScr();
            GoToXY(0,0);
            GLCD_Printf("Kisa pljusti");
            
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
            GLCD_Printf("Sunce sija");
        }
    
    if(sirovi4>1800){
                 
            GLCD_ClrScr(); 
            GoToXY(0,0);
            GLCD_Printf("Kisa sipi");
            
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

        flag = 0; //Inicijalna vrednost flag-a
        Init_T2(); //Inicijalizacija tajmera 2
        Init_T3(); //Inicijalizacija tajmera 3
        
		initUART1();//inicijalizacija UART-a
 		ADCinit();//inicijalizacija AD konvertora
        ADCON1bits.ADON=1;//pocetak Ad konverzije 
        
        //Inicijalizacija GLCD-a
        ConfigureAllPins();
        GLCD_LcdInit();
        GLCD_ClrScr();
        
        //Touch screen
        ConfigureTSPins();
        
        TRISDbits.TRISD9=0; //Izlaz za servo
        TRISDbits.TRISD8=0; //Izlaz za LM35(pali diodu RD8 kada detektuje odredjenu temperaturu)
        TRISAbits.TRISA11=0;//Izlaz za buzzer
        
        
	while(1)
		{
         
        start();
        
        if(flag==1){
            Touch_Panel();
            pritisnut_ekran();                                                        //                                         30/0,1220703125=245,76)
            ispis_temp();         
            pali_rd8();
            ispis_foto();
            ispis_pir();
            parking_senzori();
            brisaci();
        
        }
      
                  
		}//while  

    return (EXIT_SUCCESS);
}