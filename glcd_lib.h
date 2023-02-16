/*
 *
 * Biblioteka za rad sa Graphic LCD modulom
 *
 * 
 * Napomena:
 * Bio je problem pri stavljanju fontova u zaseban fajl gde je kompajler
 * ispisavao problem da postoje vise definicija "const char font" iako nije.
 * Zbog toga je potrebno drzati fontove samo u glcd_lib.c. 
 * Za slike i slicno, drzati u main fajlu.
 * 
 */


#ifndef GLCD_LIB_H_
#define GLCD_LIB_H_





/* --------------------------------------------------------------------------------------------- */ 


// Registri i generalne def. za rad

/* LCD Registers */
#define PAGE_ADRESS	0xB8	/* Adress base for Page 0 */
#define Y_ADRESS	0x40	/* Adress base for Y0	  bilo 40*/
#define START_LINE	0xC0	/* Adress base for line 0 */
#define DISPLAY_ON	0x3F	/* Turn display on	  */
#define DISPLAY_OFF	0x3E	/* Turn display off	  */


/* General use definitions */
#define RIGHT		0
#define LEFT		1
#define BUSY		0x80

#define FONT_LENGHT 5
#define FONT_LENGHT_BIG 16

#define odnos_pravougaonosti_piksela		1.3

/* Koristi se kod definisanja pravca pina ulazni ili izlazni */
#define output 0	
#define input 1

#define RF5_1 LATF|=0b0000000000100000;	//RST
#define RF5_0 LATF&=0b1111111111011111;

#define RF4_1 LATF|=0b0000000000010000;	//E
#define RF4_0 LATF&=0b1111111111101111;

#define RF1_1 LATF|=0b0000000000000010;	//RW
#define RF1_0 LATF&=0b1111111111111101;

#define RF0_1 LATF|=0b0000000000000001;	//RS
#define RF0_0 LATF&=0b1111111111111110;

#define RB5_1 LATB|=0b0000000000100000;	//CS2
#define RB5_0 LATB&=0b1111111111011111;

#define RB4_1 LATB|=0b0000000000010000;	//CS1
#define RB4_0 LATB&=0b1111111111101111;

/* nisam siguran? */
#define Nop() {__asm__ volatile ("nop");}

//#define xtal 29480000
#define xtal 25000000



/* --------------------------------------------------------------------------------------------- */ 

/* Konfiguracione funkcije */


/* Konfiguracija registara */
void SetRST(unsigned char vrednost);
void SetE(unsigned char vrednost);
void SetRW(unsigned char vrednost);
void SetRS(unsigned char vrednost);
void SetCS2(unsigned char vrednost);
void SetCS1(unsigned char vrednost);



/*
 *	Konfiguracija SVIH pinova za rad sa LCD - om
 */
void ConfigureAllPins (void);


/* 
 *	Setovanje LCD pinova na input ili output
 */
void ConfigureLcdData(unsigned char direction);


/* 
 *	Setovanje vrednosti LCD pinova
 */
void SetLcdData(unsigned char vrednost);


/* 
 *	Citanje vrednosti LCD pinova
 */
unsigned char ReadLcdData(void);


/*
 * Strobe
 * Valjda ukljucuje i iskljucuje svetlo kao refresh neki
 */
void strobe_data(void); 

/* --------------------------------------------------------------------------------------------- */ 


 
/* Funkcije za rad sa GLCD - om */
// ovo se ustvari koristi

/* 
 *	Slanje instrukcija modulu
 */
void LcdInstructionWrite (unsigned char u8Instruction);


/* 
 *	Kratak delay za timing
 */
void LcdDelay(unsigned int u32Duration);


/* 
 *	Cekanje sve dok LCD prima informacije
 */
void LcdWaitBusy (void);


/* 
 *	Postavljanje kursora po y tj. veritkalnoj osi.
 *	Prosledjujemo vrednost 0-8
 */
void GoToY(unsigned char y);


/* 
 *	Postavljanje kursora po x tj. horizontalnoj osi.
 *	Prosledjujemo vrednost 0-128
 */
void GoToX(unsigned char x);


/* 
 *	Postavljanje kursora na (x, y) pozicije
 *	Koristi funkcije GotoX, GoToY
 */
void GoToXY(unsigned char x,unsigned char y);


/* 
 *	Koristimo za skroolovanje ekrana tj. tu polovinu koju postavimo startline.
 *	Ona celu polovinu translira gore dole da joj je vrh na liniji koju postavimo.
 *	Prosledjujemo vrednost 0-64
 */
void LcdSelectStartline(unsigned char startna_linija);

/* 
 *	Selektujemo stranu LCD - a.
 *	Prosledjujemo vrednost LEFT ili RIGHT
 */
void LcdSelectSide(unsigned char u8LcdSide);


/* 
 *	Funkcija za citanje informacije sa LCD - a.
 *	Koristi funkciju ReadLcdData
 *  Vraca vrednost unsigned char
 *	nzm zasto se ne koristi samo originalna funkcija
 */
unsigned char LcdDataRead (void);


/* 
 *	Inicijalizacija GLCD modula	
 */
void GLCD_LcdInit(void);



/* Uglavnom se koriste ove naredne funkcije */
/* --------------------------------------------------------------------------------------------- */ 


/* 
 *	Salje informaciju LCD - u.
 *	Prosledjujemo vrednosti tipa unsigned char
 */
void GLCD_DataWrite (unsigned char u8Data);

/* 
 *	Brisanje ekrana
 */
void GLCD_ClrScr (void);


/* 
 *	Ispuni ceo ekran
 */
void GLCD_FillScr (void);


/* 
 *	Prikaz slike na ekranu.
 *	Sliku je potrebno pretvoriti u formu niza tj matrice.
 *	Prosledjujemo adresu niza
 */
void GLCD_DisplayPicture (unsigned char *picture);


/* 
 *	Postavljanje tacke na (x, y) poziciju
 */
void GLCD_SetDot (unsigned char u8Xaxis, unsigned char u8Yaxis);


/* 
 *	Postavljanje praznu tacku na (x, y) poziciju
 */
void GLCD_ResDot (unsigned char u8Xaxis, unsigned char u8Yaxis);


/*
 *	Crta krug na ekranu
 *	Prosledjujemo vrednosti:
 *		u8CenterX = Center absciss (in pixels)
 *		u8CenterY = Center ordinate (in pixels) 
 *		u8Radius  = Radius (in pixels)
 */
void GLCD_Circle (unsigned char u8CenterX, unsigned char u8CenterY, unsigned char u8Radius);


/*
 *	Crta pravougaonik na ekranu
 *	Prosledjujemo vrednosti:
 *		u8Xaxis1 = absciss top-left (in pixels)
 *		u8Yaxis1 = ordinate top-left (in pixels)
 *		u8Xaxis2 = absciss bottom-right (in pixels)
 *		u8Yaxis2 = ordinate bottom-right (in pixels)
 */
void GLCD_Rectangle (unsigned char u8Xaxis1,unsigned char u8Yaxis1,unsigned char u8Xaxis2,unsigned char u8Yaxis2);


/* 
 * Ispisuje se 1 karakter na ekranu.
 * Prebacivanje u novi red pri preteku i vracanje na 0-ti red kada prodje poslednji
 */ 
void GLCD_PutChar (char AskiKod);


/* 
 * Ispisuje se 1 VELIKI karakter na ekranu.
 * Prebacivanje u novi red pri preteku i vracanje na 0-ti red kada prodje poslednji
 */ 
void GLCD_PutCharBig (char AskiKod);


/* 
 * Ispisuje strin na ekranu.
 */ 
void GLCD_Printf (char *au8Text);


/* 
 * Iscrtava grid na ekranu
 */ 
void GLCD_ShowGrid(unsigned char razmak_grid);



#endif /* GLCD_LIB_H_ */