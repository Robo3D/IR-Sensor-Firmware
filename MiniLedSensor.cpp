/*
 * MiniLedSensor.cpp
 *
 * Created: 03/05/2015 11:06:12
 *  Author: David Crocker, Escher Technologies Ltd.
 * Licensed under the GNU General Public License version 3. See http://www.gnu.org/licenses/gpl-3.0.en.html.
 * This software is supplied WITHOUT WARRANTY except when it is supplied pre-programmed into
 * an electronic device that was manufactured by or for Escher Technologies Limited.
 */ 

// Version 3: changed modulation scheme to allow for charging/discharging of phototransistor base-collector capacitance
// Version 4: increased maximum value of the pullup resistor we look for to 150K, because it is higher on the Arduino Due
// Version 5: increased maximum value of the pullup resistor we look for to 160K, to get reliable results with the 150K resistor in the test rig
// Version 6: Don't enable pullup resistor on phototransistor input
// Version 7: By Ivan Volosyuk: rewrite without interrupts and with ADC noise reduction

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

#define ISR_DEBUG	(0)		// set nonzero to use PB2 as debug output pin

#define BITVAL(_x) static_cast<uint8_t>(1u << (_x))

// Pin assignments:
// PB0/MOSI			far LED drive, active high
// PB1/MISO			near LED drive, active high
// PB2/ADC1/SCK	        	output to Duet via 12K resistor
// PB3/ADC3			output to Duet via 10K resistor
// PB4/ADC2			input from phototransistor
// PB5/ADC0/RESET       	not available, used for programming

__fuse_t __fuse __attribute__((section (".fuse"))) = {0xE2u, 0xDFu, 0xFFu};

const unsigned int AdcPhototransistorChan = 2;				// ADC channel for the phototransistor
const unsigned int AdcPortBDuet10KOutputChan = 3;			// ADC channel for the 10K output bit, when we use it as an input
const unsigned int PortBNearLedBit = 1;
const unsigned int PortBFarLedBit = 0;
const unsigned int PortBDuet10KOutputBit = 3;
const unsigned int PortBDuet12KOutputBit = 2;
const uint8_t OutputOn = BITVAL(PortBDuet10KOutputBit) | BITVAL(PortBDuet12KOutputBit);
const uint8_t PortBUnusedBitMask = 0;

ISR(ADC_vect) { // ADC interrupt handler
}

void startADC() {
  ADCSRA = BITVAL(ADEN) | BITVAL(ADSC) | BITVAL(ADIE); // enable ADC, start conversion
  sleep_enable();
  do {
    sei();
    sleep_cpu();
    cli();
  } while(ADCSRA & BITVAL(ADSC));
  sleep_disable();
}

// Run the IR sensor and the fan
void runIRsensor() {
    cli();
    TIFR = BITVAL(OCF0B); // clear any pending interrupt

    ADMUX = (uint8_t)AdcPortBDuet10KOutputChan; // select the 10K resistor output bit, single ended mode
    ADCSRA = BITVAL(ADEN) | BITVAL(ADIE);	// enable ADC
    ADCSRB = 0; // ADC manual invocation
    set_sleep_mode( SLEEP_MODE_ADC );
    sleep_enable();

    // Change back to normal operation mode
    ADMUX = (uint8_t)AdcPhototransistorChan; // select input 1 = phototransistor, single ended mode
    DDRB |= BITVAL(PortBDuet10KOutputBit);  // set the pin back to being an output

    uint8_t output = BITVAL(PortBDuet12KOutputBit) | BITVAL(PortBDuet10KOutputBit); // Full output via both resistors
    PORTB = output;
    sei();

    // Start normal operation
    for (;;) {
      startADC();
      uint16_t offVal = ADC;       // get the ADC reading from the previous conversion
      PORTB = output | BITVAL(PortBNearLedBit);     // turn near LED on

      startADC(); // waste some time, initial value after switch doesn't look stable
      startADC();
      uint16_t nearVal = ADC;       // get the ADC reading from the previous conversion
      PORTB = output | BITVAL(PortBFarLedBit);     // turn far LED on
      startADC(); // waste some time, initial value after switch doesn't look stable
      startADC();
      uint16_t farVal = ADC;       // get the ADC reading from the previous conversion
      PORTB = output;                       // turn of LEDs

      uint16_t nearClean = (nearVal > offVal) ? nearVal - offVal : 0;
      uint16_t farClean = (farVal > offVal) ? farVal - offVal : 0;

      // Differential modulated IR sensor mode								
      if (nearVal > farVal) {
        output = OutputOn;
      } else {
        output = 0;
      }
      PORTB = output;
    }
  }


// Main program
int main(void) {
  cli();
  DIDR0 = BITVAL(AdcPhototransistorChan); // disable digital input buffers on ADC pins

  // Set ports and pullup resistors
  PORTB = PortBUnusedBitMask; // enable pullup on unused I/O pins

  // Enable outputs
  DDRB = BITVAL(PortBNearLedBit) | BITVAL(PortBFarLedBit) | BITVAL(PortBDuet10KOutputBit) | BITVAL(PortBDuet12KOutputBit);

  sei();

  runIRsensor(); // doesn't return
  return 0; // to keep gcc happy
}

// End
