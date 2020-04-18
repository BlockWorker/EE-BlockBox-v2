/* ************************************************************************** */


#ifndef _UI_H    /* Guard against multiple inclusion */
#define _UI_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "app.h"

#define UI_TOUCH
//#undef UI_TOUCH

#ifndef UI_TOUCH
#define LCD_RS_Set()        (LATGSET = (1<<6))
#define LCD_RS_Clear()      (LATGCLR = (1<<6))
#define LCD_RW_Set()        (LATGSET = (1<<7))
#define LCD_RW_Clear()      (LATGCLR = (1<<7))
#define LCD_E_N_Set()       (LATGSET = (1<<8))
#define LCD_E_N_Clear()     (LATGCLR = (1<<8))
#define LCD_D0_Put(x)       (LATCbits.LATC2 = x)
#define LCD_D1_Put(x)       (LATCbits.LATC4 = x)
#define LCD_D2_Put(x)       (LATCbits.LATC3 = x)
#define LCD_D3_Put(x)       (LATAbits.LATA5 = x)
#define LCD_D4_Put(x)       (LATCbits.LATC1 = x)
#define LCD_D5_Put(x)       (LATEbits.LATE7 = x)
#define LCD_D6_Put(x)       (LATEbits.LATE6 = x)
#define LCD_D7_Put(x)       (LATEbits.LATE5 = x)
#endif

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


    /* ************************************************************************** */
    /* ************************************************************************** */
    /* Section: Constants                                                         */
    /* ************************************************************************** */
    /* ************************************************************************** */





    // *****************************************************************************
    // *****************************************************************************
    // Section: Data Types
    // *****************************************************************************
    // *****************************************************************************





    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************

    void UI_IO_Init();
    void UI_Main_Init(SUCCESS_CALLBACK cb);
    void UI_Tasks();
    void UI_InterruptHandler();


    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* multiple inclusion */

/* *****************************************************************************
 End of File
 */
