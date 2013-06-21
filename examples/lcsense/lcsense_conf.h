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
#define LESENSE_LCSENSE_SENSOR_CH_CONF                     \
{                                                  \
  true,                      /* Enable scan channel. */    \
  true,                      /* Enable the assigned pin on scan channel. */ \
  true,                      /* Enable interrupts on channel. */ \
  lesenseChPinExLow,         /* GPIO pin is disabled during the excitation period. */    \
  lesenseChPinIdleDis,       /* GPIO pin is disabled during the idle period. */ \
  false,                     /* Do not use alternate excitation pins for excitation. */    \
  false,                     /* Enabled to shift results from this channel to the decoder register. */ \
  false,                     /* Disabled to invert the scan result bit. */  \
  true,                      /* Enabled to store counter value in the result buffer. */   \
  lesenseClkHF,              /* Use the HF clock for excitation timing. */    \
  lesenseClkLF,              /* Use the LF clock for sample timing. */ \
  0x0FU,                     /* Excitation time is set to 15 (+1) excitation clock cycles. */    \
  0x02U,                     /* Sample delay is set to 2(+1) sample clock cycles. */ \
  0x00U,                     /* Measure delay is set to 0 excitation clock cycles.*/    \
  0x000U,                    /* ACMP threshold has been set to 0x7FF - not used, controled by the ACMP registers*/ \
  lesenseSampleModeCounter,  /* Counter will be used in comparison. */    \
  lesenseSetIntPosEdge,      /* Interrupt is generated if the sensor triggers. */ \
  0x0000U,                   /* Counter threshold has been set to 0x00 - Calibration in setupLESENSE() */    \
  lesenseCompModeLess        /* Compare mode has been set to trigger interrupt on "less". */ \
}

/** Configuration for disabled channels. */
#define LESENSE_DISABLED_CH_CONF                     \
{                                                  \
  false,                     /* Disable scan channel. */    \
  false,                     /* Disable the assigned pin on scan channel. */ \
  false,                     /* Disable interrupts on channel. */ \
  lesenseChPinExDis,          /* GPIO pin is disabled during the excitation period. */    \
  lesenseChPinIdleDis,        /* GPIO pin is disabled during the idle period. */ \
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

#define LESENSE_LCSENSE_SCAN_CONF                                         \
{                                                                            \
  {                                                                          \
    LESENSE_DISABLED_CH_CONF,          /* Channel 0. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 1. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 2. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 3. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 4. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 5. */                      \
    LESENSE_DISABLED_CH_CONF,          /* Channel 6. */                      \
    LESENSE_LCSENSE_SENSOR_CH_CONF,    /* Channel 7. */                      \
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

/** Example state 0/A. */
#define EXAMPLE_STATE_0_A                                                       \
{                                                                               \
  0x01U,               /**/                                                     \
  0xCU,                /**/                                                     \
  1U,                  /**/                                                     \
  lesenseTransActUp,   /**/                                                     \
  false                /**/                                                     \
}

/** Example state 0/B. */
#define EXAMPLE_STATE_0_B                                                       \
{                                                                               \
  0x01U,               /**/                                                     \
  0xCU,                /**/                                                     \
  4U,                  /**/                                                     \
  lesenseTransActNone, /**/                                                     \
  false                /**/                                                     \
}

/** Example state 1/A. */
#define EXAMPLE_STATE_1_A                                                       \
{                                                                               \
  0x02U,               /**/                                                     \
  0xCU,                /**/                                                     \
  2U,                  /**/                                                     \
  lesenseTransActUp,   /**/                                                     \
  false                /**/                                                     \
}

/** Example state 1/B. */
#define EXAMPLE_STATE_1_B                                                       \
{                                                                               \
  0x02U,               /**/                                                     \
  0xCU,                /**/                                                     \
  4U,                  /**/                                                     \
  lesenseTransActNone, /**/                                                     \
  false                /**/                                                     \
}

/** Example state 2/A. */
#define EXAMPLE_STATE_2_A                                                       \
{                                                                               \
  0x03U,               /**/                                                     \
  0xCU,                /**/                                                     \
  3U,                  /**/                                                     \
  lesenseTransActUp,   /**/                                                     \
  false                /**/                                                     \
}

/** Example state 2/B. */
#define EXAMPLE_STATE_2_B                                                       \
{                                                                               \
  0x03U,               /**/                                                     \
  0xCU,                /**/                                                     \
  4U,                  /**/                                                     \
  lesenseTransActNone, /**/                                                     \
  false                /**/                                                     \
}


/** Example state 3/A. */
#define EXAMPLE_STATE_3_A                                                       \
{                                                                               \
  0x00U,               /**/                                                     \
  0xCU,                /**/                                                     \
  0U,                  /**/                                                     \
  lesenseTransActUp,   /**/                                                     \
  false                /**/                                                     \
}

/** Example state 3/B. */
#define EXAMPLE_STATE_3_B                                                       \
{                                                                               \
  0x00U,               /**/                                                     \
  0xCU,                /**/                                                     \
  4U,                  /**/                                                     \
  lesenseTransActNone, /**/                                                     \
  false                /**/                                                     \
}

/** Example state 4/A. */
#define EXAMPLE_STATE_4_A                                                       \
{                                                                               \
  0x00U,               /**/                                                     \
  0xCU,                /**/                                                     \
  4U,                  /**/                                                     \
  lesenseTransActUp,   /**/                                                     \
  false                /**/                                                     \
}

/** Default configuration for all decoder states. */
#define LESENSE_EXAMPLE_DECODER_CONF                                            \
{ /* chain |   Descriptor A   |   Descriptor B   */                             \
  {                                                                             \
    {true, EXAMPLE_STATE_0_A, EXAMPLE_STATE_0_B}, /* Decoder state 0. */        \
    {true, EXAMPLE_STATE_1_A, EXAMPLE_STATE_1_B}, /* Decoder state 1. */        \
    {true, EXAMPLE_STATE_2_A, EXAMPLE_STATE_2_B}, /* Decoder state 2. */        \
    {true, EXAMPLE_STATE_3_A, EXAMPLE_STATE_3_B}, /* Decoder state 3. */        \
    {true, EXAMPLE_STATE_4_A, EXAMPLE_STATE_4_A}, /* Decoder state 4. */        \
    {true, DEFAULT_STATE_CONF, DEFAULT_STATE_CONF}, /* Decoder state 5. */      \
    {true, DEFAULT_STATE_CONF, DEFAULT_STATE_CONF}, /* Decoder state 6. */      \
    {true, DEFAULT_STATE_CONF, DEFAULT_STATE_CONF}, /* Decoder state 7. */      \
    {true, DEFAULT_STATE_CONF, DEFAULT_STATE_CONF}, /* Decoder state 8. */      \
    {true, DEFAULT_STATE_CONF, DEFAULT_STATE_CONF}, /* Decoder state 9. */      \
    {true, DEFAULT_STATE_CONF, DEFAULT_STATE_CONF}, /* Decoder state 10. */     \
    {true, DEFAULT_STATE_CONF, DEFAULT_STATE_CONF}, /* Decoder state 11. */     \
    {true, DEFAULT_STATE_CONF, DEFAULT_STATE_CONF}, /* Decoder state 12. */     \
    {true, DEFAULT_STATE_CONF, DEFAULT_STATE_CONF}, /* Decoder state 13. */     \
    {true, DEFAULT_STATE_CONF, DEFAULT_STATE_CONF}, /* Decoder state 14. */     \
    {true, DEFAULT_STATE_CONF, DEFAULT_STATE_CONF}  /* Decoder state 15. */     \
  }                                                                             \
}

#ifdef __cplusplus
}
#endif

