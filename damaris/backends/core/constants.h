/* constants.h
 * Author: Stefan Reutter (2011)
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H


/* Settings for the spectrum M2i ADC driver */

/** default impedance for the spectrum ADC in Ohm */
const double ADC_M2I_DEFAULT_IMPEDANCE = 1e6;
/** 50 Ohm impedance is allowed as well */
const double ADC_M2I_ALLOWED_IMPEDANCE = 50.0;
/** default offset */
const int ADC_M2I_DEFAULT_OFFSET = 0;
/** default channel bitmask (2 first channels enabled) */
const int ADC_M2I_DEFAULT_CHANNELS = 3;
/** default sensitivity in volt */
const double ADC_M2I_DEFAULT_SENSITIVITY = 10.0;
/** default resolution in bits per sample */
const int ADC_M2I_DEFAULT_RESOLUTION = 14;
/** allowed sensitivity settings */
const double ADC_M2I_ALLOWED_SENSITIVITY[6] = {0.2, 0.5, 1, 2, 5, 10};
const int ADC_M2I_ALLOWED_SENSITIVITY_LENGTH = 6;

/* Settings for the spectrum MI ADC driver */

/** default impedance for the spectrum ADC in Ohm */
const double ADC_MI_DEFAULT_IMPEDANCE = 1e6;
/** 50 Ohm impedance is allowed as well */
const double ADC_MI_ALLOWED_IMPEDANCE = 50.0;
/** default offset */
const int ADC_MI_DEFAULT_OFFSET = 0;
/** default channel bitmask (2 first channels enabled) */
const int ADC_MI_DEFAULT_CHANNELS = 3;
/** default sensitivity in volt */
const double ADC_MI_DEFAULT_SENSITIVITY = 10.0;
/** default resolution in bits per sample */
const int ADC_MI_DEFAULT_RESOLUTION = 14;
/** allowed sensitivity settings */
const double ADC_MI_ALLOWED_SENSITIVITY[6] = {0.2, 0.5, 1, 2, 5, 10};
const int ADC_MI_ALLOWED_SENSITIVITY_LENGTH = 6;

#endif /* CONSTANTS_H_ */
