/* ************************************************************************** */


#ifndef _CHAR_LCD_H    /* Guard against multiple inclusion */
#define _CHAR_LCD_H

#ifndef UI_TOUCH

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include <stdbool.h>
#include <stdint.h>


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

    void CharLCD_Init(bool inc, bool shift, bool cursor, bool cursorblink, bool on);
    void CharLCD_Clear();
    void CharLCD_Home();
    void CharLCD_Set_Char_Addr(uint8_t addr);
    void CharLCD_Set_Data_Addr(uint8_t addr);
    void CharLCD_Write_Byte(uint8_t data);
    void CharLCD_Write_Data(uint8_t* data, uint8_t offset, uint8_t length);
    void CharLCD_Print(char* string);
    
    void CharLCD_Tasks();


    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif //UI_TOUCH

#endif /* multiple inclusion */

/* *****************************************************************************
 End of File
 */
