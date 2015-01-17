/*
AndyPi Spectrum Analyser for Arduino Nano
http://andypi.co.uk
Date:      27-09-2014
Design:    AndyPi
Verision:  1.0

Code based on:
OpenMusicLabs.com FHT example; Adafruit WS2801 Library & example
NOTE: FastLED library does NOT work due to memory conflicts

Hardware:
Arduino Nano
An IR LED must be connected to Arduino PWM pin 3.
Adafruit electret mic: A0 pin
*/


#include "SPI.h" // Comment out this line if using Trinket or Gemma
#include <IRremote.h>

#define OCTAVE 1 //   // Group buckets into octaves  (use the log output function LOG_OUT 1)
#define OCT_NORM 0 // Don't normalise octave intensities by number of bins
#define FHT_N 256 // set to 256 point fht
#include <FHT.h> // include the library


/*
F720DF - red
F7A05F - green
F7609F - blue
F7E01F - white
F7D02F - flash
F7F00F - strobe
F7C837 - fade
F7E817 - smooth
F728D7 -elloy
F76897 - pink
F7C03F - on
F740BF - off


*/

IRsend irsend;

//IRrecv irrecv(0);

int noise[]={204,188,68,73,150,120,88,68}; // noise level determined by playing pink noise and seeing levels [trial and error]{204,188,68,73,150,98,88,68}


void setup() {
  Serial.begin(115200); // use the serial port
//  strip.begin();  // initialise WS2801 strip
//  strip.show(); // Update LED contents, to start they are all 'off'
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
}

void loop() {

  // Start of Fourier Transform code; takes input on ADC from mic
  
  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fht_input[i] = k; // put real data into bins
    }
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_octave(); // take the output of the fht  fht_mag_log()
    sei();

    // End of Fourier Transform code - output is stored in fht_oct_out[i]. 

    // i=0-7 frequency (octave) bins (don't use 0 or 1), fht_oct_out[1]= amplitude of frequency for bin 1
    // for loop a) removes background noise average and takes absolute value b) low / high pass filter as still very noisy
    // c) maps amplitude of octave to a colour between blue and red d) sets pixel colour to amplitude of each frequency (octave)
    
     int fht_noise_adjusted[8];
     int ColourSpectrum[8];
     for (int i = 4; i < 5; i++) {  // For each of the 6 useful octave bins
       fht_noise_adjusted[i] = abs(fht_oct_out[i]-noise[i]);  // take the pink noise average level out, take the asbolute value to avoid negative numbers
       fht_noise_adjusted[i] = constrain(fht_noise_adjusted[i], 37, 125); // 37 lowpass for noise; 125 high pass doesn't go much higher than this [found by trial and error]
       ColourSpectrum[i] = map(fht_noise_adjusted[i], 37, 125, 160, 0); // map to values 0 - 160, i.e. blue to red on colour spectrum - larger range gives more colour variability [found by trial and error]
       Serial.println(ColourSpectrum[i]);
       convert (ColourSpectrum[i]);
      
       //test("NEC1", NEC, color, 32);
      // strip.setPixelColor((i-2), Wheel(ColourSpectrum[i]));  // set each pixels colour to the amplitude of that particular octave
     }  
     
 // strip.show(); // update the LEDs

  }
}








void test(char *label, int type, unsigned long value, int bits) {
  
   // Serial.println(label);
    if (type == NEC) {
      irsend.sendNEC(value, bits);
    } 
  
}

void convert (int con)
{
  /*
F720DF - red
F7A05F - green
F7609F - blue
F7E01F - white
F7D02F - flash
F7F00F - strobe
F7C837 - fade
F7E817 - smooth
F728D7 -elloy
F76897 - pink
F7C03F - on
F740BF - off


*/
if ( con < 90) {
   Serial.print("Nec:");
   Serial.println(con);
test("NEC1", NEC, 0xF740BF, 32);
}
if (con < 179 && con >= 150)
{
    Serial.print("Nec:");
   Serial.println(con);
test("NEC1", NEC, 0xF7C03F, 32);
test("NEC1", NEC, 0xF720DF, 32);
//test("NEC1", NEC, 0xF740BF, 32);
}
 if (con <= 149 && con >= 140){
   Serial.print("Nec:");
   Serial.println(con);
   test("NEC1", NEC, 0xF7C03F, 32);
   test("NEC1", NEC, 0xF7A05F, 32);
 //  test("NEC1", NEC, 0xF740BF, 32);
 }
 if (con <= 139 && con >= 130)
 {
 Serial.print("Nec:");
   Serial.println(con);
   test("NEC1", NEC, 0xF7C03F, 32);
 test("NEC1", NEC, 0xF7609F, 32);
 //test("NEC1", NEC, 0xF740BF, 32);
 }
 if (con <= 129 && con >= 120)
 {
 Serial.print("Nec:");
   Serial.println(con);
   test("NEC1", NEC, 0xF7C03F, 32);
 test("NEC1", NEC, 0xF728D7, 32);
// test("NEC1", NEC, 0xF740BF, 32);
 }
 if (con <= 119 && con >= 110)
 {
 Serial.print("Nec:");
   Serial.println(con);
   test("NEC1", NEC, 0xF7C03F, 32);
 test("NEC1", NEC, 0xF7A05F, 32);
// test("NEC1", NEC, 0xF740BF, 32);
 }
 if (con <= 109 && con >= 100)
 {
   Serial.print("Nec:");
   Serial.println(con);
   test("NEC1", NEC, 0xF7C03F, 32);
 test("NEC1", NEC, 0xF76897, 32);
// test("NEC1", NEC, 0xF740BF, 32);
 }
 if (con <= 99 && con >= 91)
 {
   Serial.print("Nec:");
   Serial.println(con);
   test("NEC1", NEC, 0xF7C03F, 32);
 test("NEC1", NEC, 0xF720DF, 32);
// test("NEC1", NEC, 0xF740BF, 32);
 }
}
