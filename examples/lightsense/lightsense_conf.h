/***************************************************************************//**
 * @file
 * @brief Low Energy Sensor (LESENSE) example configuration file.
 * @author Energy Micro AS
 * @version 3.20.0
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2012 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/

#include "em_lesense.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Configuration for capacitive sense channels. */
#define LESENSE_LIGHTSENSE_SENSOR_CH_CONF                     \
{                                                  \
  true,                      /* Enable scan channel. */    \
  false,                     /* Enable the assigned pin on scan channel. */ \
  true,                      /* Enable interrupts on channel. */ \
  lesenseChPinExHigh,        /* GPIO pin is high during the excitation period. */    \
  lesenseChPinIdleDis,       /* GPIO pin is low during the idle period. */ \
  true,                      /* Use alternate excitation pins for excitation. */    \
  false,                     /* Disabled to shift results from this channel to the decoder register. */ \
  false,                     /* Disabled to invert the scan result bit. */  \
  true,                      /* Enabled to store counter value in the result buffer. */   \
  lesenseClkLF,              /* Use the LF clock for excitation timing. */    \
  lesenseClkLF,              /* Use the LF clock for sample timing. */ \
  0x01U,                     /* Excitation time is set to 1(+1) excitation clock cycles. */    \
  0x01U,                     /* Sample delay is set to 1(+1) sample clock cycles. */ \
  0x00U,                     /* Measure delay is set to 0 excitation clock cycles.*/    \
  0x38U,                     /* ACMP threshold has been set to 0x38. */ \
  lesenseSampleModeACMP,     /* ACMP will be used in comparison. */    \
  lesenseSetIntNegEdge,      /* Interrupt is generated if the sensor triggers. */ \
  0x0000U,                   /* Counter threshold has been set to 0x00. */    \
  lesenseCompModeLess        /* Compare mode has been set to trigger interrupt on "less". */ \
}

/** Configuration for disabled channels. */
#define LESENSE_DISABLED_CH_CONF                     \
{                                                  \
  false,                     /* Disable scan channel. */    \
  false,                     /* Disable the assigned pin on scan channel. */ \
  false,                     /* Disable interrupts on channel. */ \
  lesenseChPinExDis,         /* GPIO pin is disabled during the excitation period. */    \
  lesenseChPinIdleDis,       /* GPIO pin is disabled during the idle period. */ \
  false,                     /* Don't use alternate excitation pins for excitation. */    \
  false,                     /* Disabled to shift results from this channel to the decoder register. */ \
  false,                     /* Disabled to invert the scan result bit. */  \
  false,                     /* Disabled to store counter value in the result buffer. */   \
  lesenseClkLF,              /* Use the LF clock for excitation timing. */    \
  lesenseClkLF,              /* Use the LF clock for sample timing. */ \
  0x00U,                     /* Excitation time is set to 5(+1) excitation clock cycles. */    \
  0x00U,                     /* Sample delay is set to 7(+1) sample clock cycles. */ \
  0x00U,                     /* Measure delay is set to 0 excitation clock cycles.*/    \
  0x00U,                     /* ACMP threshold has been set to 0. */ \
  lesenseSampleModeCounter,  /* ACMP output will be used in comparison. */    \
  lesenseSetIntNone,         /* No interrupt is generated by the channel. */ \
  0x00U,                     /* Counter threshold has been set to 0x01. */    \
  lesenseCompModeLess        /* Compare mode has been set to trigger interrupt on "less". */ \
}

#define LESENSE_LIGHTSENSE_SCAN_CONF                                         \
{                                                                            \
  {                                                                          \
    LESENSE_DISABLED_CH_CONF,          /* Channel 0. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 1. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 2. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 3. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 4. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 5. */                      \
    LESENSE_LIGHTSENSE_SENSOR_CH_CONF, /* Channel 6. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 7. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 8. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 9. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 10. */                     \
    LESENSE_DISABLED_CH_CONF,          /* Channel 11. */                     \
    LESENSE_DISABLED_CH_CONF,          /* Channel 12. */                     \
    LESENSE_DISABLED_CH_CONF,          /* Channel 13. */                     \
    LESENSE_DISABLED_CH_CONF,          /* Channel 14. */                     \
    LESENSE_DISABLED_CH_CONF           /* Channel 15. */                     \
  }                                                                          \
}


/** Default configuration for alternate excitation channel. */
#define LESENSE_LIGHTSENSE_ALTEX_CH_CONF                                       \
{                                                                              \
  true,                   /* Alternate excitation enabled.*/                  \
  lesenseAltExPinIdleDis, /* Alternate excitation pin is low in idle. */ \
  true                    /* Excite only for corresponding channel. */        \
}

/** Default configuration for alternate excitation channel. */
#define LESENSE_LIGHTSENSE_ALTEX_DIS_CH_CONF                                   \
{                                                                              \
  false,                   /* Alternate excitation enabled.*/                  \
  lesenseAltExPinIdleDis,  /* Alternate excitation pin is disabled in idle. */ \
  false                    /* Excite only for corresponding channel. */        \
}

/** Default configuration for all alternate excitation channels. */
#define LESENSE_LIGHTSENSE_ALTEX_CONF                                          \
{                                                                              \
  lesenseAltExMapALTEX,                                                         \
  {                                                                            \
    LESENSE_LIGHTSENSE_ALTEX_CH_CONF,     /* Alternate excitation channel 0. */\
    LESENSE_LIGHTSENSE_ALTEX_DIS_CH_CONF, /* Alternate excitation channel 1. */\
    LESENSE_LIGHTSENSE_ALTEX_DIS_CH_CONF, /* Alternate excitation channel 2. */\
    LESENSE_LIGHTSENSE_ALTEX_DIS_CH_CONF, /* Alternate excitation channel 3. */\
    LESENSE_LIGHTSENSE_ALTEX_DIS_CH_CONF, /* Alternate excitation channel 4. */\
    LESENSE_LIGHTSENSE_ALTEX_DIS_CH_CONF, /* Alternate excitation channel 5. */\
    LESENSE_LIGHTSENSE_ALTEX_DIS_CH_CONF, /* Alternate excitation channel 6. */\
    LESENSE_LIGHTSENSE_ALTEX_DIS_CH_CONF  /* Alternate excitation channel 7. */\
  }                                                                            \
}

#ifdef __cplusplus
}
#endif

