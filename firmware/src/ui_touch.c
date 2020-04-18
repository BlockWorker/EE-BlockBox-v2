
#include "ui.h"
/* ************************************************************************** */
#ifdef UI_TOUCH

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "app.h"
#include "FT8_commands.h"
#include "FT8.h"
#include "flash.h"

/*****************************/
/* Variables and definitions */
/*****************************/

SUCCESS_CALLBACK ui_initCallback;

bool ui_interruptReceived = false;
uint8_t ui_touchedTag = 0;
bool ui_touchDown = false;


/*************************/
/* Calibration functions */
/*************************/

void ui_touchmatrix_read_callback(bool success, uint32_t data, uintptr_t context) {
    flash_touch_matrix[context] = data;
    if (context >= 5) {
        FLASH_Write();
    } else {
        FT8_memRead32(REG_TOUCH_TRANSFORM_B + 4 * context, ui_touchmatrix_read_callback, context + 1);
    }
}

void ui_calibrate_callback(bool success, uintptr_t context) {
    FT8_memRead32(REG_TOUCH_TRANSFORM_A, ui_touchmatrix_read_callback, 0);
}

void ui_calibrate() {
    FT8_cmd_begindisplay(true, true, true, 0x000000);
    FT8_cmd_color(0xffffff);
    FT8_cmd_calibrate();
    FT8_cmd_enddisplay();
    FT8_cmd_execute(ui_calibrate_callback, NULL);
}

void ui_writeOrCalibrateTouch() {
    if (flash_touch_matrix[0] == flash_touch_matrix[1] &&
        flash_touch_matrix[1] == flash_touch_matrix[2] &&
        flash_touch_matrix[2] == flash_touch_matrix[3] &&
        flash_touch_matrix[3] == flash_touch_matrix[4] &&
        flash_touch_matrix[4] == flash_touch_matrix[5]) {
        ui_calibrate();
    } else {
        FT8_memWriteBuffer(REG_TOUCH_TRANSFORM_A, (uint8_t*)flash_touch_matrix, 24);
    }
}

/****************************/
/* Screen drawing functions */
/****************************/

void ui_drawMainScreen() {
    uint16_t barSplitX = 16 * 316 - (uint16_t)(batteryPercent * 6.4);
    uint32_t barColor = batteryPercent > 50.0 ? 0x00ff00 : batteryPercent > 20.0 ? 0xffff00 : 0xff0000;
    //char voltage[7] = { 0 };
    //snprintf(voltage, 7, "%5.2fV", batteryVolts);
    
    FT8_cmd_begindisplay(true, true, true, 0x000000);
    
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_start_cmd(VERTEX2F(16 * 270, 16 * 8));
    FT8_start_cmd(VERTEX2F(16 * 274, 16 * 16));
    FT8_start_cmd(VERTEX2F(16 * 274, 16 * 2));
    FT8_start_cmd(VERTEX2F(16 * 318, 16 * 22));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(16 * 276, 16 * 4));
    FT8_start_cmd(VERTEX2F(barSplitX, 16 * 20));
    FT8_cmd_color(barColor);
    FT8_start_cmd(VERTEX2F(barSplitX, 16 * 4));
    FT8_start_cmd(VERTEX2F(16 * 316, 16 * 20));
    FT8_start_cmd(END());
    FT8_cmd_color(0xffffff);
    FT8_cmd_number(150, 2, 27, 0, batteryVoltageCountAverage);
    //FT8_cmd_text(150, 2, 27, 0, voltage);
    
    FT8_cmd_color(0xffffff);
    FT8_cmd_text(160, 70, 29, FT8_OPT_CENTER, "Wait!");
    FT8_cmd_spinner(160, 140, 0, 0);
    FT8_cmd_enddisplay();
    FT8_cmd_start();
}

/****************************/
/* Touch handling functions */
/****************************/

void ui_trackReadCallback(bool success, uint32_t data, uintptr_t context) {
    if (!success) return;
    
    uint16_t tag = data & 0xffff;
    if (tag == 0) return;
    
    //uint16_t value = data >> 16;
    
    switch (tag) {
        
    }
}

void ui_tagReadCallback(bool success, uint8_t data, uintptr_t context) {
    if (!success) return;
    ui_touchedTag = data;
    if (data > 0 && !ui_touchDown) {
        FT8_memWrite8(REG_INT_MASK, 0x84);
        ui_touchDown = true;
    } else if (data == 0 && ui_touchDown) {
        FT8_memWrite8(REG_INT_MASK, 0x04);
        ui_touchDown = false;
    }
    
    switch (data) {
        case 1:
            FT8_memRead32(REG_TRACKER, ui_trackReadCallback, NULL);
            break;
        default:
            break;
    }
}

void UI_InterruptHandler() {
    ui_interruptReceived = true;
}

void UI_Tasks() {
    FT8_Tasks();
    
    if (ui_interruptReceived) {
        FT8_memRead8(REG_INT_FLAGS, NULL, NULL);
        FT8_memRead8(REG_TOUCH_TAG, ui_tagReadCallback, 0);
        ui_interruptReceived = false;
    }
    
    if (batteryUpdated) {
        //ui_drawStatusBar();
        ui_drawMainScreen();
        batteryUpdated = false;
    }
}

/****************************/
/* Initialization functions */
/****************************/

void UI_IO_Init() {
    FT8_IO_Init();
    SYS_INT_SourceEnable(INT_SOURCE_EXTERNAL_4);
}

void ui_initTimeCallback(uintptr_t context) {
    switch (context) {
        case 0:
            
            break;
    }
}

void ui_ftInitCallback(bool success) {
    if (!success) {
       if (ui_initCallback != NULL) ui_initCallback(false);
       return;
    }
    FT8_memWrite8(REG_INT_MASK, 0x04);
    FT8_memWrite8(REG_INT_EN, 0x01);
    FT8_memRead8(REG_INT_FLAGS, NULL, NULL);
    ui_writeOrCalibrateTouch();
    //ui_drawStatusBar();
    ui_drawMainScreen();
    if (ui_initCallback != NULL) ui_initCallback(true);
    
    //SYS_TIME_CallbackRegisterMS(ui_initTimeCallback, 0, 100, SYS_TIME_SINGLE);
}

void UI_Main_Init(SUCCESS_CALLBACK cb) {
    ui_initCallback = cb;
    FT8_init(ui_ftInitCallback);
}

#endif
/* *****************************************************************************
 End of File
 */
