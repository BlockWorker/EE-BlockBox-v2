
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
#include "bm83.h"
#include "dap.h"
#include "ui_touch_draws.h"

/*****************************/
/* Variables and definitions */
/*****************************/

#define UI_TIMEOUT_MAX_DIFF 100000000 //difference (tick - timeoutTick) is compared to this, greater than this means not timed out (very high value means negative difference)

static SUCCESS_CALLBACK ui_initCallback;

//bool ui_interruptReceived = false;
static uint8_t ui_touchedTag = 0;
static bool ui_touchLocked = false;
static bool ui_touchDown = false;
static bool ui_initialTouch = false;
static bool ui_touchReleased = false;

uint32_t ui_themeColor = 0x0040ff;

static uint16_t ui_touchX = 0x8000;
static uint16_t ui_touchY = 0x8000;

static uint32_t ui_nextIntAt;
static uint32_t ui_intPause;

static uint16_t ui_minVolume = 0x110; //-50dB
static uint16_t ui_maxVolume = 0x098; //-20dB
static uint16_t ui_volumeStep = 0x004; //1dB step

/*************************/
/* Calibration functions */
/*************************/

void ui_touchmatrix_read_callback(bool success, uint32_t data, uintptr_t context) {
    flash_touch_matrix[context] = data;
    if (context >= 5) {
        //FLASH_Write();
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
    /*if (flash_touch_matrix[0] == flash_touch_matrix[1] &&
        flash_touch_matrix[1] == flash_touch_matrix[2] &&
        flash_touch_matrix[2] == flash_touch_matrix[3] &&
        flash_touch_matrix[3] == flash_touch_matrix[4] &&
        flash_touch_matrix[4] == flash_touch_matrix[5]) {
        ui_calibrate();
    } else {
        FT8_memWriteBuffer(REG_TOUCH_TRANSFORM_A, (uint8_t*)flash_touch_matrix, 24);
    }*/
    ui_calibrate();
}

/********************/
/* Helper functions */
/********************/

char* ui_volToString(uint16_t vol) {
    static char res[6];
    memset(res, 0, 6);
    uint16_t rvol = vol & 0x03fc; //round and cut volume
    uint16_t rem = vol & 0x3;
    if (rem == 0x3 || (rem == 0x2 && (rvol & 0x4))) rvol += 0x4; //round up if closer to top or (equal and even number above)
    int16_t decibels = 18 - (int16_t)(rvol >> 2);
    if (decibels == 0) return "0dB";
    snprintf(res, 6, "%+ddB", decibels);
    return res;
}

/****************************/
/* Screen drawing functions */
/****************************/

void ui_drawStatusBar() {
    uint16_t barSplitX = 16 * 314 - (uint16_t)(batteryPercent * 6.4);
    char percentText[5] = { 0 };
    snprintf(percentText, 5, "%u%%", batteryPercent);
    uint16_t percentOffset = batteryPercent < 10 ? 20 : batteryPercent < 100 ? 10 : 0;
    //char voltageText[7] = { 0 };
    //snprintf(voltageText, 7, "%5.2fV", batteryVolts);
    
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0x202020);
    FT8_start_cmd(VERTEX2F(0, 0));
    FT8_start_cmd(VERTEX2F(16 * 320, 16 * 24));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(16 * 268, 16 * 8));
    FT8_start_cmd(VERTEX2F(16 * 272, 16 * 16));
    FT8_start_cmd(VERTEX2F(16 * 272, 16 * 3));
    FT8_start_cmd(VERTEX2F(16 * 316, 16 * 21));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(16 * 274, 16 * 5));
    FT8_start_cmd(VERTEX2F(barSplitX, 16 * 19));
    FT8_cmd_color(ui_themeColor);
    FT8_start_cmd(VERTEX2F(barSplitX, 16 * 5));
    FT8_start_cmd(VERTEX2F(16 * 314, 16 * 19));
    FT8_start_cmd(END());
    FT8_cmd_color(0xffffff);
    FT8_cmd_number(120, 2, 27, 0, batteryVoltageCountAverage);
    FT8_cmd_text(222 + percentOffset, 2, 27, 0, percentText);
    //FT8_cmd_text(120, 2, 27, 0, voltageText);
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawMainScreen() {
    FT8_start_cmd(SAVE_CONTEXT());
    //title and artist labels
    FT8_cmd_color(0xffffff);
    FT8_cmd_text(160, 36, 28, FT8_OPT_CENTERX, "Title");
    FT8_cmd_text(160, 67, 28, FT8_OPT_CENTERX, "Artist");
    //player controls
    ui_drawBackButton(60, 101);
    bm83_playing ? ui_drawPauseButton(137, 101) : ui_drawPlayButton(137, 101);
    ui_drawForwardButton(210, 101);
    //progress bar
    FT8_cmd_bgcolor(0x808080);
    FT8_cmd_color(ui_themeColor);
    FT8_cmd_progress(12, 169, 296, 11, 0, 30, 165);
    //position+length labels
    FT8_cmd_color(0xffffff);
    FT8_cmd_text(6, 153, 26, 0, "0:30");
    FT8_cmd_text(314, 153, 26, FT8_OPT_RIGHTX, "2:45");
    //separator lines
    FT8_start_cmd(BEGIN(FT8_LINES));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0x808080);
    FT8_start_cmd(VERTEX2F(129 * 16 + 8, 192 * 16));
    FT8_start_cmd(VERTEX2F(129 * 16 + 8, 230 * 16));
    FT8_start_cmd(VERTEX2F(176 * 16 + 8, 192 * 16));
    FT8_start_cmd(VERTEX2F(176 * 16 + 8, 230 * 16));
    FT8_start_cmd(VERTEX2F(223 * 16 + 8, 192 * 16));
    FT8_start_cmd(VERTEX2F(223 * 16 + 8, 230 * 16));
    FT8_start_cmd(VERTEX2F(270 * 16 + 8, 192 * 16));
    FT8_start_cmd(VERTEX2F(270 * 16 + 8, 230 * 16));
    //+,- buttons
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(12 * 16, 211 * 16));
    FT8_start_cmd(VERTEX2F(38 * 16, 211 * 16));
    FT8_start_cmd(VERTEX2F(94 * 16, 211 * 16));
    FT8_start_cmd(VERTEX2F(120 * 16, 211 * 16));
    FT8_start_cmd(VERTEX2F(107 * 16, 198 * 16));
    FT8_start_cmd(VERTEX2F(107 * 16, 224 * 16));
    FT8_start_cmd(END());
    //volume display = mute button
    ui_drawSpeaker28(54, 188);
    FT8_cmd_text(68, 216, 26, FT8_OPT_CENTERX, dap_muted ? "Mute" : ui_volToString(dap_volume));
    //remaining icons
    ui_drawBluetooth(133, 191);
    ui_drawBulb(180, 191);
    ui_drawCog(227, 191);
    ui_drawPowerOffButton(274, 191);
    //touch boxes (invisible)
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_start_cmd(COLOR_A(0));
    FT8_start_cmd(TAG(10)); //player backwards
    FT8_start_cmd(VERTEX2F(60 * 16, 101 * 16));
    FT8_start_cmd(VERTEX2F(110 * 16, 151 * 16));
    FT8_start_cmd(TAG(11)); //player play/pause
    FT8_start_cmd(VERTEX2F(137 * 16, 101 * 16));
    FT8_start_cmd(VERTEX2F(187 * 16, 151 * 16));
    FT8_start_cmd(TAG(12)); //player forwards
    FT8_start_cmd(VERTEX2F(210 * 16, 101 * 16));
    FT8_start_cmd(VERTEX2F(260 * 16, 151 * 16));
    FT8_start_cmd(TAG(13)); //volume -
    FT8_start_cmd(VERTEX2F(5 * 16, 191 * 16));
    FT8_start_cmd(VERTEX2F(45 * 16, 231 * 16));
    FT8_start_cmd(TAG(14)); //mute/unmute
    FT8_start_cmd(VERTEX2F(46 * 16, 191 * 16));
    FT8_start_cmd(VERTEX2F(86 * 16, 231 * 16));
    FT8_start_cmd(TAG(15)); //volume -
    FT8_start_cmd(VERTEX2F(87 * 16, 191 * 16));
    FT8_start_cmd(VERTEX2F(127 * 16, 231 * 16));
    FT8_start_cmd(TAG(16)); //bluetooth
    FT8_start_cmd(VERTEX2F(133 * 16, 191 * 16));
    FT8_start_cmd(VERTEX2F(173 * 16, 231 * 16));
    FT8_start_cmd(TAG(17)); //light
    FT8_start_cmd(VERTEX2F(180 * 16, 191 * 16));
    FT8_start_cmd(VERTEX2F(220 * 16, 231 * 16));
    FT8_start_cmd(TAG(18)); //settings
    FT8_start_cmd(VERTEX2F(227 * 16, 191 * 16));
    FT8_start_cmd(VERTEX2F(267 * 16, 231 * 16));
    FT8_start_cmd(TAG(19)); //power off
    FT8_start_cmd(VERTEX2F(274 * 16, 191 * 16));
    FT8_start_cmd(VERTEX2F(314 * 16, 231 * 16));
    FT8_start_cmd(END());
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawScreen() {
    FT8_cmd_begindisplay(true, true, true, 0x000000);
    
    ui_drawStatusBar();
    
    switch (bm83_state) {
        case BM83_NOT_INITIALIZED:
            FT8_cmd_color(0xffffff);
            FT8_cmd_text(160, 70, 29, FT8_OPT_CENTER, "Initializing...");
            FT8_cmd_spinner(160, 140, 0, 0);
            break;
        case BM83_OFF:
            ui_drawBigOnButton();
            break;
        default:
            ui_drawMainScreen();
            break;
    }
    
#if 0
    FT8_start_cmd(BEGIN(FT8_POINTS));
    FT8_start_cmd(TAG_MASK(0));
    FT8_start_cmd(POINT_SIZE(16 * 20));
    FT8_cmd_color(0x00ff00);
    FT8_start_cmd(VERTEX2F(16 * ui_touchX, 16 * ui_touchY));
    FT8_start_cmd(END());
#endif    
    
    FT8_cmd_enddisplay();
    FT8_cmd_start();
}

/****************************/
/* Touch handling functions */
/****************************/

void ui_unlockTouch(bool success) {
    ui_touchLocked = false;
    ui_drawScreen();
}

void ui_trackReadCallback(bool success, uint32_t data, uintptr_t context) {
    if (!success) return;
    
    uint16_t tag = data & 0xffff;
    if (tag == 0) return;
    
    //uint16_t value = data >> 16;
    
    switch (tag) {
        
    }
}

void ui_tagReadCallback(bool success, uint8_t data, uintptr_t context) {
    if (success) {
        if (!ui_touchReleased) ui_touchedTag = data;

        switch (ui_touchedTag) {
            case 1: //power on
                if (ui_initialTouch) {
                    ui_touchLocked = true;
                    DAP_StartUp(ui_unlockTouch);
                    BM83_PowerOn(NULL);
                }
                break;
            case 10: //previous
                if (ui_initialTouch && bm83_state == BM83_CONNECTED) {
                    ui_touchLocked = true;
                    BM83_PrevTrack(ui_unlockTouch);
                }
                break;
            case 11: //play/pause
                if (ui_initialTouch && bm83_state == BM83_CONNECTED) {
                    ui_touchLocked = true;
                    BM83_PlayPause(ui_unlockTouch);
                }
                break;
            case 12: //next
                if (ui_initialTouch && bm83_state == BM83_CONNECTED) {
                    ui_touchLocked = true;
                    BM83_NextTrack(ui_unlockTouch);
                }
                break;
            case 13: //volume -
                if (ui_initialTouch && !dap_muted && dap_volume < ui_minVolume) {
                    ui_touchLocked = true;
                    uint16_t newVol = dap_volume + 0x4;
                    DAP_SetVolume(newVol, ui_unlockTouch);
                    if (bm83_abs_vol_supported) {
                        uint8_t absVol = ((uint32_t)(ui_minVolume - newVol) * (uint32_t)0x7F) / (uint32_t)(ui_minVolume - ui_maxVolume);
                        BM83_SetAbsVolume(absVol, NULL);
                    }
                }
                break;
            case 14: //mute/unmute
                if (ui_initialTouch) {
                    ui_touchLocked = true;
                    if (dap_muted) DAP_Unmute(ui_unlockTouch);
                    else DAP_Mute(ui_unlockTouch);
                }
                break;
            case 15: //volume +
                if (ui_initialTouch && !dap_muted && dap_volume < ui_minVolume) {
                    ui_touchLocked = true;
                    uint16_t newVol = dap_volume - 0x4;
                    DAP_SetVolume(newVol, ui_unlockTouch);
                    if (bm83_abs_vol_supported) {
                        uint8_t absVol = ((uint32_t)(ui_minVolume - newVol) * (uint32_t)0x7F) / (uint32_t)(ui_minVolume - ui_maxVolume);
                        BM83_SetAbsVolume(absVol, NULL);
                    }
                }
                break;
            case 16: //pairing
                if (ui_initialTouch) {
                    ui_touchLocked = true;
                    if (bm83_state == BM83_CONNECTED) BM83_DisconnectLink(ui_unlockTouch);
                    else if (bm83_pairing) BM83_ExitPairing(ui_unlockTouch);
                    else BM83_EnterPairing(ui_unlockTouch);
                }
                break;
            case 19: //power off
                if (ui_touchReleased) {
                    ui_touchLocked = true;
                    DAP_ShutDown(ui_unlockTouch);
                    BM83_PowerOff(NULL);
                }
                break;
            default:
                break;
        }

        if (ui_touchReleased) ui_touchedTag = 0;
    }
    ui_initialTouch = false;
    ui_touchReleased = false;
}

void ui_touchCoordReadCallback(bool success, uint32_t data, uintptr_t context) {
    if (!success) return;
    ui_touchX = data >> 16;
    ui_touchY = data & 0xffff;
    if (ui_touchX <= 320 && ui_touchY <= 240) FT8_memRead8(REG_TOUCH_TAG, ui_tagReadCallback, NULL);
    //ui_drawScreen();
}

void ui_touchReadCallback(bool success, uint32_t data, uintptr_t context) {
    if (!success) return;
    bool newTouched = !(data & 0x80000000);
    if (newTouched && !ui_touchDown) {
        FT8_memWrite8(REG_INT_MASK, 0x82);
        ui_touchDown = true;
        ui_initialTouch = true;
    } else if (!newTouched && ui_touchDown) {
        FT8_memWrite8(REG_INT_MASK, 0x02);
        ui_touchDown = false;
        ui_touchReleased = true;
    }
    if (ui_touchDown) FT8_memRead32(REG_TOUCH_TAG_XY, ui_touchCoordReadCallback, NULL);
    else FT8_memRead8(REG_TOUCH_TAG, ui_tagReadCallback, NULL);
}

void ui_doScreenDraw(bool success) {
    ui_drawScreen();
}

void ui_bm83StateChange(BM83_STATE_CHANGE_TYPE changeType) {
    switch (changeType) {
        case BM83_CHANGE_VOLUME:
            __asm__("nop"); //stupid label workaround
            uint16_t newVol = ui_minVolume - ((uint32_t)(ui_minVolume - ui_maxVolume) * (uint32_t)(bm83_abs_vol)) / (uint32_t)0x7F;
            uint16_t mod = newVol % ui_volumeStep;
            if (mod != 0) {
                newVol -= mod;
                if (mod > ui_volumeStep / 2) newVol += ui_volumeStep;
            }
            DAP_SetVolume(newVol, ui_doScreenDraw);
            break;
        default: break;
    }
    
    ui_drawScreen();
}

void ui_touchSafetyUnlock(uintptr_t context) {
    ui_touchLocked = false;
}

/*void UI_InterruptHandler() {
    ui_interruptReceived = true;
}*/

void UI_Tasks() {
    FT8_Tasks();
    
    uint32_t tick = SYS_TIME_CounterGet();
    
    if (tick - ui_nextIntAt < UI_TIMEOUT_MAX_DIFF) {
        if (!GPIO_PinRead(GPIO_PIN_RC4) && !ui_touchLocked) {
            FT8_memRead8(REG_INT_FLAGS, NULL, NULL);
            FT8_memRead32(REG_TOUCH_DIRECT_XY, ui_touchReadCallback, NULL);
            ui_nextIntAt = tick + ui_intPause;
        } else {
            ui_nextIntAt = tick;
        }
    }
    
    
    if (batteryUpdated) {
        ui_drawScreen();
        batteryUpdated = false;
    }
}

/****************************/
/* Initialization functions */
/****************************/

void UI_IO_Init() {
    FT8_IO_Init();
    //SYS_INT_SourceEnable(INT_SOURCE_EXTERNAL_4);
    
    ui_intPause = SYS_TIME_MSToCount(10);
}

void ui_ftInitCallback(bool success) {
    if (!success) {
       if (ui_initCallback != NULL) ui_initCallback(false);
       return;
    }
    FT8_memWrite8(REG_INT_MASK, 0x02);
    FT8_memWrite8(REG_INT_EN, 0x01);
    FT8_memRead8(REG_INT_FLAGS, NULL, NULL);
    ui_writeOrCalibrateTouch();
    ui_drawScreen();
    BM83_SetStateChangeCallback(ui_bm83StateChange);
    if (ui_initCallback != NULL) ui_initCallback(true);
    
    ui_nextIntAt = SYS_TIME_CounterGet();
    
    SYS_TIME_CallbackRegisterMS(ui_touchSafetyUnlock, 0, 5000, SYS_TIME_PERIODIC);
}

void UI_Main_Init(SUCCESS_CALLBACK cb) {
    ui_initCallback = cb;
    FT8_init(ui_ftInitCallback);
}

#endif
/* *****************************************************************************
 End of File
 */
