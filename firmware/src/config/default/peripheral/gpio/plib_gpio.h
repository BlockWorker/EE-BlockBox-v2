/*******************************************************************************
  GPIO PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_gpio.h

  Summary:
    GPIO PLIB Header File

  Description:
    This library provides an interface to control and interact with Parallel
    Input/Output controller (GPIO) module.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#ifndef PLIB_GPIO_H
#define PLIB_GPIO_H

#include <device.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data types and constants
// *****************************************************************************
// *****************************************************************************

/*** Macros for LCD_BL_CTL pin ***/
#define LCD_BL_CTL_Set()               (LATGSET = (1<<15))
#define LCD_BL_CTL_Clear()             (LATGCLR = (1<<15))
#define LCD_BL_CTL_Toggle()            (LATGINV= (1<<15))
#define LCD_BL_CTL_Get()               ((PORTG >> 15) & 0x1)
#define LCD_BL_CTL_OutputEnable()      (TRISGCLR = (1<<15))
#define LCD_BL_CTL_InputEnable()       (TRISGSET = (1<<15))
#define LCD_BL_CTL_PIN                  GPIO_PIN_RG15
/*** Macros for LCD_ADDPIN pin ***/
#define LCD_ADDPIN_Set()               (LATASET = (1<<5))
#define LCD_ADDPIN_Clear()             (LATACLR = (1<<5))
#define LCD_ADDPIN_Toggle()            (LATAINV= (1<<5))
#define LCD_ADDPIN_Get()               ((PORTA >> 5) & 0x1)
#define LCD_ADDPIN_OutputEnable()      (TRISACLR = (1<<5))
#define LCD_ADDPIN_InputEnable()       (TRISASET = (1<<5))
#define LCD_ADDPIN_PIN                  GPIO_PIN_RA5
/*** Macros for LCD_GPIO3 pin ***/
#define LCD_GPIO3_Set()               (LATESET = (1<<5))
#define LCD_GPIO3_Clear()             (LATECLR = (1<<5))
#define LCD_GPIO3_Toggle()            (LATEINV= (1<<5))
#define LCD_GPIO3_Get()               ((PORTE >> 5) & 0x1)
#define LCD_GPIO3_OutputEnable()      (TRISECLR = (1<<5))
#define LCD_GPIO3_InputEnable()       (TRISESET = (1<<5))
#define LCD_GPIO3_PIN                  GPIO_PIN_RE5
/*** Macros for LCD_GPIO2 pin ***/
#define LCD_GPIO2_Set()               (LATESET = (1<<6))
#define LCD_GPIO2_Clear()             (LATECLR = (1<<6))
#define LCD_GPIO2_Toggle()            (LATEINV= (1<<6))
#define LCD_GPIO2_Get()               ((PORTE >> 6) & 0x1)
#define LCD_GPIO2_OutputEnable()      (TRISECLR = (1<<6))
#define LCD_GPIO2_InputEnable()       (TRISESET = (1<<6))
#define LCD_GPIO2_PIN                  GPIO_PIN_RE6
/*** Macros for LCD_GPIO1 pin ***/
#define LCD_GPIO1_Set()               (LATESET = (1<<7))
#define LCD_GPIO1_Clear()             (LATECLR = (1<<7))
#define LCD_GPIO1_Toggle()            (LATEINV= (1<<7))
#define LCD_GPIO1_Get()               ((PORTE >> 7) & 0x1)
#define LCD_GPIO1_OutputEnable()      (TRISECLR = (1<<7))
#define LCD_GPIO1_InputEnable()       (TRISESET = (1<<7))
#define LCD_GPIO1_PIN                  GPIO_PIN_RE7
/*** Macros for LCD_GPIO0 pin ***/
#define LCD_GPIO0_Set()               (LATCSET = (1<<1))
#define LCD_GPIO0_Clear()             (LATCCLR = (1<<1))
#define LCD_GPIO0_Toggle()            (LATCINV= (1<<1))
#define LCD_GPIO0_Get()               ((PORTC >> 1) & 0x1)
#define LCD_GPIO0_OutputEnable()      (TRISCCLR = (1<<1))
#define LCD_GPIO0_InputEnable()       (TRISCSET = (1<<1))
#define LCD_GPIO0_PIN                  GPIO_PIN_RC1
/*** Macros for LCD_PD_N pin ***/
#define LCD_PD_N_Set()               (LATCSET = (1<<3))
#define LCD_PD_N_Clear()             (LATCCLR = (1<<3))
#define LCD_PD_N_Toggle()            (LATCINV= (1<<3))
#define LCD_PD_N_Get()               ((PORTC >> 3) & 0x1)
#define LCD_PD_N_OutputEnable()      (TRISCCLR = (1<<3))
#define LCD_PD_N_InputEnable()       (TRISCSET = (1<<3))
#define LCD_PD_N_PIN                  GPIO_PIN_RC3
/*** Macros for LED_MODE1 pin ***/
#define LED_MODE1_Set()               (LATGSET = (1<<9))
#define LED_MODE1_Clear()             (LATGCLR = (1<<9))
#define LED_MODE1_Toggle()            (LATGINV= (1<<9))
#define LED_MODE1_Get()               ((PORTG >> 9) & 0x1)
#define LED_MODE1_OutputEnable()      (TRISGCLR = (1<<9))
#define LED_MODE1_InputEnable()       (TRISGSET = (1<<9))
#define LED_MODE1_PIN                  GPIO_PIN_RG9
/*** Macros for LED_MODE2 pin ***/
#define LED_MODE2_Set()               (LATASET = (1<<0))
#define LED_MODE2_Clear()             (LATACLR = (1<<0))
#define LED_MODE2_Toggle()            (LATAINV= (1<<0))
#define LED_MODE2_Get()               ((PORTA >> 0) & 0x1)
#define LED_MODE2_OutputEnable()      (TRISACLR = (1<<0))
#define LED_MODE2_InputEnable()       (TRISASET = (1<<0))
#define LED_MODE2_PIN                  GPIO_PIN_RA0
/*** Macros for CHG_FULLCHG_N pin ***/
#define CHG_FULLCHG_N_Set()               (LATBSET = (1<<8))
#define CHG_FULLCHG_N_Clear()             (LATBCLR = (1<<8))
#define CHG_FULLCHG_N_Toggle()            (LATBINV= (1<<8))
#define CHG_FULLCHG_N_Get()               ((PORTB >> 8) & 0x1)
#define CHG_FULLCHG_N_OutputEnable()      (TRISBCLR = (1<<8))
#define CHG_FULLCHG_N_InputEnable()       (TRISBSET = (1<<8))
#define CHG_FULLCHG_N_PIN                  GPIO_PIN_RB8
/*** Macros for CHG_FASTCHG_N pin ***/
#define CHG_FASTCHG_N_Set()               (LATBSET = (1<<9))
#define CHG_FASTCHG_N_Clear()             (LATBCLR = (1<<9))
#define CHG_FASTCHG_N_Toggle()            (LATBINV= (1<<9))
#define CHG_FASTCHG_N_Get()               ((PORTB >> 9) & 0x1)
#define CHG_FASTCHG_N_OutputEnable()      (TRISBCLR = (1<<9))
#define CHG_FASTCHG_N_InputEnable()       (TRISBSET = (1<<9))
#define CHG_FASTCHG_N_PIN                  GPIO_PIN_RB9
/*** Macros for CHG_FAULT_N pin ***/
#define CHG_FAULT_N_Set()               (LATBSET = (1<<10))
#define CHG_FAULT_N_Clear()             (LATBCLR = (1<<10))
#define CHG_FAULT_N_Toggle()            (LATBINV= (1<<10))
#define CHG_FAULT_N_Get()               ((PORTB >> 10) & 0x1)
#define CHG_FAULT_N_OutputEnable()      (TRISBCLR = (1<<10))
#define CHG_FAULT_N_InputEnable()       (TRISBSET = (1<<10))
#define CHG_FAULT_N_PIN                  GPIO_PIN_RB10
/*** Macros for AMP_CLIP_N pin ***/
#define AMP_CLIP_N_Set()               (LATFSET = (1<<8))
#define AMP_CLIP_N_Clear()             (LATFCLR = (1<<8))
#define AMP_CLIP_N_Toggle()            (LATFINV= (1<<8))
#define AMP_CLIP_N_Get()               ((PORTF >> 8) & 0x1)
#define AMP_CLIP_N_OutputEnable()      (TRISFCLR = (1<<8))
#define AMP_CLIP_N_InputEnable()       (TRISFSET = (1<<8))
#define AMP_CLIP_N_PIN                  GPIO_PIN_RF8
/*** Macros for AMP_OTW_N pin ***/
#define AMP_OTW_N_Set()               (LATASET = (1<<2))
#define AMP_OTW_N_Clear()             (LATACLR = (1<<2))
#define AMP_OTW_N_Toggle()            (LATAINV= (1<<2))
#define AMP_OTW_N_Get()               ((PORTA >> 2) & 0x1)
#define AMP_OTW_N_OutputEnable()      (TRISACLR = (1<<2))
#define AMP_OTW_N_InputEnable()       (TRISASET = (1<<2))
#define AMP_OTW_N_PIN                  GPIO_PIN_RA2
/*** Macros for AMP_FAULT_N pin ***/
#define AMP_FAULT_N_Set()               (LATASET = (1<<3))
#define AMP_FAULT_N_Clear()             (LATACLR = (1<<3))
#define AMP_FAULT_N_Toggle()            (LATAINV= (1<<3))
#define AMP_FAULT_N_Get()               ((PORTA >> 3) & 0x1)
#define AMP_FAULT_N_OutputEnable()      (TRISACLR = (1<<3))
#define AMP_FAULT_N_InputEnable()       (TRISASET = (1<<3))
#define AMP_FAULT_N_PIN                  GPIO_PIN_RA3
/*** Macros for AMP_RESET_N pin ***/
#define AMP_RESET_N_Set()               (LATASET = (1<<4))
#define AMP_RESET_N_Clear()             (LATACLR = (1<<4))
#define AMP_RESET_N_Toggle()            (LATAINV= (1<<4))
#define AMP_RESET_N_Get()               ((PORTA >> 4) & 0x1)
#define AMP_RESET_N_OutputEnable()      (TRISACLR = (1<<4))
#define AMP_RESET_N_InputEnable()       (TRISASET = (1<<4))
#define AMP_RESET_N_PIN                  GPIO_PIN_RA4
/*** Macros for DSP_VALID pin ***/
#define DSP_VALID_Set()               (LATFSET = (1<<4))
#define DSP_VALID_Clear()             (LATFCLR = (1<<4))
#define DSP_VALID_Toggle()            (LATFINV= (1<<4))
#define DSP_VALID_Get()               ((PORTF >> 4) & 0x1)
#define DSP_VALID_OutputEnable()      (TRISFCLR = (1<<4))
#define DSP_VALID_InputEnable()       (TRISFSET = (1<<4))
#define DSP_VALID_PIN                  GPIO_PIN_RF4
/*** Macros for DSP_BKND_ERR_N pin ***/
#define DSP_BKND_ERR_N_Set()               (LATFSET = (1<<5))
#define DSP_BKND_ERR_N_Clear()             (LATFCLR = (1<<5))
#define DSP_BKND_ERR_N_Toggle()            (LATFINV= (1<<5))
#define DSP_BKND_ERR_N_Get()               ((PORTF >> 5) & 0x1)
#define DSP_BKND_ERR_N_OutputEnable()      (TRISFCLR = (1<<5))
#define DSP_BKND_ERR_N_InputEnable()       (TRISFSET = (1<<5))
#define DSP_BKND_ERR_N_PIN                  GPIO_PIN_RF5
/*** Macros for DSP_MUTE_N pin ***/
#define DSP_MUTE_N_Set()               (LATDSET = (1<<9))
#define DSP_MUTE_N_Clear()             (LATDCLR = (1<<9))
#define DSP_MUTE_N_Toggle()            (LATDINV= (1<<9))
#define DSP_MUTE_N_Get()               ((PORTD >> 9) & 0x1)
#define DSP_MUTE_N_OutputEnable()      (TRISDCLR = (1<<9))
#define DSP_MUTE_N_InputEnable()       (TRISDSET = (1<<9))
#define DSP_MUTE_N_PIN                  GPIO_PIN_RD9
/*** Macros for DSP_PDN_N pin ***/
#define DSP_PDN_N_Set()               (LATDSET = (1<<10))
#define DSP_PDN_N_Clear()             (LATDCLR = (1<<10))
#define DSP_PDN_N_Toggle()            (LATDINV= (1<<10))
#define DSP_PDN_N_Get()               ((PORTD >> 10) & 0x1)
#define DSP_PDN_N_OutputEnable()      (TRISDCLR = (1<<10))
#define DSP_PDN_N_InputEnable()       (TRISDSET = (1<<10))
#define DSP_PDN_N_PIN                  GPIO_PIN_RD10
/*** Macros for DSP_RESET_N pin ***/
#define DSP_RESET_N_Set()               (LATDSET = (1<<11))
#define DSP_RESET_N_Clear()             (LATDCLR = (1<<11))
#define DSP_RESET_N_Toggle()            (LATDINV= (1<<11))
#define DSP_RESET_N_Get()               ((PORTD >> 11) & 0x1)
#define DSP_RESET_N_OutputEnable()      (TRISDCLR = (1<<11))
#define DSP_RESET_N_InputEnable()       (TRISDSET = (1<<11))
#define DSP_RESET_N_PIN                  GPIO_PIN_RD11
/*** Macros for DSP_EMO1 pin ***/
#define DSP_EMO1_Set()               (LATDSET = (1<<0))
#define DSP_EMO1_Clear()             (LATDCLR = (1<<0))
#define DSP_EMO1_Toggle()            (LATDINV= (1<<0))
#define DSP_EMO1_Get()               ((PORTD >> 0) & 0x1)
#define DSP_EMO1_OutputEnable()      (TRISDCLR = (1<<0))
#define DSP_EMO1_InputEnable()       (TRISDSET = (1<<0))
#define DSP_EMO1_PIN                  GPIO_PIN_RD0
/*** Macros for AUX_DETECT pin ***/
#define AUX_DETECT_Set()               (LATCSET = (1<<14))
#define AUX_DETECT_Clear()             (LATCCLR = (1<<14))
#define AUX_DETECT_Toggle()            (LATCINV= (1<<14))
#define AUX_DETECT_Get()               ((PORTC >> 14) & 0x1)
#define AUX_DETECT_OutputEnable()      (TRISCCLR = (1<<14))
#define AUX_DETECT_InputEnable()       (TRISCSET = (1<<14))
#define AUX_DETECT_PIN                  GPIO_PIN_RC14
/*** Macros for BM_RST_N pin ***/
#define BM_RST_N_Set()               (LATDSET = (1<<13))
#define BM_RST_N_Clear()             (LATDCLR = (1<<13))
#define BM_RST_N_Toggle()            (LATDINV= (1<<13))
#define BM_RST_N_Get()               ((PORTD >> 13) & 0x1)
#define BM_RST_N_OutputEnable()      (TRISDCLR = (1<<13))
#define BM_RST_N_InputEnable()       (TRISDSET = (1<<13))
#define BM_RST_N_PIN                  GPIO_PIN_RD13
/*** Macros for BM_MFB pin ***/
#define BM_MFB_Set()               (LATFSET = (1<<0))
#define BM_MFB_Clear()             (LATFCLR = (1<<0))
#define BM_MFB_Toggle()            (LATFINV= (1<<0))
#define BM_MFB_Get()               ((PORTF >> 0) & 0x1)
#define BM_MFB_OutputEnable()      (TRISFCLR = (1<<0))
#define BM_MFB_InputEnable()       (TRISFSET = (1<<0))
#define BM_MFB_PIN                  GPIO_PIN_RF0
/*** Macros for CTL_VOL_DOWN pin ***/
#define CTL_VOL_DOWN_Set()               (LATESET = (1<<1))
#define CTL_VOL_DOWN_Clear()             (LATECLR = (1<<1))
#define CTL_VOL_DOWN_Toggle()            (LATEINV= (1<<1))
#define CTL_VOL_DOWN_Get()               ((PORTE >> 1) & 0x1)
#define CTL_VOL_DOWN_OutputEnable()      (TRISECLR = (1<<1))
#define CTL_VOL_DOWN_InputEnable()       (TRISESET = (1<<1))
#define CTL_VOL_DOWN_InterruptEnable()   (CNENESET = (1<<1))
#define CTL_VOL_DOWN_InterruptDisable()  (CNENECLR = (1<<1))
#define CTL_VOL_DOWN_PIN                  GPIO_PIN_RE1
/*** Macros for CTL_MFB pin ***/
#define CTL_MFB_Set()               (LATGSET = (1<<14))
#define CTL_MFB_Clear()             (LATGCLR = (1<<14))
#define CTL_MFB_Toggle()            (LATGINV= (1<<14))
#define CTL_MFB_Get()               ((PORTG >> 14) & 0x1)
#define CTL_MFB_OutputEnable()      (TRISGCLR = (1<<14))
#define CTL_MFB_InputEnable()       (TRISGSET = (1<<14))
#define CTL_MFB_InterruptEnable()   (CNENGSET = (1<<14))
#define CTL_MFB_InterruptDisable()  (CNENGCLR = (1<<14))
#define CTL_MFB_PIN                  GPIO_PIN_RG14
/*** Macros for CTL_VOL_UP pin ***/
#define CTL_VOL_UP_Set()               (LATGSET = (1<<12))
#define CTL_VOL_UP_Clear()             (LATGCLR = (1<<12))
#define CTL_VOL_UP_Toggle()            (LATGINV= (1<<12))
#define CTL_VOL_UP_Get()               ((PORTG >> 12) & 0x1)
#define CTL_VOL_UP_OutputEnable()      (TRISGCLR = (1<<12))
#define CTL_VOL_UP_InputEnable()       (TRISGSET = (1<<12))
#define CTL_VOL_UP_InterruptEnable()   (CNENGSET = (1<<12))
#define CTL_VOL_UP_InterruptDisable()  (CNENGCLR = (1<<12))
#define CTL_VOL_UP_PIN                  GPIO_PIN_RG12
/*** Macros for CTL_BACK pin ***/
#define CTL_BACK_Set()               (LATGSET = (1<<13))
#define CTL_BACK_Clear()             (LATGCLR = (1<<13))
#define CTL_BACK_Toggle()            (LATGINV= (1<<13))
#define CTL_BACK_Get()               ((PORTG >> 13) & 0x1)
#define CTL_BACK_OutputEnable()      (TRISGCLR = (1<<13))
#define CTL_BACK_InputEnable()       (TRISGSET = (1<<13))
#define CTL_BACK_InterruptEnable()   (CNENGSET = (1<<13))
#define CTL_BACK_InterruptDisable()  (CNENGCLR = (1<<13))
#define CTL_BACK_PIN                  GPIO_PIN_RG13
/*** Macros for CTL_PLAY pin ***/
#define CTL_PLAY_Set()               (LATESET = (1<<2))
#define CTL_PLAY_Clear()             (LATECLR = (1<<2))
#define CTL_PLAY_Toggle()            (LATEINV= (1<<2))
#define CTL_PLAY_Get()               ((PORTE >> 2) & 0x1)
#define CTL_PLAY_OutputEnable()      (TRISECLR = (1<<2))
#define CTL_PLAY_InputEnable()       (TRISESET = (1<<2))
#define CTL_PLAY_InterruptEnable()   (CNENESET = (1<<2))
#define CTL_PLAY_InterruptDisable()  (CNENECLR = (1<<2))
#define CTL_PLAY_PIN                  GPIO_PIN_RE2
/*** Macros for CTL_FWD pin ***/
#define CTL_FWD_Set()               (LATESET = (1<<3))
#define CTL_FWD_Clear()             (LATECLR = (1<<3))
#define CTL_FWD_Toggle()            (LATEINV= (1<<3))
#define CTL_FWD_Get()               ((PORTE >> 3) & 0x1)
#define CTL_FWD_OutputEnable()      (TRISECLR = (1<<3))
#define CTL_FWD_InputEnable()       (TRISESET = (1<<3))
#define CTL_FWD_InterruptEnable()   (CNENESET = (1<<3))
#define CTL_FWD_InterruptDisable()  (CNENECLR = (1<<3))
#define CTL_FWD_PIN                  GPIO_PIN_RE3
/*** Macros for LCD_BL_SW pin ***/
#define LCD_BL_SW_Set()               (LATESET = (1<<4))
#define LCD_BL_SW_Clear()             (LATECLR = (1<<4))
#define LCD_BL_SW_Toggle()            (LATEINV= (1<<4))
#define LCD_BL_SW_Get()               ((PORTE >> 4) & 0x1)
#define LCD_BL_SW_OutputEnable()      (TRISECLR = (1<<4))
#define LCD_BL_SW_InputEnable()       (TRISESET = (1<<4))
#define LCD_BL_SW_PIN                  GPIO_PIN_RE4


// *****************************************************************************
/* GPIO Port

  Summary:
    Identifies the available GPIO Ports.

  Description:
    This enumeration identifies the available GPIO Ports.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all ports are available on all devices.  Refer to the specific
    device data sheet to determine which ports are supported.
*/

typedef enum
{
    GPIO_PORT_A = 0,
    GPIO_PORT_B = 1,
    GPIO_PORT_C = 2,
    GPIO_PORT_D = 3,
    GPIO_PORT_E = 4,
    GPIO_PORT_F = 5,
    GPIO_PORT_G = 6,
} GPIO_PORT;

// *****************************************************************************
/* GPIO Port Pins

  Summary:
    Identifies the available GPIO port pins.

  Description:
    This enumeration identifies the available GPIO port pins.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all pins are available on all devices.  Refer to the specific
    device data sheet to determine which pins are supported.
*/

typedef enum
{
    GPIO_PIN_RA0 = 0,
    GPIO_PIN_RA1 = 1,
    GPIO_PIN_RA2 = 2,
    GPIO_PIN_RA3 = 3,
    GPIO_PIN_RA4 = 4,
    GPIO_PIN_RA5 = 5,
    GPIO_PIN_RA6 = 6,
    GPIO_PIN_RA7 = 7,
    GPIO_PIN_RA9 = 9,
    GPIO_PIN_RA10 = 10,
    GPIO_PIN_RA14 = 14,
    GPIO_PIN_RA15 = 15,
    GPIO_PIN_RB0 = 16,
    GPIO_PIN_RB1 = 17,
    GPIO_PIN_RB2 = 18,
    GPIO_PIN_RB3 = 19,
    GPIO_PIN_RB4 = 20,
    GPIO_PIN_RB5 = 21,
    GPIO_PIN_RB6 = 22,
    GPIO_PIN_RB7 = 23,
    GPIO_PIN_RB8 = 24,
    GPIO_PIN_RB9 = 25,
    GPIO_PIN_RB10 = 26,
    GPIO_PIN_RB11 = 27,
    GPIO_PIN_RB12 = 28,
    GPIO_PIN_RB13 = 29,
    GPIO_PIN_RB14 = 30,
    GPIO_PIN_RB15 = 31,
    GPIO_PIN_RC1 = 33,
    GPIO_PIN_RC2 = 34,
    GPIO_PIN_RC3 = 35,
    GPIO_PIN_RC4 = 36,
    GPIO_PIN_RC12 = 44,
    GPIO_PIN_RC13 = 45,
    GPIO_PIN_RC14 = 46,
    GPIO_PIN_RC15 = 47,
    GPIO_PIN_RD0 = 48,
    GPIO_PIN_RD1 = 49,
    GPIO_PIN_RD2 = 50,
    GPIO_PIN_RD3 = 51,
    GPIO_PIN_RD4 = 52,
    GPIO_PIN_RD5 = 53,
    GPIO_PIN_RD9 = 57,
    GPIO_PIN_RD10 = 58,
    GPIO_PIN_RD11 = 59,
    GPIO_PIN_RD12 = 60,
    GPIO_PIN_RD13 = 61,
    GPIO_PIN_RD14 = 62,
    GPIO_PIN_RD15 = 63,
    GPIO_PIN_RE0 = 64,
    GPIO_PIN_RE1 = 65,
    GPIO_PIN_RE2 = 66,
    GPIO_PIN_RE3 = 67,
    GPIO_PIN_RE4 = 68,
    GPIO_PIN_RE5 = 69,
    GPIO_PIN_RE6 = 70,
    GPIO_PIN_RE7 = 71,
    GPIO_PIN_RE8 = 72,
    GPIO_PIN_RE9 = 73,
    GPIO_PIN_RF0 = 80,
    GPIO_PIN_RF1 = 81,
    GPIO_PIN_RF2 = 82,
    GPIO_PIN_RF3 = 83,
    GPIO_PIN_RF4 = 84,
    GPIO_PIN_RF5 = 85,
    GPIO_PIN_RF8 = 88,
    GPIO_PIN_RF12 = 92,
    GPIO_PIN_RF13 = 93,
    GPIO_PIN_RG0 = 96,
    GPIO_PIN_RG1 = 97,
    GPIO_PIN_RG6 = 102,
    GPIO_PIN_RG7 = 103,
    GPIO_PIN_RG8 = 104,
    GPIO_PIN_RG9 = 105,
    GPIO_PIN_RG12 = 108,
    GPIO_PIN_RG13 = 109,
    GPIO_PIN_RG14 = 110,
    GPIO_PIN_RG15 = 111,

    /* This element should not be used in any of the GPIO APIs.
       It will be used by other modules or application to denote that none of the GPIO Pin is used */
    GPIO_PIN_NONE = -1

} GPIO_PIN;

typedef  void (*GPIO_PIN_CALLBACK) ( GPIO_PIN pin, uintptr_t context);

void GPIO_Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on multiple pins of a port
// *****************************************************************************
// *****************************************************************************

uint32_t GPIO_PortRead(GPIO_PORT port);

void GPIO_PortWrite(GPIO_PORT port, uint32_t mask, uint32_t value);

uint32_t GPIO_PortLatchRead ( GPIO_PORT port );

void GPIO_PortSet(GPIO_PORT port, uint32_t mask);

void GPIO_PortClear(GPIO_PORT port, uint32_t mask);

void GPIO_PortToggle(GPIO_PORT port, uint32_t mask);

void GPIO_PortInputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortOutputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortInterruptEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortInterruptDisable(GPIO_PORT port, uint32_t mask);

// *****************************************************************************
// *****************************************************************************
// Section: Local Data types and Prototypes
// *****************************************************************************
// *****************************************************************************

typedef struct {

    /* target pin */
    GPIO_PIN                 pin;

    /* Callback for event on target pin*/
    GPIO_PIN_CALLBACK        callback;

    /* Callback Context */
    uintptr_t               context;

} GPIO_PIN_CALLBACK_OBJ;

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on one pin at a time
// *****************************************************************************
// *****************************************************************************

static inline void GPIO_PinWrite(GPIO_PIN pin, bool value)
{
    GPIO_PortWrite(pin>>4, (uint32_t)(0x1) << (pin & 0xF), (uint32_t)(value) << (pin & 0xF));
}

static inline bool GPIO_PinRead(GPIO_PIN pin)
{
    return (bool)(((GPIO_PortRead(pin>>4)) >> (pin & 0xF)) & 0x1);
}

static inline bool GPIO_PinLatchRead(GPIO_PIN pin)
{
    return (bool)((GPIO_PortLatchRead(pin>>4) >> (pin & 0xF)) & 0x1);
}

static inline void GPIO_PinToggle(GPIO_PIN pin)
{
    GPIO_PortToggle(pin>>4, 0x1 << (pin & 0xF));
}

static inline void GPIO_PinSet(GPIO_PIN pin)
{
    GPIO_PortSet(pin>>4, 0x1 << (pin & 0xF));
}

static inline void GPIO_PinClear(GPIO_PIN pin)
{
    GPIO_PortClear(pin>>4, 0x1 << (pin & 0xF));
}

static inline void GPIO_PinInputEnable(GPIO_PIN pin)
{
    GPIO_PortInputEnable(pin>>4, 0x1 << (pin & 0xF));
}

static inline void GPIO_PinOutputEnable(GPIO_PIN pin)
{
    GPIO_PortOutputEnable(pin>>4, 0x1 << (pin & 0xF));
}

static inline void GPIO_PinInterruptEnable(GPIO_PIN pin)
{
    GPIO_PortInterruptEnable(pin>>4, 0x1 << (pin & 0xF));
}

static inline void GPIO_PinInterruptDisable(GPIO_PIN pin)
{
    GPIO_PortInterruptDisable(pin>>4, 0x1 << (pin & 0xF));
}

bool GPIO_PinInterruptCallbackRegister(
    GPIO_PIN pin,
    const   GPIO_PIN_CALLBACK callBack,
    uintptr_t context
);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // PLIB_GPIO_H
