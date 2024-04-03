
    /////////////////////////////
   //  PS/2-mouse to MSX v1.0 //
  //  Made By: NYYRIKKI 2013 //
 ////////////////////////////

//added changes for Amigeast PCB and Arduino Nano

//Changed config of PS/2 pins to (3,2)
//cut db9 trace 5 and connect to 5v
//cut db9 around pin 8 and connect to pin 6 arduino
//cut db9 around pin 9 and connect to gnd arduino
//connect db9 pin 7 to pin 7 arduino

/*
   Usage:
   Press reset = Mouse mode
   Press reset & hold right mouseb                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                qutton down = Joystick emulation
   Press reset & hold left mousebutton down = Mouse + Joystick emulation (*)
   
   If you have mouse with a scrolling wheel (optional) you can use the
   wheel to adjust speed of mouse / sensitivity of joystick.
   
   If status LED does NOT blink check your PS2-mouse connection and press reset.
   
   (*) Please note: This mode might cause problems with some of the existing software.
//////////////////////////////////////////////////////////////////////////////////*/

  // As of Arduino 1.0 ps2.zip should be unpacked into arduino/libraries.
  // In ps2.h replace "WProgram.h" with "Arduino.h".
 
  #include "ps2.h"

  /////////////////////////////////
 // HW Connection instructions: //
/////////////////////////////////

// PS/2 connector: (Female)   USB-connetor: (Female)
//
//           1 2                    ---------
//          3   4                  | 4 3 2 1 |
//           5|6                    -=======-
//
//        Mini-DIN-6F                 USB-AF
//
//            (Both looking from outside)
//
// Connecting PS2: 
// pin 1 -> DATA
// pin 3 -> GND
// pin 4 -> +5V
// pin 5 -> CLK

// Define Arduino pins used in format: CLK , DATA

  PS2 mouse(3, 2);  
  
// If you use USB-connector, connect:
// pin 1 -> +5V
// pin 2 -> DATA
// pin 3 -> CLK 
// pin 4 -> GND
//
// Usually USB-mice that are produced before 2010 support PS/2 protocol out of the box.
//
// Connecting Joystick:
//
//  MSX-connector : Arduino pin
//     (DB9F)
  #define JoyPin1 12
  #define JoyPin2 11
  #define JoyPin3 10
  #define JoyPin4 9
  //      JoyPin5 +5V (N/C or power)
  #define JoyPin6 8
  #define JoyPin7 7
  #define JoyPin8 6//8
  //      JoyPin9 GND (Connect to Arduino GND)
  
  #define InfoPin 13 // Status LED

///////////////////////////////////////////////////////////////////////////////

// Variables
  char rx=0,ry=0,rz=0,x=0,y=0,mx=0,my=0,jx=0,jy=0,scale=15,MouseID=0,EmuMode=0;
  char olimit=60, ilimit=50, solimit=0, silimit=0, mstat=0, StatCnt=0;
  long time; // No C

void mouse_init()
{
  mouse.write(0xff);   // reset
  mouse.read();        // ack byte
  mouse.read();        // Self-test passed
  mouse.read();        // Mouse ID
  mouse.write(0xf0);   // remote mode
  mouse.read();        // ack
  
  mouse.write(0xeb);   // give me data!
  mouse.read();        // ignore ack
 
  EmuMode = mouse.read() & 3; // Read Emulation mode from buttons
  if (EmuMode==3) EmuMode=0;
 
  mouse.read();        // ignore X
  mouse.read();        // ignore Y
  
  mouse.write(0xf3);   // Set sample rate
  mouse.read();        // ack
  mouse.write(200);    // Max sample rate
  mouse.read();        // ack  
  
  mouse.write(0xf3);   // Set sample rate
  mouse.read();        // ack
  mouse.write(100);
  mouse.read();        // ack  
  
  mouse.write(0xf3);   // Set sample rate
  mouse.read();        // ack
  mouse.write(80);
  mouse.read();        // ack  
  
  mouse.write(0xf2);   // Get device ID
  mouse.read();        // ack
  MouseID = mouse.read();   // Mouse ID
  
  mouse.write(0xf3);   // Set sample rate
  mouse.read();        // ack
  mouse.write(200);    // Max sample rate
  mouse.read();        // ack  
  
  mouse.write(0xe8);   // Set Resolution
  mouse.read();        // ack
  mouse.write(2);      // 4 counts/mm
  mouse.read();        // ack  
  
  delayMicroseconds(100);
}

void setup()
{  
  digitalWrite(JoyPin1,LOW);
  digitalWrite(JoyPin2,LOW);
  digitalWrite(JoyPin3,LOW);
  digitalWrite(JoyPin4,LOW);
  digitalWrite(JoyPin6,LOW);
  digitalWrite(JoyPin7,LOW);

  digitalWrite(InfoPin,LOW);
  pinMode(InfoPin,OUTPUT);
  
  Serial.begin(9600);
  Serial.print("\n\nPS/2-mouse to MSX-mouse converter for Arduino\n");
  Serial.print("---------------------------------------------\n");
  Serial.print("Made By: NYYRIKKI 2013\n\n");
  Serial.print("Initializing PS/2 mouse...");
  mouse_init();
  Serial.print("Ok\n\nI'm a ");
  switch (EmuMode) {
    case 0: Serial.print ("mouse!");
    break;
    case 1: Serial.print ("mouse & joystick hybrid!");
    break;
    case 2: Serial.print("joystick!");
    break;
  }
  Serial.print("\n");
  pinMode(1, INPUT_PULLUP);
}

int buttonScale=0;
void loop()
{
  StatCnt++;
  if (StatCnt==50) StatCnt=0;
  
  mouse.write(0xeb);    // give me data!
  mouse.read();         // ignore ack
  mstat = mouse.read(); // Read buttons

  rx = mouse.read();
  ry = mouse.read();  

  if (MouseID==3) {
    rz=mouse.read();
    scale=scale-rz;
    if (scale<5) scale=5;
    if (scale>25) scale=25;
  }

  if(digitalRead(1)==HIGH)
  {
    if(buttonScale==1)
    {
      scale=scale-5;
      if (scale<5) scale=25;
    }
    buttonScale=buttonScale=1;
  }
  else
  {
    buttonScale=0;
  }
 
  if(mstat&1) pinMode(JoyPin6,OUTPUT); else pinMode(JoyPin6,INPUT);
  if(mstat&2) pinMode(JoyPin7,OUTPUT); else pinMode(JoyPin7,INPUT);
  
  if (EmuMode==0 | EmuMode==1) {  // Mouse emulation 
  
    mx = mx-rx;
    my = my+ry;
    x=mx*(scale)/20;
    y=my*(scale)/20;

    time=millis()+40;
    
    sendMSX(x);  //-X
    sendMSX(y);  //-Y
    if (millis()<time) {
      mx=0;my=0;
      if (StatCnt<25) digitalWrite(InfoPin,LOW); else digitalWrite(InfoPin,HIGH);
      if (EmuMode==1) {
        sendMSX(0);
        sendMSX(0);
      }
      time=millis()+2;
    } else if (StatCnt<48) digitalWrite(InfoPin,LOW); else digitalWrite(InfoPin,HIGH);
    
    while (digitalRead(JoyPin8)==LOW) {if (millis()>time) break;};
    JoyHigh(); 
 }

  if (EmuMode==2 | EmuMode==1){   // Joystick emulation

    jy=jy+ry;
    jx=jx-rx;
    if (StatCnt<40) digitalWrite(InfoPin,LOW); else digitalWrite(InfoPin,HIGH);
    if (digitalRead(JoyPin8)==LOW) {
      solimit=olimit*(scale)/20;
      silimit=ilimit*(scale)/20;

      if (jy>solimit) jy=solimit;
      if (jy<-solimit) jy=-solimit;
      if (jx>solimit) jx=solimit;
      if (jx<-solimit) jx=-solimit;
  
      if (jy>silimit)  pinMode(JoyPin1,OUTPUT); else pinMode(JoyPin1,INPUT);
      if (jy<-silimit) pinMode(JoyPin2,OUTPUT); else pinMode(JoyPin2,INPUT);
      if (jx>silimit)  pinMode(JoyPin3,OUTPUT); else pinMode(JoyPin3,INPUT);
      if (jx<-silimit) pinMode(JoyPin4,OUTPUT); else pinMode(JoyPin4,INPUT);
    } else JoyHigh();
  }
}

void sendMSX(char c)
{
    while (digitalRead(JoyPin8)==LOW) {if (millis()>time) return;};
    if(c&128) pinMode(JoyPin4,INPUT); else pinMode(JoyPin4,OUTPUT);
    if(c&64)  pinMode(JoyPin3,INPUT); else pinMode(JoyPin3,OUTPUT);
    if(c&32)  pinMode(JoyPin2,INPUT); else pinMode(JoyPin2,OUTPUT);
    if(c&16)  pinMode(JoyPin1,INPUT); else pinMode(JoyPin1,OUTPUT);
    while (digitalRead(JoyPin8)==HIGH) {if (millis()>time) return;};
    if(c&8)   pinMode(JoyPin4,INPUT); else pinMode(JoyPin4,OUTPUT);
    if(c&4)   pinMode(JoyPin3,INPUT); else pinMode(JoyPin3,OUTPUT);
    if(c&2)   pinMode(JoyPin2,INPUT); else pinMode(JoyPin2,OUTPUT);
    if(c&1)   pinMode(JoyPin1,INPUT); else pinMode(JoyPin1,OUTPUT);
}
      
void JoyHigh()
{
    pinMode(JoyPin1,INPUT);
    pinMode(JoyPin2,INPUT);
    pinMode(JoyPin3,INPUT);
    pinMode(JoyPin4,INPUT);
}
