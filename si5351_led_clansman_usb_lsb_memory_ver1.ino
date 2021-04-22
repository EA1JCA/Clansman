// Fvfo: 3.35..to 31.75.....
// Frf:  1.6 to 30..........
//
// used Arduino board: e.g. "Nano" Uno or similar
// adapted for the Clansman PRC320 by EA1JCA , clk 1 is cio 1.75 usb or 1.746.8 lsb. Clk0 is vfo+if22
// used Si5351A breakout board ( Fx=25MHz ):
// http://www.qrp-labs.com/synth.html   
//
// original software code made by OE1CGS, extended by DJ7OO
// http://www.qrp-labs.com/synth/oe1cgs.html 
//
//  
// Si5351A I2C SDA: #A4
//             SCL: #A5 
// 2khz tone on pin 5......connect via 10nf to a 5k preset and adjust for 2.5 volt peak to peak output 
// mode_sw lsb or usb selection     by connecting #9 to ground
// STEP selection                   by connecting #4 to ground 
// EEPROM FRQ storing               by connecting #6 to ground 
#include <si5351.h>
#include <Wire.h>
// Class instantiation
Si5351 si5351;

#include "EEPROM.h" 
#include <LedPrint.h>
 LedPrintJustifiable myLed = LedPrintJustifiable              // 8 digit LED display Module
      (
        11, // DATA PIN
        13, // CLOCK PIN
        10, // CS PIN
        8,  // NUMBER OF DIGITS
        1   // Orientation 0/1, if it looks backwards try the other
      );

 float frq; 
 float frq_old;
 float step = 1000;
 float step_old; 
 unsigned long currentTime;
 unsigned long loopTime;
 const int pin_A = 2;  // pin 2
 const int pin_B = 3;  // pin 3
 const unsigned long iffreq_usb = 1750000; //.............................................  10965000 for cb 1750000 for clansman usb set the IF frequency in Hz.
 const unsigned long iffreq_lsb = 1746800; //...............................................10962000 for cb 1746800 for clansman lsb     
 unsigned char encoder_A;
 unsigned char encoder_B;
 unsigned char encoder_A_prev=0;
 int press = 0;
 int cnt = 1;
 int cwtone = 5;
 boolean toggle = true;
 int mode_State = 0;
 const int mode_sw = 9;    // mode switch const int mode_sw = 9; 

/////////////////////////////////////////////////////////////////////////

void rotary_enc()
{
   currentTime = millis();
   if(currentTime >= (loopTime + 2)){
    encoder_A = digitalRead(pin_A);    
    encoder_B = digitalRead(pin_B);   
    if((!encoder_A) && (encoder_A_prev)){
    if(encoder_B) {
        frq = frq+step;               
        if(frq>31750000) {frq=3350000;}                          //vfo limits .3.35..to 31.75 clansman
      }   
      else {
        frq = frq-step;               
        if(frq<3350000) {frq=31750000;}                           // vfo limits . 31.75 to 3.35 clansman
      }   
    }   
    encoder_A_prev = encoder_A;     // Store value of A for next time       
    loopTime = currentTime;         // Updates loopTime
    }
  }

//////////////////////////////////////////////////////////////
void EEPROMWritelong(int address,long value)
      {
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
      }

//////////////////////////////////////////////////////////////
//This function will return a 4 byte (32bit) long from the eeprom
//at the specified address to address + 3.
long EEPROMReadlong(long address)
      {
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
      }

////////////////////////////////////////////////////////////  


///////////////////////////////////////////////////////////////
void setup() 
 {
  // Set Brightness
      myLed.setIntensity(0);
      
      // Center justify (0), alternatives -1 for left and 1 for right
      myLed.justify(0);
  
 Serial.begin(9600);
 tone(cwtone, 2000);      
 // Initialize the display
myLed.println("EA1JCA");               // add your call here 
delay(3000);

// Set GPIO

 pinMode(4,INPUT  );                   // no pullups on current arduinopinMode(4,INPUT_PULLUP);
 pinMode(mode_sw, INPUT);
 pinMode(6,INPUT);      // pinMode(9,INPUT_PULLUP);  
 pinMode(pin_A, INPUT);
 pinMode(pin_B, INPUT);

// Turn on pullup resistors
 digitalWrite(pin_A, HIGH);
 digitalWrite(pin_B, HIGH);
 digitalWrite(4, HIGH);                  //stepbutton
 digitalWrite(mode_sw, HIGH);           // mode switch lsb usb
 digitalWrite(6, HIGH);                 //  memory

 
 currentTime = millis();
 loopTime = currentTime; 
 frq = EEPROMReadlong(0); 
 //Serial.print("Frf read from EEPROM; ");
 //Serial.print(frq/1000,1);
 //Serial.println(" KHz"); 
 if(frq<3350000||frq>31750000) {frq=15750000;}                          // original if(frq<4700000||frq>5100000) {frq=4850000;} 31750000) {frq=3350000;

// Wire.begin();                         // Initialize I2C-communication as master
                                       // SDA on pin ADC04
                                       // SCL on pin ADC05
//  SetFrequency(frq);                   // Set TX-Frequency 

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 120800);                              // run config routine to find the correction factor

  si5351.drive_strength(SI5351_CLK0,SI5351_DRIVE_8MA);                          //  power adjust normally 8ma , 2ma , 4ma or 6ma avalible

  si5351.drive_strength(SI5351_CLK2,SI5351_DRIVE_8MA);                          //  power adjust normally 8ma , 2ma , 4ma or 6ma avalible
}

void loop() 
{
  myLed.justify(-1); 
  myLed.println((frq/1000)-1750,2);
  press = digitalRead(4);
  if (press == LOW)
  {
  cnt = cnt+1; 
  if(cnt==4 ) { cnt=0 ; }
//  Serial.println(cnt);
  if(cnt==0) { step=100; }                //100hz steps         adjust to suit your stepping preferences
  else
  if(cnt==1)  { step=500; }               //500hz steps
  else
  if(cnt==2) { step=10000; }              //10khz steps
 else
  if(cnt==3) { step=1000000; }             //1mhz steps      
  
  delay(300);

  
  }

  press = digitalRead(6);
  if (press == LOW)
  {
  long address=0;
  EEPROMWritelong(address, frq); 
  address+=4;
  
  delay(300);
  }
  
  rotary_enc();
  if(frq!=frq_old)
  mode_State = digitalRead(mode_sw);
  
 // check if the mode sw has changed. If it has, the mode State is HIGH:
  if (mode_State == HIGH)  {
// GO lsb
            si5351.set_freq((frq + iffreq_lsb) * 100ULL, SI5351_CLK0);//.............................vfo control
            si5351.set_freq(iffreq_lsb * 100ULL, SI5351_CLK2);//.....................................cio control
           
} else {

           si5351.set_freq((frq + iffreq_usb) * 100ULL, SI5351_CLK0);//..........................vfo control
           si5351.set_freq(iffreq_usb * 100ULL, SI5351_CLK2);//..................................cio control


 }  }


 
 
 
 
 
