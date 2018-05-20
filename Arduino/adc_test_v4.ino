/******************************************************************************

  Arduino Second Order Delta-Sigma Modulator

  github.com/monisc/DC-Modulator - April 2017


  Reference: ATmega328P Datasheet -> at328

  Useful macros defined by the AVR libraries

  _BV(bit) = (1 << (bit))

  sfr |= _BV(bit) <== Set bit number indicated by "bit" in register "sfr"
  sfr &= ~(_BV(bit)) <== Clear bit number indicated by "bit" in register "sfr"

******************************************************************************/

/*** Global variable declarations ***/
/* Input and output pins */
const int out_pin = 6;

int i; /* To use in the loop */

/* Input and output values */
double u = 0;
double v = 0;

float x[2] = {0, 0};
float xn[2] = {0, 0};

/* Volatile variable to save the sample from the ADC */
volatile uint8_t ADC_val;


void setup() {
  /*** Serial communication for debugging purposes ***/
  Serial.begin(9600);

  /*** Setup the Arduino output pin to use ***/
  /* pinMode(10, OUTPUT); */
  /* start_time = micros(); */

  /*** Disable all interrupts on configuration ***/
  noInterrupts();

  /*** We will use the Timer2 in the ATmega328P ***/
  TCCR2A = 0; /* Set the entire TCRR2A register to 0, normal mode */
  TCCR2B = 0; /* Set the entire TCRR2B register to 0, normal mode */

  TCCR2A |= _BV(WGM21); /* Set CTC mode (at328 table 22-9) */
  TCCR2B |= _BV(CS21);  /* Timer2 Clock Prescaler to 8 (at328 table 22-10) */

  TCNT2 = 0; /* Initialize counter */

  /* To calculate the value to compare to: 16MHz/8 = 2MHz ==> 2MHz/X = ~22kHz ==> X = 90 */
  OCR2A = 90; /* Compare Match register set to 90 (which gives a timer frequency of 22.191kHz) */

  /*** ADC Setup ***/
  ADMUX = 0; /* Clear the ADMUX register */
  ADMUX |= _BV(REFS0); /* Use AVcc as the reference (at328 table 28-3) */
  ADMUX |= _BV(ADLAR); /* Left adjust for 8 bit resolution (at328 section 28.9.6.) */
  ADMUX |= 0x5; /* Set ADC Input Channel to ADC5 (Arduino A5) (at328 table 28-4) */

  /* Enable de ADC */
  ADCSRA |= _BV(ADEN);
  
  /* Set prescaler to 16 (ADPS[2:0] = 100) (at328 table 28-5) */
  ADCSRA &= ~(_BV(ADPS1) | _BV(ADPS0));
  ADCSRA |= _BV(ADPS2);

  /*** Enable timer interrupt ***/
  TIMSK2 |= _BV(OCIE2A);

  /* Enable all interrupts */
  interrupts();
}

void loop() { /* Empty, but needed to avoid compilation errors */
}

ISR(TIMER2_COMPA_vect) {
  ADCSRA |= _BV(ADSC);
  
  ADC_val = ADCH;

  /*u = analogRead(in_pin)*5.0/1023.0; // Get new value from A0 (problema amb l'offset?? */
  u = ADCH*5.0/255.0-0.4;

  x[0] = xn[0] - v + u;
  x[1] = xn[0] + xn[1] - 2*v + u;

  if (x[1] >= 0) {
    v = 1;
  } else {
    v = -1;
  }

  for (i = 0; i < 2; i++) {
    xn[i] = x[i];
  }

  analogWrite(out_pin, v); /* Temporal */

  while (bit_is_set(ADCSRA, ADSC));
}
