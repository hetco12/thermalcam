/***************************************************
  This is a library for the Adafruit 1.8" SPI display.

This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
The 1.8" TFT shield
  ----> https://www.adafruit.com/product/802
The 1.44" TFT breakout
  ----> https://www.adafruit.com/product/2088
as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#define TFT_CS     10 //chip select pin for the TFT screen
#define TFT_RST    9  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to 0!
#define TFT_DC     8
#include "digitalfastwrite.h"
#include "Adafruit_ST77xx.h"
#include <limits.h>
#ifndef ARDUINO_STM32_FEATHER
  #include "pins_arduino.h"
  #include "wiring_private.h"
#endif
#include "SPIxx.h"

inline uint16_t swapcolor(uint16_t x) { 
  return (x << 11) | (x & 0x07E0) | (x >> 11);
}

#if defined (SPI_HAS_TRANSACTION)
  static SPISettings mySPISettings;
#elif defined (__AVR__) || defined(CORE_TEENSY)
  static uint8_t SPCRbackup;
  static uint8_t mySPCR;
#endif


#if defined (SPI_HAS_TRANSACTION)
#define SPI_BEGIN_TRANSACTION()    if (_hwSPI)    SPI.beginTransaction(mySPISettings)/***************************************************
  This is a library for the Adafruit 1.8" SPI display.

This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
The 1.8" TFT shield
  ----> https://www.adafruit.com/product/802
The 1.44" TFT breakout
  ----> https://www.adafruit.com/product/2088
as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#define safety_Checks false //this makes every spi speed safe, if we remove checks we can run as fast as possible at max spi
#define TFT_CS     10 //chip select pin for the TFT screen
#define TFT_RST    9  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to 0!
#define TFT_DC     8

#include "digitalfastwrite.h"
#include "Adafruit_ST77xx.h"
#include <limits.h>
#ifndef ARDUINO_STM32_FEATHER
  #include "pins_arduino.h"
  #include "wiring_private.h"
#endif
uint8_t hardcodeh;
uint8_t hardcodew;
#include "SPIxx.h"
//we need to define some data here for spi writes. we only do this here and not in actual spi area.
//in spi routine maybe better later on
unsigned char SPI_BUFFER[64];//looped buffer for spi
byte SPI_READ_LOCATE;//0 to 63
byte SPI_WRITE_LOCATE;//0 to 63

void BufferSPIWRITE(uint8_t data){
  //spi system is not in a write, such as it could be at slower speeds
 SPI_WRITE_LOCATE++;//since we are reading next area we increment
 //SPI_WRITE_LOCATE=SPI_WRITE_LOCATE+1&63;//we make sure it can only go 64 bytes
 //SPI_BUFFER[SPI_WRITE_LOCATE] =data;//we store byte for transfer
}
void BufferSPISEND(){//this area sends one byte of data
  if (SPI_READ_LOCATE !=SPI_WRITE_LOCATE){//we make sure we have data to send otherwise we are done
 while(!(SPSR & (1<<SPIF) ));//we do this to make absolutely sure
 //we are only sending when spi has completed. 
 SPDR= SPI_BUFFER[SPI_READ_LOCATE];
 SPI_READ_LOCATE= SPI_READ_LOCATE+1 &63;//we make sure it can only go 64 bytes
}
}


inline uint16_t swapcolor(uint16_t x) { 
  return (x << 11) | (x & 0x07E0) | (x >> 11);
}

#if defined (SPI_HAS_TRANSACTION)
  static SPISettings mySPISettings;
#elif defined (__AVR__) || defined(CORE_TEENSY)
  static uint8_t SPCRbackup;
  static uint8_t mySPCR;
#endif


#if defined (SPI_HAS_TRANSACTION)
#define SPI_BEGIN_TRANSACTION()    if (_hwSPI)    SPI.beginTransaction(mySPISettings)
#define SPI_END_TRANSACTION()      if (_hwSPI)    SPI.endTransaction()
#else
#define SPI_BEGIN_TRANSACTION()   // (void)
#define SPI_END_TRANSACTION()     // (void)
#endif

// Constructor when using software SPI.  All output pins are configurable.
Adafruit_ST77xx::Adafruit_ST77xx(int8_t cs, int8_t dc, int8_t sid, int8_t sclk, int8_t rst) 
  : Adafruit_GFX(ST7735_TFTWIDTH_128, ST7735_TFTHEIGHT_160)
{
  _cs   = cs;
  _dc   = dc;
  _sid  = sid;
  _sclk = sclk;
  _rst  = rst;
  _hwSPI = false;
}

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Adafruit_ST77xx::Adafruit_ST77xx(int8_t cs, int8_t dc, int8_t rst) 
  : Adafruit_GFX(ST7735_TFTWIDTH_128, ST7735_TFTHEIGHT_160) {
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
  _hwSPI = true;
  _sid  = _sclk = -1;
}

inline void Adafruit_ST77xx::spiwrite(uint8_t c) {

  //Serial.println(c, HEX);

  if (_hwSPI) {
#if defined (SPI_HAS_TRANSACTION)
      SPI.transfer(c);
#elif defined (__AVR__) || defined(CORE_TEENSY)
      //this is where transfer happens. try to move as much as possible here
      SPCRbackup = SPCR;
      SPCR = mySPCR;
      SPI.transfer(c);

      while (!(SPSR & _BV(SPIF)));

      SPCR = SPCRbackup;
#elif defined (__arm__)
      SPI.setClockDivider(SPI_CLOCK_DIV2); //8MHz
      SPI.setDataMode(SPI_MODE0);
      SPI.transfer(c);
#endif
  } else {

    // Fast SPI bitbang swiped from LPD8806 library
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
#if defined(USE_FAST_IO)
      if(c & bit) *dataport |=  datapinmask;
      else        *dataport &= ~datapinmask;
      *clkport |=  clkpinmask;
      *clkport &= ~clkpinmask;
#else
      if(c & bit) digitalWrite(_sid, HIGH);
      else        digitalWrite(_sid, LOW);
      digitalWrite(_sclk, HIGH);
      digitalWrite(_sclk, LOW);
#endif
    }
  }
}


void Adafruit_ST77xx::writecommand(uint8_t c) {
digitalWriteFast(TFT_DC,LOW); //DC_LOW();
 digitalWriteFast(TFT_DC,LOW); //DC_LOW();
 digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  SPI_BEGIN_TRANSACTION();

  spiwrite(c);
 digitalWriteFast(TFT_CS,HIGH); //CS_HIGH();
 digitalWriteFast(TFT_CS,HIGH); //CS_HIGH();
  SPI_END_TRANSACTION();
}


void Adafruit_ST77xx::writedata(uint8_t c) {
  SPI_BEGIN_TRANSACTION();

   digitalWriteFast(TFT_DC,HIGH); //DC_LOW();
   digitalWriteFast(TFT_DC,HIGH); //DC_LOW();
 digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  digitalWriteFast(TFT_CS,LOW);// CS_LOW();
//  DC_HIGH();
 // CS_LOW();
    
  spiwrite(c);
 digitalWriteFast(TFT_CS,HIGH);
  digitalWriteFast(TFT_CS,HIGH);
 // CS_HIGH();
  SPI_END_TRANSACTION();
}


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Adafruit_ST77xx::displayInit(const uint8_t *addr) {

  uint8_t  numCommands, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++);   // Number of commands to follow
  while(numCommands--) {                 // For each command...
    writecommand(pgm_read_byte(addr++)); //   Read, issue command
    numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
    ms       = numArgs & ST_CMD_DELAY;   //   If hibit set, delay follows args
    numArgs &= ~ST_CMD_DELAY;            //   Mask out delay bit
    while(numArgs--) {                   //   For each argument...
      writedata(pgm_read_byte(addr++));  //     Read, issue argument
    }

    if(ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      delay(ms);
    }
  }
}


// Initialization code common to all ST77XX displays
void Adafruit_ST77xx::commonInit(const uint8_t *cmdList) {
  _ystart = _xstart = 0;
  _colstart  = _rowstart = 0; // May be overridden in init func

  pinMode(_dc, OUTPUT);
  pinMode(_cs, OUTPUT);

#if defined(USE_FAST_IO)
  csport    = portOutputRegister(digitalPinToPort(_cs));
  dcport    = portOutputRegister(digitalPinToPort(_dc));
  cspinmask = digitalPinToBitMask(_cs);
  dcpinmask = digitalPinToBitMask(_dc);
#endif

  if(_hwSPI) { // Using hardware SPI
#if defined (SPI_HAS_TRANSACTION)
    SPI.begin();
    mySPISettings = SPISettings(24000000, MSBFIRST, SPI_MODE0);
#elif defined (__AVR__) || defined(CORE_TEENSY)
    SPCRbackup = SPCR;
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.setDataMode(SPI_MODE0);
    mySPCR = SPCR; // save our preferred state
    //Serial.print("mySPCR = 0x"); Serial.println(SPCR, HEX);
    SPCR = SPCRbackup;  // then restore
#elif defined (__SAM3X8E__)
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV4); //4MHz//8mhz
    SPI.setDataMode(SPI_MODE0);
#endif
  } else {
    pinMode(_sclk, OUTPUT);
    pinMode(_sid , OUTPUT);
    digitalWrite(_sclk, LOW);
    digitalWrite(_sid, LOW);

#if defined(USE_FAST_IO)
    clkport     = portOutputRegister(digitalPinToPort(_sclk));
    dataport    = portOutputRegister(digitalPinToPort(_sid));
    clkpinmask  = digitalPinToBitMask(_sclk);
    datapinmask = digitalPinToBitMask(_sid);
#endif
  }

  // toggle RST low to reset; CS low so it'll listen to us
  CS_LOW();
  if (_rst != -1) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(500);
    digitalWrite(_rst, LOW);
    delay(500);
    digitalWrite(_rst, HIGH);
    delay(500);
  }

  if(cmdList) 
    displayInit(cmdList);
}



void Adafruit_ST77xx::setRotation(uint8_t m) {

  writecommand(ST77XX_MADCTL);
  rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     writedata(ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB);

     _xstart = _colstart;
     _ystart = _rowstart;
     break;
   case 1:
     writedata(ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB);

     _ystart = _colstart;
     _xstart = _rowstart;
     break;
  case 2:
     writedata(ST77XX_MADCTL_RGB);
 
     _xstart = _colstart;
     _ystart = _rowstart;
     break;

   case 3:
     writedata(ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB);

     _ystart = _colstart;
     _xstart = _rowstart;
     break;
  }
}

void Adafruit_ST77xx::setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1,
 uint8_t y1) {

  uint16_t x_start = x0 + _xstart, x_end = x1 + _xstart;
  uint16_t y_start = y0 + _ystart, y_end = y1 + _ystart;
  

  writecommand(ST77XX_CASET); // Column addr set
  writedata(x_start >> 8);
  writedata(x_start & 0xFF);     // XSTART 
  writedata(x_end >> 8);
  writedata(x_end & 0xFF);     // XEND

  writecommand(ST77XX_RASET); // Row addr set
  writedata(y_start >> 8);
  writedata(y_start & 0xFF);     // YSTART
  writedata(y_end >> 8);
  writedata(y_end & 0xFF);     // YEND

  writecommand(ST77XX_RAMWR); // write to RAM
}


void Adafruit_ST77xx::pushColor(uint16_t color) {
  SPI_BEGIN_TRANSACTION();
   digitalWriteFast(TFT_CS,HIGH);
   digitalWriteFast(TFT_CS,HIGH);
  //DC_HIGH();
 // CS_LOW();
 digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  spiwrite(color >> 8);
  spiwrite(color);

  CS_HIGH();
  SPI_END_TRANSACTION();
}

void Adafruit_ST77xx::drawPixel(int16_t x, int16_t y, uint16_t color) {

  if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

  setAddrWindow(x,y,x+1,y+1);

  SPI_BEGIN_TRANSACTION();
  //DC_HIGH();
   digitalWriteFast(TFT_DC,HIGH); 
     digitalWriteFast(TFT_DC,HIGH);
 // CS_LOW();
 digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  spiwrite(color >> 8);
  spiwrite(color);
 digitalWriteFast(TFT_CS,HIGH);
  digitalWriteFast(TFT_CS,HIGH);
 // CS_HIGH();
  SPI_END_TRANSACTION();
}


void Adafruit_ST77xx::drawFastVLine(int16_t x, int16_t y, int16_t h,
 uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((y+h-1) >= _height) h = _height-y;
  setAddrWindow(x, y, x, y+h-1);

  uint8_t hi = color >> 8, lo = color;
    
  SPI_BEGIN_TRANSACTION();
  DC_HIGH();
  CS_LOW();

  while (h--) {
    spiwrite(hi);
    spiwrite(lo);
  }

  CS_HIGH();
  SPI_END_TRANSACTION();
}


void Adafruit_ST77xx::drawFastHLine(int16_t x, int16_t y, int16_t w,
  uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((x+w-1) >= _width)  w = _width-x;
  setAddrWindow(x, y, x+w-1, y);

  uint8_t hi = color >> 8, lo = color;

  SPI_BEGIN_TRANSACTION();
  DC_HIGH();
  CS_LOW();

  while (w--) {
    spiwrite(hi);
    spiwrite(lo);
  }

  CS_HIGH();
  SPI_END_TRANSACTION();
}



void Adafruit_ST77xx::fillScreen(uint16_t color) {
  fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void Adafruit_ST77xx::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {

  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width)  w = _width  - x;
  if((y + h - 1) >= _height) h = _height - y;

  setAddrWindow(x, y, x+w-1, y+h-1);

  uint8_t hi = color >> 8, lo = color;
    
  SPI_BEGIN_TRANSACTION();

  DC_HIGH();
  CS_LOW();
  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      spiwrite(hi);
      spiwrite(lo);
    }
  }
  CS_HIGH();
  SPI_END_TRANSACTION();
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color

// fill a rectangle
void Adafruit_ST77xx::fillRectFast(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {

  //uint8_t hi = color >> 8, lo = color;
  
  while (!(SPSR & _BV(SPIF)));
  setAddrWindow(x, y, x+w-1, y+h-1);
  x=w*h;// most of time is in pixel writes. a dealy is needed for spi to write data
 while (!(SPSR & _BV(SPIF)));
  SPI_BEGIN_TRANSACTION();
  digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
  digitalWriteFast(TFT_CS,LOW);// old CS_LOW();
    while ( x>0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
 //SPCR = SPCRbackup;  //we place at top for next loop iteration
 SPCRbackup = SPCR; //not sure what this does or if it is really needed, but we keep at begining of loop unroll

      SPCR = mySPCR;
      #if safety_Checks != false   
       while (!(SPSR & _BV(SPIF)));
       #endif
      __asm__("nop\n\t");  
      SPDR = highByte(color);
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); //delayed for fast spi not to need another loop
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      SPCR = mySPCR;
      #if safety_Checks != false   
       while (!(SPSR & _BV(SPIF)));
       #endif
       __asm__("nop\n\t"); 
      SPDR =lowByte(color);
      if (x==1){ return;}//shave time off of negating zero 
           x--;
      
//we are useing the jmp and loop call times to reduce the timing needed here.
// since loop time is at the delay needed for spi, this is as efficient as it can be, as every clock
//per color pixel draw for fill is flooded to spi!    
    }//x
     
  digitalWriteFast(TFT_CS,HIGH);//old CS_HIGH();
 SPI_END_TRANSACTION();
}

//below we have complex code that is less painful to look at if it is in #define
#if safety_Checks != false             
#define complexspi while(!(SPSR & (1<<SPIF) ));SPCRbackup = SPCR; SPCR = mySPCR; SPDR 
#else
#define complexspi SPCRbackup = SPCR; SPCR = mySPCR; SPDR 
#endif
//this is small delay
#define shortSpiDelay __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); 

//last comment out ';' not needed here, but can be added in commands later 
#define longSpiDelay __asm__("nop\n\t"); __asm__("nop\n\t");__asm__("nop\n\t"); __asm__("nop\n\t");  __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t");__asm__("nop\n\t");// __asm__("nop\n\t");  


void Adafruit_ST77xx::fillRectFast4colors(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color0,uint16_t color1,uint16_t color2,uint16_t color3) {
  while(!(SPSR & (1<<SPIF) ));
  setAddrWindow(x, y, x+w-1, y+h-1);

  SPI_BEGIN_TRANSACTION();
  while(!(SPSR & (1<<SPIF) ));
  if (hardcodeh == 0){//we only set one time 
hardcodeh= h/2;//we have values in ram for further sub steps
hardcodew= w/2;}  

digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
digitalWriteFast(TFT_CS,LOW);// old CS_LOW();

uint16_t countx;uint16_t county;//we use these to understand what to draw!
SPCR = SPCRbackup;  //we place at top for next loop iteration
 county=hardcodeh;while (county >0 ){countx=hardcodew;while (countx >0) {complexspi= highByte(color0);longSpiDelay;complexspi=lowByte(color0) ;shortSpiDelay;countx--;}
countx=hardcodew; while (countx >0) {                                    complexspi=highByte(color1);longSpiDelay;complexspi=lowByte(color1);shortSpiDelay;countx--;}
county--; }//end of first half of rectange [0][1] drawn!
county=hardcodeh;while (county >0){countx=hardcodew;while (countx >0) {  complexspi=highByte(color2);longSpiDelay;complexspi=lowByte(color2);shortSpiDelay;countx--;}
countx=hardcodew;while (countx >0) {                                     complexspi= highByte(color3);longSpiDelay;complexspi=lowByte(color3);shortSpiDelay; countx--;}
county--; }//end of first half of rectange [2][3] drawn!

digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
digitalWriteFast(TFT_CS,HIGH);//old CS_HIGH();
SPI_END_TRANSACTION();
}


void Adafruit_ST77xx::fillRectFast16colors(int16_t x, int16_t y, int16_t w, int16_t h,
             uint16_t color0, uint16_t color1, uint16_t color2, uint16_t color3){
//this is where squares are written together!
  setAddrWindow(x, y, x+w-1, y+h-1);
  //uses more memory, but i think it is more efficient
  if (hardcodeh == 0){//we only set one time 
hardcodeh= h/4;//we have values in ram for further sub steps
hardcodew= w/4;} 

  SPI_BEGIN_TRANSACTION();

  //created complex commands in #define so code can be a lot cleaner
  //[4][5][6][7] 
  //[8][9][A][B] //this is how command updates screen
  //[C][D][E][F] //a,b,c,d,e,f are 10,11,12,13,14,15
digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
digitalWriteFast(TFT_CS,LOW);// old CS_LOW();

uint16_t countx;uint16_t county;//we use these to understand what to draw!

SPCR = SPCRbackup;  //we place at top for next loop iteration
 county=hardcodeh;while (county >0 ){countx=hardcodew;while (countx >0) {complexspi =highByte(color0);longSpiDelay; complexspi =lowByte(color0); shortSpiDelay;countx--;} 
countx=hardcodew;while (countx >0) {complexspi =highByte(color1);longSpiDelay;complexspi=lowByte(color1);shortSpiDelay;countx--;}
countx=hardcodew;while (countx >0) {complexspi =highByte(color2);longSpiDelay; complexspi =lowByte(color2); shortSpiDelay;countx--;}
countx=hardcodew;while (countx >0) {complexspi =highByte(color3);longSpiDelay;complexspi=lowByte(color3);shortSpiDelay;countx--;}
    county--;};//                     [0][1][2][3] drawn!
}
void Adafruit_ST77xx::fillRectFast16colors1(uint16_t color4, uint16_t color5, uint16_t color6, uint16_t color7){
  
  uint16_t countx;uint16_t county;//we use these to understand what to draw!
 county=hardcodeh;while (county >0 ){countx=hardcodew;while (countx >0) {complexspi =highByte(color4);longSpiDelay; complexspi =lowByte(color4); shortSpiDelay;countx--; }
countx=hardcodew;while (countx >0) {complexspi =highByte(color5);longSpiDelay;complexspi=lowByte(color5);shortSpiDelay;countx--;}
countx=hardcodew;while (countx >0) {complexspi =highByte(color6);longSpiDelay; complexspi =lowByte(color6); shortSpiDelay;countx--;}
countx=hardcodew;while (countx >0) {complexspi =highByte(color7);longSpiDelay;complexspi=lowByte(color7);shortSpiDelay;countx--;}
    county--;};  //                   [4][5][6][7] drawn!  
}
void Adafruit_ST77xx::fillRectFast16colors2(uint16_t color8, uint16_t color9, uint16_t color10, uint16_t color11){
  

  uint16_t countx;uint16_t county;//we use these to understand what to draw!
 county=hardcodeh;while (county >0 ){countx=hardcodew;while (countx >0) {complexspi =highByte(color8);longSpiDelay; complexspi =lowByte(color8); shortSpiDelay;countx--;}
countx=hardcodew;while (countx >0) {complexspi =highByte(color9);longSpiDelay;complexspi=lowByte(color9);shortSpiDelay;countx--;}
countx=hardcodew;while (countx >0) {complexspi =highByte(color10);longSpiDelay; complexspi =lowByte(color10); shortSpiDelay;countx--;}
countx=hardcodew;while (countx >0) {complexspi =highByte(color11);longSpiDelay;complexspi=lowByte(color11);shortSpiDelay;countx--;}
    county--;}; //                    [8][9][A][B] drawn!
}
void Adafruit_ST77xx::fillRectFast16colors3(uint16_t color12, uint16_t color13, uint16_t color14, uint16_t color15){
 
  uint16_t countx;uint16_t county;//we use these to understand what to draw!
 county=hardcodeh;while (county >0 ){countx=hardcodew;while (countx >0) {complexspi =highByte(color12);longSpiDelay; complexspi =lowByte(color12); shortSpiDelay;countx--;}
countx=hardcodew;while (countx >0) {complexspi =highByte(color13);longSpiDelay;complexspi=lowByte(color13);shortSpiDelay;countx--;}
countx=hardcodew;while (countx >0) {complexspi =highByte(color14);longSpiDelay; complexspi =lowByte(color14); shortSpiDelay;countx--;}
countx=hardcodew;while (countx >0) {complexspi =highByte(color15);longSpiDelay;complexspi=lowByte(color15);shortSpiDelay;countx--;}
 county--;}; //                       [C][D][E][F] drawn! 
 //while(!(SPSR & (1<<SPIF) ));

digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
digitalWriteFast(TFT_CS,HIGH);//old CS_HIGH();
SPI_END_TRANSACTION();             
                
}

void Adafruit_ST77xx::fillRectFast64colors(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color0, uint16_t color1, uint16_t color2, uint16_t color3, uint16_t color4, uint16_t color5, uint16_t color6, uint16_t color7){
             // [00][01][02][03]  [04][05][06][07] //we process 64 rectangles at a time, color data will be processed into 16x16 pixels.
             // [08][09][10][11]  [12][13][14][15]//each rectangle in this mode is down to 2 x 2
             // [16][17][18][19]  [20][21][22][23]
             // [24][25][26][27]  [28][29][30][31]

             
             // [32][33][34][35]  [36][37][38][39]
             // [40][41][42][43]  [44][45][46][47]
             // [48][49][50][51]  [52][53][54][55]
             // [56][57][58][59]  [60][61][62][63]
  //this is where squares are written together!
  setAddrWindow(x, y, x+w-1, y+h-1);
  SPI_BEGIN_TRANSACTION();
  //created complex commands in #define so code can be a lot cleaner
  //this is a complex command that complexspi= hi[0] value. we need to condence code at tight as possible here so many commands will share lines
hardcodeh= h/8;//we have values in ram for further sub steps
hardcodew= w/8;
digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
digitalWriteFast(TFT_CS,LOW);// old CS_LOW();
  uint16_t countx; uint16_t county;//we use these to understand what to draw!
// while(!(SPSR & (1<<SPIF) ));
SPCR = SPCRbackup;  //we place at top for next loop iteration. while loop back is done during spi burst
county=hardcodeh;while (county >0 ){//row0 
countx=hardcodew; while (countx >0) {complexspi = highByte(color0);longSpiDelay; complexspi =lowByte(color0); shortSpiDelay;countx--;}//col0
countx=hardcodew; while (countx >0) {complexspi = highByte(color1);longSpiDelay; complexspi =lowByte(color1); shortSpiDelay;countx--;}//col1
countx=hardcodew; while (countx >0) {complexspi = highByte(color2);longSpiDelay; complexspi =lowByte(color2); shortSpiDelay;countx--;}//col2
countx=hardcodew; while (countx >0) {complexspi = highByte(color3);longSpiDelay; complexspi =lowByte(color3); shortSpiDelay;countx--;}//col3
countx=hardcodew; while (countx >0) {complexspi = highByte(color4);longSpiDelay; complexspi =lowByte(color4); shortSpiDelay;countx--;}//col4
countx=hardcodew; while (countx >0) {complexspi = highByte(color5);longSpiDelay; complexspi =lowByte(color5); shortSpiDelay;countx--;}//col5
countx=hardcodew; while (countx >0) {complexspi = highByte(color6);longSpiDelay; complexspi =lowByte(color6); shortSpiDelay;countx--;}//col6                          
countx=hardcodew; while (countx >0) {complexspi = highByte(color7);longSpiDelay; complexspi =lowByte(color7); shortSpiDelay;countx--;}//col7  
county--;}
}//end of part0
void Adafruit_ST77xx::fillRectFast64colors1(uint16_t color8, uint16_t color9, uint16_t color10, uint16_t color11, uint16_t color12, uint16_t color13, uint16_t color14, uint16_t color15){
 uint16_t countx; uint16_t county;//we use these to understand what to draw!
// while(!(SPSR & (1<<SPIF) ));
county=hardcodeh;while (county >0 ){//row1 
countx=hardcodew; while (countx >0) {complexspi = highByte(color8);longSpiDelay; complexspi =lowByte(color8); shortSpiDelay;countx--;}//col0
countx=hardcodew; while (countx >0) {complexspi = highByte(color9);longSpiDelay; complexspi =lowByte(color9); shortSpiDelay;countx--;}//col1
countx=hardcodew; while (countx >0) {complexspi = highByte(color10);longSpiDelay; complexspi =lowByte(color10); shortSpiDelay;countx--;}//col2
countx=hardcodew; while (countx >0) {complexspi = highByte(color11);longSpiDelay; complexspi =lowByte(color11); shortSpiDelay;countx--;}//col3
countx=hardcodew; while (countx >0) {complexspi = highByte(color12);longSpiDelay; complexspi =lowByte(color12); shortSpiDelay;countx--;}//col4
countx=hardcodew; while (countx >0) {complexspi = highByte(color13);longSpiDelay; complexspi =lowByte(color13); shortSpiDelay;countx--;}//col5
countx=hardcodew; while (countx >0) {complexspi = highByte(color14);longSpiDelay; complexspi =lowByte(color14); shortSpiDelay;countx--;}//col6                          
countx=hardcodew; while (countx >0) {complexspi = highByte(color15);longSpiDelay; complexspi =lowByte(color15); shortSpiDelay;countx--;}//col7  
county--;}
}//end of part 1
void Adafruit_ST77xx::fillRectFast64colors2(uint16_t color16, uint16_t color17, uint16_t color18, uint16_t color19, uint16_t color20, uint16_t color21, uint16_t color22, uint16_t color23){
uint16_t countx; uint16_t county;//we use these to understand what to draw!
// while(!(SPSR & (1<<SPIF) ));
county=hardcodeh;while (county >0 ){//row2 
countx=hardcodew; while (countx >0) {complexspi = highByte(color16);longSpiDelay; complexspi =lowByte(color16); shortSpiDelay;countx--;}//col0
countx=hardcodew; while (countx >0) {complexspi = highByte(color17);longSpiDelay; complexspi =lowByte(color17); shortSpiDelay;countx--;}//col1
countx=hardcodew; while (countx >0) {complexspi = highByte(color18);longSpiDelay; complexspi =lowByte(color18); shortSpiDelay;countx--;}//col2
countx=hardcodew; while (countx >0) {complexspi = highByte(color19);longSpiDelay; complexspi =lowByte(color19); shortSpiDelay;countx--;}//col3
countx=hardcodew; while (countx >0) {complexspi = highByte(color20);longSpiDelay; complexspi =lowByte(color20); shortSpiDelay;countx--;}//col4
countx=hardcodew; while (countx >0) {complexspi = highByte(color21);longSpiDelay; complexspi =lowByte(color21); shortSpiDelay;countx--;}//col5
countx=hardcodew; while (countx >0) {complexspi = highByte(color22);longSpiDelay; complexspi =lowByte(color22); shortSpiDelay;countx--;}//col6                          
countx=hardcodew; while (countx >0) {complexspi = highByte(color23);longSpiDelay; complexspi =lowByte(color23); shortSpiDelay;countx--;}//col7  
county--;}
}//end of part 2
void Adafruit_ST77xx::fillRectFast64colors3(uint16_t color24, uint16_t color25, uint16_t color26, uint16_t color27, uint16_t color28, uint16_t color29, uint16_t color30, uint16_t color31){
 uint16_t countx; uint16_t county;//we use these to understand what to draw!
// while(!(SPSR & (1<<SPIF) ));
county=hardcodeh;while (county >0 ){//row3
countx=hardcodew; while (countx >0) {complexspi = highByte(color24);longSpiDelay; complexspi =lowByte(color24); shortSpiDelay;countx--;}//col0
countx=hardcodew; while (countx >0) {complexspi = highByte(color25);longSpiDelay; complexspi =lowByte(color25); shortSpiDelay;countx--;}//col1
countx=hardcodew; while (countx >0) {complexspi = highByte(color26);longSpiDelay; complexspi =lowByte(color26); shortSpiDelay;countx--;}//col2
countx=hardcodew; while (countx >0) {complexspi = highByte(color27);longSpiDelay; complexspi =lowByte(color27); shortSpiDelay;countx--;}//col3
countx=hardcodew; while (countx >0) {complexspi = highByte(color28);longSpiDelay; complexspi =lowByte(color28); shortSpiDelay;countx--;}//col4
countx=hardcodew; while (countx >0) {complexspi = highByte(color29);longSpiDelay; complexspi =lowByte(color29); shortSpiDelay;countx--;}//col5
countx=hardcodew; while (countx >0) {complexspi = highByte(color30);longSpiDelay; complexspi =lowByte(color30); shortSpiDelay;countx--;}//col6                          
countx=hardcodew; while (countx >0) {complexspi = highByte(color31);longSpiDelay; complexspi =lowByte(color31); shortSpiDelay;countx--;}//col7  
county--;}
}//end of part 3
void Adafruit_ST77xx::fillRectFast64colors4(uint16_t color32, uint16_t color33, uint16_t color34, uint16_t color35, uint16_t color36, uint16_t color37, uint16_t color38, uint16_t color39){
  uint16_t countx; uint16_t county;//we use these to understand what to draw!
// while(!(SPSR & (1<<SPIF) ));
county=hardcodeh;while (county >0 ){//row4 
countx=hardcodew; while (countx >0) {complexspi = highByte(color32);longSpiDelay; complexspi =lowByte(color32); shortSpiDelay;countx--;}//col0
countx=hardcodew; while (countx >0) {complexspi = highByte(color33);longSpiDelay; complexspi =lowByte(color33); shortSpiDelay;countx--;}//col1
countx=hardcodew; while (countx >0) {complexspi = highByte(color34);longSpiDelay; complexspi =lowByte(color34); shortSpiDelay;countx--;}//col2
countx=hardcodew; while (countx >0) {complexspi = highByte(color35);longSpiDelay; complexspi =lowByte(color35); shortSpiDelay;countx--;}//col3
countx=hardcodew; while (countx >0) {complexspi = highByte(color36);longSpiDelay; complexspi =lowByte(color36); shortSpiDelay;countx--;}//col4
countx=hardcodew; while (countx >0) {complexspi = highByte(color37);longSpiDelay; complexspi =lowByte(color37); shortSpiDelay;countx--;}//col5
countx=hardcodew; while (countx >0) {complexspi = highByte(color38);longSpiDelay; complexspi =lowByte(color38); shortSpiDelay;countx--;}//col6                          
countx=hardcodew; while (countx >0) {complexspi = highByte(color39);longSpiDelay; complexspi =lowByte(color39); shortSpiDelay;countx--;}//col7  
county--;}
}//end of part 4
void Adafruit_ST77xx::fillRectFast64colors5(uint16_t color40, uint16_t color41, uint16_t color42, uint16_t color43, uint16_t color44, uint16_t color45, uint16_t color46, uint16_t color47){
  uint16_t countx; uint16_t county;//we use these to understand what to draw!
// while(!(SPSR & (1<<SPIF) ));
county=hardcodeh;while (county >0 ){//row5 
countx=hardcodew; while (countx >0) {complexspi = highByte(color40);longSpiDelay; complexspi =lowByte(color40); shortSpiDelay;countx--;}//col0
countx=hardcodew; while (countx >0) {complexspi = highByte(color41);longSpiDelay; complexspi =lowByte(color41); shortSpiDelay;countx--;}//col1
countx=hardcodew; while (countx >0) {complexspi = highByte(color42);longSpiDelay; complexspi =lowByte(color42); shortSpiDelay;countx--;}//col2
countx=hardcodew; while (countx >0) {complexspi = highByte(color43);longSpiDelay; complexspi =lowByte(color43); shortSpiDelay;countx--;}//col3
countx=hardcodew; while (countx >0) {complexspi = highByte(color44);longSpiDelay; complexspi =lowByte(color44); shortSpiDelay;countx--;}//col4
countx=hardcodew; while (countx >0) {complexspi = highByte(color45);longSpiDelay; complexspi =lowByte(color45); shortSpiDelay;countx--;}//col5
countx=hardcodew; while (countx >0) {complexspi = highByte(color46);longSpiDelay; complexspi =lowByte(color46); shortSpiDelay;countx--;}//col6                          
countx=hardcodew; while (countx >0) {complexspi = highByte(color47);longSpiDelay; complexspi =lowByte(color47); shortSpiDelay;countx--;}//col7  
county--;}
}//end of part 5
void Adafruit_ST77xx::fillRectFast64colors6(uint16_t color48, uint16_t color49, uint16_t color50, uint16_t color51, uint16_t color52, uint16_t color53, uint16_t color54, uint16_t color55){
  uint16_t countx; uint16_t county;//we use these to understand what to draw!
// while(!(SPSR & (1<<SPIF) ));
county=hardcodeh;while (county >0 ){//row6 
countx=hardcodew; while (countx >0) {complexspi = highByte(color48);longSpiDelay; complexspi =lowByte(color48); shortSpiDelay;countx--;}//col0
countx=hardcodew; while (countx >0) {complexspi = highByte(color49);longSpiDelay; complexspi =lowByte(color49); shortSpiDelay;countx--;}//col1
countx=hardcodew; while (countx >0) {complexspi = highByte(color50);longSpiDelay; complexspi =lowByte(color50); shortSpiDelay;countx--;}//col2
countx=hardcodew; while (countx >0) {complexspi = highByte(color51);longSpiDelay; complexspi =lowByte(color51); shortSpiDelay;countx--;}//col3
countx=hardcodew; while (countx >0) {complexspi = highByte(color52);longSpiDelay; complexspi =lowByte(color52); shortSpiDelay;countx--;}//col4
countx=hardcodew; while (countx >0) {complexspi = highByte(color53);longSpiDelay; complexspi =lowByte(color53); shortSpiDelay;countx--;}//col5
countx=hardcodew; while (countx >0) {complexspi = highByte(color54);longSpiDelay; complexspi =lowByte(color54); shortSpiDelay;countx--;}//col6                          
countx=hardcodew; while (countx >0) {complexspi = highByte(color55);longSpiDelay; complexspi =lowByte(color55); shortSpiDelay;countx--;}//col7  
county--;}
}//end of part 6
void Adafruit_ST77xx::fillRectFast64colors7(uint16_t color56, uint16_t color57, uint16_t color58, uint16_t color59, uint16_t color60, uint16_t color61, uint16_t color62, uint16_t color63){
  uint16_t countx; uint16_t county;//we use these to understand what to draw!
// while(!(SPSR & (1<<SPIF) ));
county=hardcodeh;while (county >0 ){//row7 
countx=hardcodew; while (countx >0) {complexspi = highByte(color56);longSpiDelay; complexspi =lowByte(color56); shortSpiDelay;countx--;}//col0
countx=hardcodew; while (countx >0) {complexspi = highByte(color57);longSpiDelay; complexspi =lowByte(color57); shortSpiDelay;countx--;}//col1
countx=hardcodew; while (countx >0) {complexspi = highByte(color58);longSpiDelay; complexspi =lowByte(color58); shortSpiDelay;countx--;}//col2
countx=hardcodew; while (countx >0) {complexspi = highByte(color59);longSpiDelay; complexspi =lowByte(color59); shortSpiDelay;countx--;}//col3
countx=hardcodew; while (countx >0) {complexspi = highByte(color60);longSpiDelay; complexspi =lowByte(color60); shortSpiDelay;countx--;}//col4
countx=hardcodew; while (countx >0) {complexspi = highByte(color61);longSpiDelay; complexspi =lowByte(color61); shortSpiDelay;countx--;}//col5
countx=hardcodew; while (countx >0) {complexspi = highByte(color62);longSpiDelay; complexspi =lowByte(color62); shortSpiDelay;countx--;}//col6                          
countx=hardcodew; while (countx >0) {complexspi = highByte(color63);longSpiDelay; complexspi =lowByte(color63); shortSpiDelay;countx--;}//col7  
county--;}
while(!(SPSR & (1<<SPIF) ));
    digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
 
  digitalWriteFast(TFT_CS,HIGH);//old CS_HIGH();
  SPI_END_TRANSACTION();  
}//end of part 7


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Adafruit_ST77xx::Color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}



void Adafruit_ST77xx::invertDisplay(boolean i) {
  writecommand(i ? ST77XX_INVON : ST77XX_INVOFF);
}


/******** low level bit twiddling **********/


inline void Adafruit_ST77xx::CS_HIGH(void) {
#if defined(USE_FAST_IO)
  *csport |= cspinmask;
#else
  digitalWriteFast(TFT_CS, HIGH);
#endif
}

inline void Adafruit_ST77xx::CS_LOW(void) {
#if defined(USE_FAST_IO)
  *csport &= ~cspinmask;
#else
 digitalWriteFast(TFT_CS,LOW);
#endif
}

inline void Adafruit_ST77xx::DC_HIGH(void) {
#if defined(USE_FAST_IO)
  *dcport |= dcpinmask;
#else
 digitalWriteFast(TFT_DC,HIGH)
#endif
}

inline void Adafruit_ST77xx::DC_LOW(void) {
#if defined(USE_FAST_IO)
  *dcport &= ~dcpinmask;
#else
 digitalWriteFast(TFT_DC,LOW)
#endif
}



////////// stuff not actively being used, but kept for posterity
/*

 uint8_t Adafruit_ST77xx::spiread(void) {
 uint8_t r = 0;
 if (_sid > 0) {
 r = shiftIn(_sid, _sclk, MSBFIRST);
 } else {
 //SID_DDR &= ~_BV(SID);
 //int8_t i;
 //for (i=7; i>=0; i--) {
 //  SCLK_PORT &= ~_BV(SCLK);
 //  r <<= 1;
 //  r |= (SID_PIN >> SID) & 0x1;
 //  SCLK_PORT |= _BV(SCLK);
 //}
 //SID_DDR |= _BV(SID);
 
 }
 return r;
 }
 
 
 void Adafruit_ST77xx::dummyclock(void) {
 
 if (_sid > 0) {
 digitalWrite(_sclk, LOW);
 digitalWrite(_sclk, HIGH);
 } else {
 // SCLK_PORT &= ~_BV(SCLK);
 //SCLK_PORT |= _BV(SCLK);
 }
 }
 uint8_t Adafruit_ST77xx::readdata(void) {
 *portOutputRegister(rsport) |= rspin;
 
 *portOutputRegister(csport) &= ~ cspin;
 
 uint8_t r = spiread();
 
 *portOutputRegister(csport) |= cspin;
 
 return r;
 
 } 
 
 uint8_t Adafruit_ST77xx::readcommand8(uint8_t c) {
 digitalWrite(_rs, LOW);
 
 *portOutputRegister(csport) &= ~ cspin;
 
 spiwrite(c);
 
 digitalWrite(_rs, HIGH);
 pinMode(_sid, INPUT); // input!
 digitalWrite(_sid, LOW); // low
 spiread();
 uint8_t r = spiread();
 
 
 *portOutputRegister(csport) |= cspin;
 
 
 pinMode(_sid, OUTPUT); // back to output
 return r;
 }
 
 
 uint16_t Adafruit_ST77xx::readcommand16(uint8_t c) {
 digitalWrite(_rs, LOW);
 if (_cs)
 digitalWrite(_cs, LOW);
 
 spiwrite(c);
 pinMode(_sid, INPUT); // input!
 uint16_t r = spiread();
 r <<= 8;
 r |= spiread();
 if (_cs)
 digitalWrite(_cs, HIGH);
 
 pinMode(_sid, OUTPUT); // back to output
 return r;
 }
 
 uint32_t Adafruit_ST77xx::readcommand32(uint8_t c) {
 digitalWrite(_rs, LOW);
 if (_cs)
 digitalWrite(_cs, LOW);
 spiwrite(c);
 pinMode(_sid, INPUT); // input!
 
 dummyclock();
 dummyclock();
 
 uint32_t r = spiread();
 r <<= 8;
 r |= spiread();
 r <<= 8;
 r |= spiread();
 r <<= 8;
 r |= spiread();
 if (_cs)
 digitalWrite(_cs, HIGH);
 
 pinMode(_sid, OUTPUT); // back to output
 return r;
 }
 
 */

#define SPI_END_TRANSACTION()      if (_hwSPI)    SPI.endTransaction()
#else
#define SPI_BEGIN_TRANSACTION()   // (void)
#define SPI_END_TRANSACTION()     // (void)
#endif

// Constructor when using software SPI.  All output pins are configurable.
Adafruit_ST77xx::Adafruit_ST77xx(int8_t cs, int8_t dc, int8_t sid, int8_t sclk, int8_t rst) 
  : Adafruit_GFX(ST7735_TFTWIDTH_128, ST7735_TFTHEIGHT_160)
{
  _cs   = cs;
  _dc   = dc;
  _sid  = sid;
  _sclk = sclk;
  _rst  = rst;
  _hwSPI = false;
}

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Adafruit_ST77xx::Adafruit_ST77xx(int8_t cs, int8_t dc, int8_t rst) 
  : Adafruit_GFX(ST7735_TFTWIDTH_128, ST7735_TFTHEIGHT_160) {
  _cs   = cs;
  _dc   = dc;
  _rst  = rst;
  _hwSPI = true;
  _sid  = _sclk = -1;
}

inline void Adafruit_ST77xx::spiwrite(uint8_t c) {

  //Serial.println(c, HEX);

  if (_hwSPI) {
#if defined (SPI_HAS_TRANSACTION)
      SPI.transfer(c);
#elif defined (__AVR__) || defined(CORE_TEENSY)
      //this is where transfer happens. try to move as much as possible here
      SPCRbackup = SPCR;
      SPCR = mySPCR;
      SPI.transfer(c);
      __asm__("nop\n\t"); //delayed for fast spi not to need another loop
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      //while (!(SPSR & _BV(SPIF)));
      SPCR = SPCRbackup;
#elif defined (__arm__)
      SPI.setClockDivider(21); //4MHz
      SPI.setDataMode(SPI_MODE0);
      SPI.transfer(c);
#endif
  } else {

    // Fast SPI bitbang swiped from LPD8806 library
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
#if defined(USE_FAST_IO)
      if(c & bit) *dataport |=  datapinmask;
      else        *dataport &= ~datapinmask;
      *clkport |=  clkpinmask;
      *clkport &= ~clkpinmask;
#else
      if(c & bit) digitalWrite(_sid, HIGH);
      else        digitalWrite(_sid, LOW);
      digitalWrite(_sclk, HIGH);
      digitalWrite(_sclk, LOW);
#endif
    }
  }
}


void Adafruit_ST77xx::writecommand(uint8_t c) {
digitalWriteFast(TFT_DC,LOW); //DC_LOW();
 digitalWriteFast(TFT_DC,LOW); //DC_LOW();
 digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  SPI_BEGIN_TRANSACTION();

  spiwrite(c);
 digitalWriteFast(TFT_CS,HIGH); //CS_HIGH();
 digitalWriteFast(TFT_CS,HIGH); //CS_HIGH();
  SPI_END_TRANSACTION();
}


void Adafruit_ST77xx::writedata(uint8_t c) {
  SPI_BEGIN_TRANSACTION();

   digitalWriteFast(TFT_DC,HIGH); //DC_LOW();
   digitalWriteFast(TFT_DC,HIGH); //DC_LOW();
 digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  digitalWriteFast(TFT_CS,LOW);// CS_LOW();
//  DC_HIGH();
 // CS_LOW();
    
  spiwrite(c);
 digitalWriteFast(TFT_CS,HIGH);
  digitalWriteFast(TFT_CS,HIGH);
 // CS_HIGH();
  SPI_END_TRANSACTION();
}


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Adafruit_ST77xx::displayInit(const uint8_t *addr) {

  uint8_t  numCommands, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++);   // Number of commands to follow
  while(numCommands--) {                 // For each command...
    writecommand(pgm_read_byte(addr++)); //   Read, issue command
    numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
    ms       = numArgs & ST_CMD_DELAY;   //   If hibit set, delay follows args
    numArgs &= ~ST_CMD_DELAY;            //   Mask out delay bit
    while(numArgs--) {                   //   For each argument...
      writedata(pgm_read_byte(addr++));  //     Read, issue argument
    }

    if(ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if(ms == 255) ms = 500;     // If 255, delay for 500 ms
      delay(ms);
    }
  }
}


// Initialization code common to all ST77XX displays
void Adafruit_ST77xx::commonInit(const uint8_t *cmdList) {
  _ystart = _xstart = 0;
  _colstart  = _rowstart = 0; // May be overridden in init func

  pinMode(_dc, OUTPUT);
  pinMode(_cs, OUTPUT);

#if defined(USE_FAST_IO)
  csport    = portOutputRegister(digitalPinToPort(_cs));
  dcport    = portOutputRegister(digitalPinToPort(_dc));
  cspinmask = digitalPinToBitMask(_cs);
  dcpinmask = digitalPinToBitMask(_dc);
#endif

  if(_hwSPI) { // Using hardware SPI
#if defined (SPI_HAS_TRANSACTION)
    SPI.begin();
    mySPISettings = SPISettings(24000000, MSBFIRST, SPI_MODE0);
#elif defined (__AVR__) || defined(CORE_TEENSY)
    SPCRbackup = SPCR;
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.setDataMode(SPI_MODE0);
    mySPCR = SPCR; // save our preferred state
    //Serial.print("mySPCR = 0x"); Serial.println(SPCR, HEX);
    SPCR = SPCRbackup;  // then restore
#elif defined (__SAM3X8E__)
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV2); //4MHz//8mhz
    SPI.setDataMode(SPI_MODE0);
#endif
  } else {
    pinMode(_sclk, OUTPUT);
    pinMode(_sid , OUTPUT);
    digitalWrite(_sclk, LOW);
    digitalWrite(_sid, LOW);

#if defined(USE_FAST_IO)
    clkport     = portOutputRegister(digitalPinToPort(_sclk));
    dataport    = portOutputRegister(digitalPinToPort(_sid));
    clkpinmask  = digitalPinToBitMask(_sclk);
    datapinmask = digitalPinToBitMask(_sid);
#endif
  }

  // toggle RST low to reset; CS low so it'll listen to us
  CS_LOW();
  if (_rst != -1) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(500);
    digitalWrite(_rst, LOW);
    delay(500);
    digitalWrite(_rst, HIGH);
    delay(500);
  }

  if(cmdList) 
    displayInit(cmdList);
}



void Adafruit_ST77xx::setRotation(uint8_t m) {

  writecommand(ST77XX_MADCTL);
  rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     writedata(ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB);

     _xstart = _colstart;
     _ystart = _rowstart;
     break;
   case 1:
     writedata(ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB);

     _ystart = _colstart;
     _xstart = _rowstart;
     break;
  case 2:
     writedata(ST77XX_MADCTL_RGB);
 
     _xstart = _colstart;
     _ystart = _rowstart;
     break;

   case 3:
     writedata(ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB);

     _ystart = _colstart;
     _xstart = _rowstart;
     break;
  }
}

void Adafruit_ST77xx::setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1,
 uint8_t y1) {

  uint16_t x_start = x0 + _xstart, x_end = x1 + _xstart;
  uint16_t y_start = y0 + _ystart, y_end = y1 + _ystart;
  

  writecommand(ST77XX_CASET); // Column addr set
  writedata(x_start >> 8);
  writedata(x_start & 0xFF);     // XSTART 
  writedata(x_end >> 8);
  writedata(x_end & 0xFF);     // XEND

  writecommand(ST77XX_RASET); // Row addr set
  writedata(y_start >> 8);
  writedata(y_start & 0xFF);     // YSTART
  writedata(y_end >> 8);
  writedata(y_end & 0xFF);     // YEND

  writecommand(ST77XX_RAMWR); // write to RAM
}


void Adafruit_ST77xx::pushColor(uint16_t color) {
  SPI_BEGIN_TRANSACTION();
   digitalWriteFast(TFT_CS,HIGH);
   digitalWriteFast(TFT_CS,HIGH);
  //DC_HIGH();
 // CS_LOW();
 digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  spiwrite(color >> 8);
  spiwrite(color);

  CS_HIGH();
  SPI_END_TRANSACTION();
}

void Adafruit_ST77xx::drawPixel(int16_t x, int16_t y, uint16_t color) {

  if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

  setAddrWindow(x,y,x+1,y+1);

  SPI_BEGIN_TRANSACTION();
  //DC_HIGH();
   digitalWriteFast(TFT_DC,HIGH); 
     digitalWriteFast(TFT_DC,HIGH);
 // CS_LOW();
 digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  digitalWriteFast(TFT_CS,LOW);// CS_LOW();
  spiwrite(color >> 8);
  spiwrite(color);
 digitalWriteFast(TFT_CS,HIGH);
  digitalWriteFast(TFT_CS,HIGH);
 // CS_HIGH();
  SPI_END_TRANSACTION();
}


void Adafruit_ST77xx::drawFastVLine(int16_t x, int16_t y, int16_t h,
 uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((y+h-1) >= _height) h = _height-y;
  setAddrWindow(x, y, x, y+h-1);

  uint8_t hi = color >> 8, lo = color;
    
  SPI_BEGIN_TRANSACTION();
  DC_HIGH();
  CS_LOW();

  while (h--) {
    spiwrite(hi);
    spiwrite(lo);
  }

  CS_HIGH();
  SPI_END_TRANSACTION();
}


void Adafruit_ST77xx::drawFastHLine(int16_t x, int16_t y, int16_t w,
  uint16_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((x+w-1) >= _width)  w = _width-x;
  setAddrWindow(x, y, x+w-1, y);

  uint8_t hi = color >> 8, lo = color;

  SPI_BEGIN_TRANSACTION();
  DC_HIGH();
  CS_LOW();

  while (w--) {
    spiwrite(hi);
    spiwrite(lo);
  }

  CS_HIGH();
  SPI_END_TRANSACTION();
}



void Adafruit_ST77xx::fillScreen(uint16_t color) {
  fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void Adafruit_ST77xx::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {

  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width)  w = _width  - x;
  if((y + h - 1) >= _height) h = _height - y;

  setAddrWindow(x, y, x+w-1, y+h-1);

  uint8_t hi = color >> 8, lo = color;
    
  SPI_BEGIN_TRANSACTION();

  DC_HIGH();
  CS_LOW();
  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      spiwrite(hi);
      spiwrite(lo);
    }
  }
  CS_HIGH();
  SPI_END_TRANSACTION();
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color

// fill a rectangle
void Adafruit_ST77xx::fillRectFast(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {
//this is needed for text but not fills with solid color!
  // rudimentary clipping (drawChar w/big text requires this)
 // if((x >= _width) || (y >= _height)) return;
 // if((x + w - 1) >= _width)  w = _width  - x;
 // if((y + h - 1) >= _height) h = _height - y;

  setAddrWindow(x, y, x+w-1, y+h-1);

  uint8_t hi = color >> 8, lo = color;
    
  SPI_BEGIN_TRANSACTION();

  digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
  
  digitalWriteFast(TFT_CS,LOW);// old CS_LOW();

//  y=h;
 
    x=w*h;// most of time is in pixel writes. a dealy is needed for spi to write data

    
   //   spiwrite(hi);//we update color
   //   spiwrite(lo);
//__attribute__((optimize("unroll-loops")));
    while ( x>0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
 //SPCR = SPCRbackup;  //we place at top for next loop iteration
 SPCRbackup = SPCR; //not sure what this does or if it is really needed, but we keep at begining of loop unroll
      SPCR = mySPCR;
       
      SPDR = hi;
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); //delayed for fast spi not to need another loop
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
      __asm__("nop\n\t"); 
    //  __asm__("nop\n\t"); 
     // __asm__("nop\n\t"); 
      //while (!(SPSR & _BV(SPIF)));
    //  SPCR = SPCRbackup;

    // SPCRbackup = SPCR;
      SPCR = mySPCR;
      // __asm__("nop\n\t"); 
      SPDR =lo;
      
     __asm__("nop\n\t"); 
      if (x==1){ return;}//shave time off of negating zero
          __asm__("nop\n\t"); 
           __asm__("nop\n\t"); 
           x--;
          
//we are useing the jmp and loop call times to reduce the timing needed here.
// since loop time is at the delay needed for spi, this is as efficient as it can be, as every clock
//per color pixel draw for fill is flooded to spi!

       
    }//x
    digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
 
  digitalWriteFast(TFT_CS,HIGH);//old CS_HIGH();
  SPI_END_TRANSACTION();
}

//below we have one that makes subsampling easier
void Adafruit_ST77xx::fillRectFast4colors(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color0,uint16_t color1,uint16_t color2,uint16_t color3) {
//this is needed for text but not fills with solid color!
  // rudimentary clipping (drawChar w/big text requires this)
 // if((x >= _width) || (y >= _height)) return;
 // if((x + w - 1) >= _width)  w = _width  - x;
 // if((y + h - 1) >= _height) h = _height - y;

  setAddrWindow(x, y, x+w-1, y+h-1);
  //uses more memory, but i think it is more efficient
  uint8_t hi0 = color0 >> 8, lo0 = color0;//we set this in advance, and we do the changes in between write cycles of spi! so no time lost!
  uint8_t hi1 = color1 >> 8, lo1 = color1;//we set this in advance, and we do the changes in between write cycles of spi! so no time lost!
  uint8_t hi2 = color2 >> 8, lo2 = color2;//we set this in advance, and we do the changes in between write cycles of spi! so no time lost!
  uint8_t hi3 = color3 >> 8, lo3 = color3;//we set this in advance, and we do the changes in between write cycles of spi! so no time lost!  
  SPI_BEGIN_TRANSACTION();
  //created complex commands in #define so code can be a lot cleaner
  //this is a complex command that complexspi= hi[0] value
#define complexspi SPCRbackup = SPCR; SPCR = mySPCR; SPDR 
//this is small delay
#define shortSpiDelay __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t") 
//last comment out ';' not needed here, but can be added in commands later 
#define longSpiDelay __asm__("nop\n\t"); __asm__("nop\n\t");__asm__("nop\n\t"); __asm__("nop\n\t");  __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t")  
  
digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
digitalWriteFast(TFT_CS,LOW);// old CS_LOW();

//  y=h;
  uint16_t countx;//we use these to understand what to draw!
  uint16_t county;

 SPCR = SPCRbackup;  //we place at top for next loop iteration
county=h/2;//+1;
while (county !=0 ){//first rectangle part
         countx=w/2;
          
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
    complexspi = hi0;longSpiDelay; complexspi =lo0; shortSpiDelay;   
           countx--;    
    }
       countx=w/2;
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.     
     complexspi = hi1;longSpiDelay;complexspi=lo1;shortSpiDelay;
           countx--;     
    }
    county--;
}//end of first half of rectange [0][1] drawn!


county=h/2;//+1; 
while (county !=0){
     countx=w/2;//we need to reset countx here as well
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
      complexspi = hi2;longSpiDelay;complexspi =lo2;shortSpiDelay;
      countx--;     
    }
 
    countx=w/2;
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
 complexspi = hi3;longSpiDelay;; complexspi =lo3;shortSpiDelay;         
           countx--;
    }
    county--;
}//end of first half of rectange [2][3] drawn!

         

    digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
 
  digitalWriteFast(TFT_CS,HIGH);//old CS_HIGH();
  SPI_END_TRANSACTION();
}


void Adafruit_ST77xx::fillRectFast16colors(int16_t x, int16_t y, int16_t w, int16_t h,
             uint16_t color0, uint16_t color1, uint16_t color2, uint16_t color3,
             uint16_t color4, uint16_t color5, uint16_t color6, uint16_t color7,
             uint16_t color8, uint16_t color9, uint16_t color10, uint16_t color11,
             uint16_t color12, uint16_t color13, uint16_t color14, uint16_t color15){
//this is where squares are written together!
  setAddrWindow(x, y, x+w-1, y+h-1);
  //uses more memory, but i think it is more efficient
  uint8_t hi0 = color0 >> 8, lo0 = color0 ;
  uint8_t hi1 = color1 >> 8, lo1 = color1 ;
  uint8_t hi2 = color2 >> 8, lo2 = color2 ;
  uint8_t hi3 = color3 >> 8, lo3 = color3 ;
  uint8_t hi4 = color4 >> 8, lo4 = color4 ;
  uint8_t hi5 = color5 >> 8, lo5 = color5 ;
  uint8_t hi6 = color6 >> 8, lo6 = color6 ;
  uint8_t hi7 = color7 >> 8, lo7 = color7 ;
  uint8_t hi8 = color8 >> 8, lo8 = color8 ;
  uint8_t hi9 = color9 >> 8, lo9 = color9 ; 
  uint8_t hi10= color10>> 8, lo10= color10; 
  uint8_t hi11= color11>> 8, lo11= color11;
  uint8_t hi12= color12>> 8, lo12= color12;
  uint8_t hi13= color13>> 8, lo13= color13;
  uint8_t hi14= color14>> 8, lo14= color14;
  uint8_t hi15= color15>> 8, lo15= color15;  
  SPI_BEGIN_TRANSACTION();
  //created complex commands in #define so code can be a lot cleaner
  //this is a complex command that complexspi= hi[0] value
#define complexspi SPCRbackup = SPCR; SPCR = mySPCR; SPDR 
//this is small delay
#define shortSpiDelay __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); 
//last comment out ';' not needed here, but can be added in commands later 
#define longSpiDelay __asm__("nop\n\t"); __asm__("nop\n\t");__asm__("nop\n\t"); __asm__("nop\n\t");  __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t");  
  //[0][1][2][3] 
  //[4][5][6][7] 
  //[8][9][A][B] //this is how command updates screen
  //[C][D][E][F] //a,b,c,d,e,f are 10,11,12,13,14,15
digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
digitalWriteFast(TFT_CS,LOW);// old CS_LOW();

//  y=h;
  uint16_t countx;//we use these to understand what to draw!
  uint16_t county;

 SPCR = SPCRbackup;  //we place at top for next loop iteration
county=h/4;//+1;
while (county !=0 ){//first rectangle part
         countx=w/4;
          
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
    complexspi = hi0;longSpiDelay; complexspi =lo0; shortSpiDelay;   
           countx--;    
                   }
       countx=w/4;
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.     
     complexspi = hi1;longSpiDelay;complexspi=lo1;shortSpiDelay;
           countx--;     
                    }
         countx=w/4;          
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
    complexspi = hi2;longSpiDelay; complexspi =lo2; shortSpiDelay;   
           countx--;    
                   }
       countx=w/4;
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.     
     complexspi = hi3;longSpiDelay;complexspi=lo3;shortSpiDelay;
           countx--;     
                  }
    
    county--;
};//                     [0][1][2][3] drawn!
county=h/4;//+1;
while (county !=0 ){//first rectangle part
         countx=w/4;
          
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
    complexspi = hi4;longSpiDelay; complexspi =lo4; shortSpiDelay;   
           countx--;    
                   }
       countx=w/4;
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.     
     complexspi = hi5;longSpiDelay;complexspi=lo5;shortSpiDelay;
           countx--;     
                    }
         countx=w/4;          
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
    complexspi = hi6;longSpiDelay; complexspi =lo6; shortSpiDelay;   
           countx--;    
                   }
       countx=w/4;
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.     
     complexspi = hi7;longSpiDelay;complexspi=lo7;shortSpiDelay;
           countx--;     
                  }
    
    county--;
};  //                        [4][5][6][7] drawn!
county=h/4;//+1;
while (county !=0 ){//first rectangle part
         countx=w/4;
          
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
    complexspi = hi8;longSpiDelay; complexspi =lo8; shortSpiDelay;   
           countx--;    
                   }
       countx=w/4;
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.     
     complexspi = hi9;longSpiDelay;complexspi=lo9;shortSpiDelay;
           countx--;     
                    }
         countx=w/4;          
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
    complexspi = hi10;longSpiDelay; complexspi =lo10; shortSpiDelay;   
           countx--;    
                   }
       countx=w/4;
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.     
     complexspi = hi11;longSpiDelay;complexspi=lo11;shortSpiDelay;
           countx--;     
                  }
    
    county--;
}; //                         [8][9][A][B] drawn!
county=h/4;//+1;
while (county !=0 ){//first rectangle part
         countx=w/4;
          
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
    complexspi = hi12;longSpiDelay; complexspi =lo12; shortSpiDelay;   
           countx--;    
                   }
       countx=w/4;
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.     
     complexspi = hi13;longSpiDelay;complexspi=lo13;shortSpiDelay;
           countx--;     
                    }
         countx=w/4;          
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.
    complexspi = hi14;longSpiDelay; complexspi =lo14; shortSpiDelay;   
           countx--;    
                   }
       countx=w/4;
    while (countx !=0) {//we need to unroll this loop and add a command that cycles thru to push address one byte at a time.     
     complexspi = hi15;longSpiDelay;complexspi=lo15;shortSpiDelay;
           countx--;     
                  }
    
    county--;
}; //                         [8][9][A][B] drawn!         

    digitalWriteFast(TFT_DC,HIGH);// old DC_HIGH();
 
  digitalWriteFast(TFT_CS,HIGH);//old CS_HIGH();
  SPI_END_TRANSACTION();             
             }

             
// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Adafruit_ST77xx::Color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}



void Adafruit_ST77xx::invertDisplay(boolean i) {
  writecommand(i ? ST77XX_INVON : ST77XX_INVOFF);
}


/******** low level bit twiddling **********/


inline void Adafruit_ST77xx::CS_HIGH(void) {
#if defined(USE_FAST_IO)
  *csport |= cspinmask;
#else
  digitalWriteFast(TFT_CS, HIGH);
#endif
}

inline void Adafruit_ST77xx::CS_LOW(void) {
#if defined(USE_FAST_IO)
  *csport &= ~cspinmask;
#else
 digitalWriteFast(TFT_CS,LOW);
#endif
}

inline void Adafruit_ST77xx::DC_HIGH(void) {
#if defined(USE_FAST_IO)
  *dcport |= dcpinmask;
#else
 digitalWriteFast(TFT_DC,HIGH)
#endif
}

inline void Adafruit_ST77xx::DC_LOW(void) {
#if defined(USE_FAST_IO)
  *dcport &= ~dcpinmask;
#else
 digitalWriteFast(TFT_DC,LOW)
#endif
}



////////// stuff not actively being used, but kept for posterity
/*

 uint8_t Adafruit_ST77xx::spiread(void) {
 uint8_t r = 0;
 if (_sid > 0) {
 r = shiftIn(_sid, _sclk, MSBFIRST);
 } else {
 //SID_DDR &= ~_BV(SID);
 //int8_t i;
 //for (i=7; i>=0; i--) {
 //  SCLK_PORT &= ~_BV(SCLK);
 //  r <<= 1;
 //  r |= (SID_PIN >> SID) & 0x1;
 //  SCLK_PORT |= _BV(SCLK);
 //}
 //SID_DDR |= _BV(SID);
 
 }
 return r;
 }
 
 
 void Adafruit_ST77xx::dummyclock(void) {
 
 if (_sid > 0) {
 digitalWrite(_sclk, LOW);
 digitalWrite(_sclk, HIGH);
 } else {
 // SCLK_PORT &= ~_BV(SCLK);
 //SCLK_PORT |= _BV(SCLK);
 }
 }
 uint8_t Adafruit_ST77xx::readdata(void) {
 *portOutputRegister(rsport) |= rspin;
 
 *portOutputRegister(csport) &= ~ cspin;
 
 uint8_t r = spiread();
 
 *portOutputRegister(csport) |= cspin;
 
 return r;
 
 } 
 
 uint8_t Adafruit_ST77xx::readcommand8(uint8_t c) {
 digitalWrite(_rs, LOW);
 
 *portOutputRegister(csport) &= ~ cspin;
 
 spiwrite(c);
 
 digitalWrite(_rs, HIGH);
 pinMode(_sid, INPUT); // input!
 digitalWrite(_sid, LOW); // low
 spiread();
 uint8_t r = spiread();
 
 
 *portOutputRegister(csport) |= cspin;
 
 
 pinMode(_sid, OUTPUT); // back to output
 return r;
 }
 
 
 uint16_t Adafruit_ST77xx::readcommand16(uint8_t c) {
 digitalWrite(_rs, LOW);
 if (_cs)
 digitalWrite(_cs, LOW);
 
 spiwrite(c);
 pinMode(_sid, INPUT); // input!
 uint16_t r = spiread();
 r <<= 8;
 r |= spiread();
 if (_cs)
 digitalWrite(_cs, HIGH);
 
 pinMode(_sid, OUTPUT); // back to output
 return r;
 }
 
 uint32_t Adafruit_ST77xx::readcommand32(uint8_t c) {
 digitalWrite(_rs, LOW);
 if (_cs)
 digitalWrite(_cs, LOW);
 spiwrite(c);
 pinMode(_sid, INPUT); // input!
 
 dummyclock();
 dummyclock();
 
 uint32_t r = spiread();
 r <<= 8;
 r |= spiread();
 r <<= 8;
 r |= spiread();
 r <<= 8;
 r |= spiread();
 if (_cs)
 digitalWrite(_cs, HIGH);
 
 pinMode(_sid, OUTPUT); // back to output
 return r;
 }
 
 */
