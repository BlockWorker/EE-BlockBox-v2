/* ************************************************************************** */
#ifndef UI_TOUCH

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "char_lcd.h"
#include "app.h"
#include "ui.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

#define CHAR_LCD_QUEUE_LEN 128

uint16_t queue[CHAR_LCD_QUEUE_LEN];
uint16_t queueWritePtr = 0;
uint16_t queueReadPtr = 0;
bool busySending = false;

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

void charlcd_queueWrite(bool rs, uint8_t data, bool longdelay) {
    uint16_t queueVal = (longdelay << 9) | (rs << 8) | data;
    queue[queueWritePtr++] = queueVal;
    queueWritePtr %= CHAR_LCD_QUEUE_LEN;
}

inline void charlcd_setup_data(uint8_t data) {
    LCD_D0_Put(data & 1);
    data >>= 1;
    LCD_D1_Put(data & 1);
    data >>= 1;
    LCD_D2_Put(data & 1);
    data >>= 1;
    LCD_D3_Put(data & 1);
    data >>= 1;
    LCD_D4_Put(data & 1);
    data >>= 1;
    LCD_D5_Put(data & 1);
    data >>= 1;
    LCD_D6_Put(data & 1);
    data >>= 1;
    LCD_D7_Put(data & 1);
}

void charlcd_write_callback(uintptr_t context) {
    uint16_t val = queue[queueReadPtr];
    switch (context) {
        case 0:
        {
            busySending = true;
            LCD_E_N_Set();
            LCD_RW_Clear();
            if (val & 0x10) LCD_RS_Set();
            else LCD_RS_Clear();
            charlcd_setup_data(val & 0xFF);
            SYS_TIME_CallbackRegisterUS(charlcd_write_callback, 1, 10, SYS_TIME_SINGLE);
            break;
        }
        case 1:
        {
            LCD_E_N_Clear();
            SYS_TIME_CallbackRegisterUS(charlcd_write_callback, 2, 10, SYS_TIME_SINGLE);
            break;
        }
        case 2:
        {
            LCD_E_N_Set();
            if (val & 0x20) SYS_TIME_CallbackRegisterMS(charlcd_write_callback, 3, 2, SYS_TIME_SINGLE);
            else SYS_TIME_CallbackRegisterUS(charlcd_write_callback, 3, 50, SYS_TIME_SINGLE);
            break;
        }
        case 3:
        {
            queueReadPtr += 1;
            queueReadPtr %= CHAR_LCD_QUEUE_LEN;
            if (queueReadPtr != queueWritePtr) charlcd_write_callback(0);
            else busySending = false;
            break;
        }
    }
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

void CharLCD_Init(bool inc, bool shift, bool cursor, bool cursorblink, bool on) {
    charlcd_queueWrite(false, 0b00110011, false);
    charlcd_queueWrite(false, 0b00110011, false);
    charlcd_queueWrite(false, 0b00110011, false); //data length init
    charlcd_queueWrite(false, 0b00111011, false); //lines init
    charlcd_queueWrite(false, 0b00000100 | (inc << 1) | shift, false); //entry mode
    charlcd_queueWrite(false, 0b00001000 | (on << 2) | (cursor << 1) | cursorblink , false); //display control
    charlcd_queueWrite(false, 0b00000001, true); //clear
    if (!busySending) charlcd_write_callback(0);
}

void CharLCD_Clear() {
    charlcd_queueWrite(false, 0b00000001, true);
}

void CharLCD_Home() {
    charlcd_queueWrite(false, 0b00000010, true);
}

void CharLCD_Set_Char_Addr(uint8_t addr) {
    charlcd_queueWrite(false, 0b01000000 | (addr & 0b00111111), false);
}

void CharLCD_Set_Data_Addr(uint8_t addr) {
    charlcd_queueWrite(false, 0b10000000 | (addr & 0b01111111), false);
}

void CharLCD_Write_Byte(uint8_t data) {
    charlcd_queueWrite(true, data, false);
}

void CharLCD_Write_Data(uint8_t* data, uint8_t offset, uint8_t length) {
    if (length > CHAR_LCD_QUEUE_LEN) length = CHAR_LCD_QUEUE_LEN;
    uint8_t i;
    for (i = 0; i < length; i++) {
        charlcd_queueWrite(true, data[i + offset], false);
    }
}

void CharLCD_Print(char* string) {
    uint8_t i = 0;
    while (string[i] != 0 && i < CHAR_LCD_QUEUE_LEN) {
        charlcd_queueWrite(true, string[i++], false);
    }
}

void CharLCD_Tasks() {
    if (!busySending && queueReadPtr < queueWritePtr) charlcd_write_callback(0);
}


#endif
/* *****************************************************************************
 End of File
 */
