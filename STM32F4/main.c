/*******************************************************

  STM32F4 Second Order Delta-Sigma Modulator

  github.com/monisc/DS-Modulator - April 2017

  STM32F407
  ChibiOS 2.6.2

  Using the Matlab simulations, a sampling frequency of
  1MHz is good enough for the second order modulator.

  Input analog signal: pin PC1 (ADC Channel 1)
  Output signal: pin PA4 (DAC Channel 1)

*******************************************************/

/*** Main files to use ChibiOS ***/
#include "ch.h"
#include "hal.h"

/*** ADC and DAC conversion group settings ***/
#define ADC_GRP_NUM_CHANNELS 1
#define ADC_GRP_BUF_DEPTH 1

#define DAC_GRP_NUM_CHANNELS 1
#define DAC_GRP_BUF_DEPTH 5 /**** REVISAR ****/

/*** Global variables ***/
static adcsample_t samples[ADC_GRP_NUM_CHANNELS * ADC_GRP_BUF_DEPTH]; /* Buffer for ADC samples */
static dacsample_t dac_buffer[DAC_GRP_BUF_DEPTH]; /* Buffer for DAC samples ***/

/* Useful variables */
int i;
static int running = 0;

/* Input and output values */
double u = 0;
double v = 0;

double x[2] = {0, 0};
double xn[2] = {0, 0};

/*** Configuration ***/
/* GPT2 configuration */
static const GPTConfig gpt2cfg = {
		1000000, /* Timer clock = 1MHz */
		gpt2cb, /* Timer callback pointer */
};

/* GPT2 callback */
static void gpt2cb(GPTDriver *gptp) {
	(void)gptp;

	/* Voltage = value * ADC_SUPPLY_VOLTAGE/0xFFF */
	u = samples[0]*3.3/4095.0; /* adcsample_t is of type uint16_t */

	/*** Second Order Delta-Sigma equations ***/
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

	/*** REVISAR ***/
	/* Reconverting the voltage to samples (1 --> 2000, -1 --> 0) */
	dac_buffer[0] = (1+v)*1000; /* Per ara agafem valor arbitrari 2000 */
};

/* ADC conversion group */
static const ADCConversionGroup adcgrpcfg = {
		.circular = TRUE, /* Enables the circular buffer mode for the group */
		.num_channels = ADC_GRP_NUM_CHANNELS, /* Number of analog channels belonging to the conversion group */
		.end_cb = NULL, /* Callback function */
		.error_cb = NULL, /* Error callback */
		.cr1 = 0, /* CR1 register content */
		.cr2 = ADC_CR2_SWSTART, /* CR2 register content */
		.smpr1 = ADC_SMPR1_SMP_AN11(ADC_SAMPLE_3), /* SMPR1 register content */
		.smpr2 = 0, /* SMPR2 register content */
		.sqr1 = ADC_SQR1_NUM_CH(ADC_GRP_NUM_CHANNELS), /* SQR1 register content */
		.sqr2 = 0, /* SQR2 register content */
		.sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11) /* SQR3 register content */
};

/* DAC conversion group */
static const DACConversionGroup dacgrpcfg = {
		.num_channels = DAC_GRP_NUM_CHANNELS,
		.end_cb       = NULL,
		.error_cb     = NULL,
		.trigger      = DAC_TRG(0)
};

/*** Thread ***/
static WORKING_AREA(waThreadDS, 128);

static msg_t ThreadDS(void *arg) {
	(void)arg;

	chRegSetThreadName("Delta-Sigma");

	while (TRUE) {
		if (palReadPad(GPIOA, GPIOA_BUTTON)) {
			if (running) {
				gptStopTimer(&GPTD2);
				running = 0;
			} else {
				gptStartContinuous(&GPTD2, 1); /* Trigger at the value of 1 (so 1MHz rate) */
				running = 1;
			}
		}

		chThdSleepMilliseconds(100);
	}

	return 0;
}

int main(void) {
	/* System initializations */
	halInit();
	chSysInit();

	/* Analog pins setup (input: PC1, output: PA4) */
	palSetGroupMode(GPIOC, PAL_PORT_BIT(1), 0, PAL_MODE_INPUT_ANALOG); /* PC1 */
	palSetPadMode(GPIOA, PAL_PORT_BIT(4), 0, PAL_MODE_INPUT_ANALOG); /* PA4 */

	/* Create the DS (Delta-Sigma) thread */
	chThdCreateStatic(waThreadDS, sizeof(waThreadDS), NORMALPRIO, ThreadDS, NULL);

	/* Start the sampling timer */
	gptStart(&GPTD2, &gpt2cfg);

	/* Start the ADC and the conversion */
	adcStart(&ADCD1, NULL);
	adcStartConversion(&ADCD1, &adcgrpcfg, samples, ADC_GRP_BUF_DEPTH);

	/* Start the DAC and the conversion */
	dacStart(&DACD1, NULL);
	dacStartConversion(&DACD1, &dacgrpcfg, dac_buffer, DAC_GRP_BUF_DEPTH);

	/* A loop that keeps the system running */
	while (TRUE) {
		chThdSleepMilliseconds(5000);
	}
}
