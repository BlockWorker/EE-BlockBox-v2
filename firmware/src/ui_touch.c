
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
#include "light.h"
#include "ui_touch_draws.h"
#include "system/time/src/sys_time_local.h"

/*****************************/
/* Variables and definitions */
/*****************************/

#define UI_TIMEOUT_MAX_DIFF 100000000 //difference (tick - timeoutTick) is compared to this, greater than this means not timed out (very high value means negative difference)
#define UI_THEME_COLOR_BLUE 0x0040ff
#define UI_THEME_COLOR_RED 0xb40000
#define UI_THEME_COLOR_YELLOW 0xff6a00
#define UI_THEME_COLOR_GREEN 0x007f0e

typedef enum {
    UI_NO_SETTINGS = 0,
    UI_VOL_SETTINGS,
    UI_FX_SETTINGS,
    UI_DISP_SETTINGS,
    UI_LIGHT_SETTINGS,
    UI_POWER_SETTINGS,
    UI_LINK_SETTINGS
} UI_SETTINGS_TYPE;

static SUCCESS_CALLBACK ui_initCallback;

//bool ui_interruptReceived = false;
static uint8_t ui_touchedTag = 0;
static bool ui_touchLocked = false;
static bool ui_touchDown = false;
static bool ui_initialTouch = false;
static bool ui_touchReleased = false;
static bool ui_longTouch = false;
static bool ui_longTouchTick = false;
static uint32_t ui_nextTouchTickAt;
static uint32_t ui_touchTickPause;

static bool ui_screenUpdateNeeded = false;
static bool ui_screenUpdating = false;
static uint32_t ui_nextScreenUpdateAt;
static uint32_t ui_screenUpdatePause;

uint32_t ui_themeColor = 0x0040ff;

static uint8_t ui_screenBrightness = 64;

static UI_SETTINGS_TYPE ui_settingsType = UI_NO_SETTINGS;

static uint16_t ui_touchX = 0x8000;
static uint16_t ui_touchY = 0x8000;

static uint32_t ui_nextIntAt;
static uint32_t ui_intPause;

static bool ui_screenActive = true;
static bool ui_screenActivateRequested = false;
static bool ui_screenDeactivateRequested = false;
static uint8_t ui_screenFadeBrightness = 64;
static uint8_t ui_screenFadeStep;
static uint32_t ui_screenFadeStepTime;
static uint32_t ui_nextScreenFadeStepAt;
static uint32_t ui_screenDeactivateAt;
static uint32_t ui_screenDeactivateDelay;

static uint32_t ui_fadeoutDelays[] = { 495000000U, 990000000U, 1980000000U, 2970000000U, 0U };
static const char* ui_fadeoutDelayNames[] = { "5 s", "10 s", "20 s", "30 s", "Never" };

static uint16_t ui_minVolume = 0x110; //-50dB
static uint16_t ui_maxVolume = 0x098; //-20dB
static uint16_t ui_volumeStep = 0x008; //2dB step

/*************************/
/* Calibration functions */
/*************************/

void ui_touchmatrix_read_callback(bool success, uint32_t data, uintptr_t context) {
    flash_touch_matrix[context] = data;
    if (context >= 5) {
        FLASH_Write();
        ui_screenUpdateNeeded = true;
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

void ui_setBrightnessCallback(bool success, uintptr_t context) {
    if (success) {
        ui_screenFadeBrightness = (uint8_t)context;
        if (ui_screenActive) ui_screenBrightness = ui_screenFadeBrightness;
    }
}

void ui_setScreenBrightness(uint8_t brightness) {
    if (brightness == ui_screenFadeBrightness) return;
    FT8_memWrite8C(REG_PWM_DUTY, brightness, ui_setBrightnessCallback, brightness);
}

uint8_t ui_getFadeoutDelayIndex(int64_t delay) {
    int i;
    for (i = 1; i < 5; i++) {
        if (ui_screenDeactivateDelay == ui_fadeoutDelays[i]) return i;
    }
    return 0;
}

/****************************/
/* Screen drawing functions */
/****************************/

void ui_drawStatusBar() {
    //uint16_t barSplitX = 16 * 314 - (uint16_t)(batteryPercent * 6.4);
    char percentText[5] = { 0 };
    snprintf(percentText, 5, "%u%%", batteryPercent);
    //uint16_t percentOffset = batteryPercent < 10 ? 20 : batteryPercent < 100 ? 10 : 0;
    char voltageText[7] = { 0 };
    snprintf(voltageText, 7, "%5.2fV", batteryVolts);
    
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0x303030);
    FT8_start_cmd(VERTEX2F(0, 0));
    FT8_start_cmd(VERTEX2F(16 * 320, 16 * 24));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(16 * 268, 16 * 8));
    FT8_start_cmd(VERTEX2F(16 * 272, 16 * 16));
    FT8_start_cmd(VERTEX2F(16 * 272, 16 * 3));
    FT8_start_cmd(VERTEX2F(16 * 316, 16 * 21));
    /*if (batteryDataValid) {
        FT8_cmd_color(0x000000);
        FT8_start_cmd(VERTEX2F(16 * 274, 16 * 5));
        FT8_start_cmd(VERTEX2F(barSplitX, 16 * 19));
        FT8_cmd_color(ui_themeColor);
        FT8_start_cmd(VERTEX2F(barSplitX, 16 * 5));
        FT8_start_cmd(VERTEX2F(16 * 314, 16 * 19));
    } else {*/
        FT8_cmd_color(0x808080);
        FT8_start_cmd(VERTEX2F(16 * 274, 16 * 5));
        FT8_start_cmd(VERTEX2F(16 * 314, 16 * 19));
    //}
    FT8_start_cmd(END());
    FT8_cmd_color(0xffffff);
    FT8_cmd_number(40, 2, 27, 0, batteryVoltageCountAverage);
    //FT8_cmd_number(160, 2, 27, 0, batteryCurrentCountAverage);
    FT8_cmd_text(120, 2, 27, 0, voltageText);
    /*if (batteryDataValid) FT8_cmd_text(265, 2, 27, FT8_OPT_RIGHTX, percentText);
    else*/ FT8_cmd_text(265, 2, 27, FT8_OPT_RIGHTX, "---%");
    //FT8_cmd_text(120, 2, 27, 0, voltageText);
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawMainScreen() {
    FT8_start_cmd(SAVE_CONTEXT());
    //title and artist labels
    FT8_cmd_color(0xffffff);
    //FT8_cmd_text(160, 36, 28, FT8_OPT_CENTERX, "Title");
    //FT8_cmd_text(160, 67, 28, FT8_OPT_CENTERX, "Artist");
    FT8_cmd_text(160, 45, 29, FT8_OPT_CENTERX, "BlockBox v2");
    //player controls
    ui_drawBackButton(60, 101);
    bm83_playing ? ui_drawPauseButton(137, 101) : ui_drawPlayButton(137, 101);
    ui_drawForwardButton(210, 101);
    //progress bar
    FT8_cmd_bgcolor(0x808080);
    FT8_cmd_color(ui_themeColor);
    FT8_cmd_progress(12, 169, 296, 11, 0, 165, 165);
    //position+length labels
    FT8_cmd_color(0xffffff);
    FT8_cmd_text(6, 153, 26, 0, "-:--");
    FT8_cmd_text(314, 153, 26, FT8_OPT_RIGHTX, "-:--");
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
    ui_drawBulb(180, 191, 0x000000, light_on);
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

void ui_drawSettingsTopBar() {
    FT8_start_cmd(SAVE_CONTEXT());
    //selected menu rectangle
    uint16_t rectX;
    switch (ui_settingsType) {
        case UI_VOL_SETTINGS:
            rectX = 12 * 16;
            break;
        case UI_FX_SETTINGS:
            rectX = 57 * 16;
            break;
        case UI_DISP_SETTINGS:
            rectX = 102 * 16;
            break;
        case UI_LIGHT_SETTINGS:
            rectX = 147 * 16;
            break;
        case UI_POWER_SETTINGS:
            rectX = 192 * 16;
            break;
        case UI_LINK_SETTINGS:
            rectX = 237 * 16;
            break;
        default:
            rectX = 12 * 16;
            break;
    }
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(80));
    FT8_cmd_color(ui_themeColor);
    FT8_start_cmd(VERTEX2F(rectX, 36 * 16));
    FT8_start_cmd(VERTEX2F(rectX + 26 * 16, 62 * 16));
    FT8_start_cmd(END());
    //sub-op icons
    ui_drawSpeaker40(6, 29, rectX == 12 * 16 ? ui_themeColor : 0x000000);
    ui_drawBulb(140, 29, rectX == 147 * 16 ? ui_themeColor : 0x000000, true);
    ui_drawLightning(185, 29);
    ui_drawChain(230, 29, rectX == 237 * 16 ? ui_themeColor : 0x000000);
    //screen icon border
    FT8_start_cmd(BEGIN(FT8_LINE_STRIP));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(101 * 16 + 8, 36 * 16 + 8));
    FT8_start_cmd(VERTEX2F(128 * 16 + 8, 36 * 16 + 8));
    FT8_start_cmd(VERTEX2F(128 * 16 + 8, 56 * 16 + 8));
    FT8_start_cmd(VERTEX2F(101 * 16 + 8, 56 * 16 + 8));
    FT8_start_cmd(VERTEX2F(101 * 16 + 8, 36 * 16 + 8));
    FT8_start_cmd(END());
    //fx icon points
    FT8_start_cmd(BEGIN(FT8_POINTS));
    FT8_start_cmd(POINT_SIZE(64));
    FT8_start_cmd(VERTEX2F(60 * 16, 57 * 16));
    FT8_start_cmd(VERTEX2F(70 * 16, 41 * 16));
    FT8_start_cmd(VERTEX2F(80 * 16, 49 * 16));
    FT8_start_cmd(END());
    //fx icon lines
    FT8_start_cmd(BEGIN(FT8_LINES));
    FT8_start_cmd(VERTEX2F(60 * 16, 35 * 16));
    FT8_start_cmd(VERTEX2F(60 * 16, 63 * 16));
    FT8_start_cmd(VERTEX2F(70 * 16, 35 * 16));
    FT8_start_cmd(VERTEX2F(70 * 16, 63 * 16));
    FT8_start_cmd(VERTEX2F(80 * 16, 35 * 16));
    FT8_start_cmd(VERTEX2F(80 * 16, 63 * 16));
    //screen icon lines
    FT8_start_cmd(VERTEX2F(108 * 16, 61 * 16 + 8));
    FT8_start_cmd(VERTEX2F(122 * 16, 61 * 16 + 8));
    FT8_start_cmd(VERTEX2F(112 * 16 + 8, 56 * 16 + 8));
    FT8_start_cmd(VERTEX2F(112 * 16 + 8, 61 * 16 + 8));
    FT8_start_cmd(VERTEX2F(117 * 16 + 8, 56 * 16 + 8));
    FT8_start_cmd(VERTEX2F(117 * 16 + 8, 61 * 16 + 8));
    //close icon lines
    FT8_start_cmd(VERTEX2F(283 * 16, 37 * 16));
    FT8_start_cmd(VERTEX2F(307 * 16, 61 * 16));
    FT8_start_cmd(VERTEX2F(283 * 16, 61 * 16));
    FT8_start_cmd(VERTEX2F(307 * 16, 37 * 16));
    //separator lines
    FT8_cmd_color(0x808080);
    FT8_start_cmd(VERTEX2F(47 * 16 + 8, 30 * 16));
    FT8_start_cmd(VERTEX2F(47 * 16 + 8, 68 * 16));
    FT8_start_cmd(VERTEX2F(92 * 16 + 8, 30 * 16));
    FT8_start_cmd(VERTEX2F(92 * 16 + 8, 68 * 16));
    FT8_start_cmd(VERTEX2F(137 * 16 + 8, 30 * 16));
    FT8_start_cmd(VERTEX2F(137 * 16 + 8, 68 * 16));
    FT8_start_cmd(VERTEX2F(182 * 16 + 8, 30 * 16));
    FT8_start_cmd(VERTEX2F(182 * 16 + 8, 68 * 16));
    FT8_start_cmd(VERTEX2F(227 * 16 + 8, 30 * 16));
    FT8_start_cmd(VERTEX2F(227 * 16 + 8, 68 * 16));
    FT8_start_cmd(VERTEX2F(272 * 16 + 8, 30 * 16));
    FT8_start_cmd(VERTEX2F(272 * 16 + 8, 68 * 16));
    FT8_start_cmd(END());
    //touch boxes (invisible)
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_start_cmd(COLOR_A(0));
    FT8_start_cmd(TAG(30)); //volume tab
    FT8_start_cmd(VERTEX2F(5 * 16, 29 * 16));
    FT8_start_cmd(VERTEX2F(45 * 16, 69 * 16));
    FT8_start_cmd(TAG(31)); //fx tab
    FT8_start_cmd(VERTEX2F(50 * 16, 29 * 16));
    FT8_start_cmd(VERTEX2F(90 * 16, 69 * 16));
    FT8_start_cmd(TAG(32)); //disp tab
    FT8_start_cmd(VERTEX2F(95 * 16, 29 * 16));
    FT8_start_cmd(VERTEX2F(135 * 16, 69 * 16));
    FT8_start_cmd(TAG(33)); //light tab
    FT8_start_cmd(VERTEX2F(140 * 16, 29 * 16));
    FT8_start_cmd(VERTEX2F(180 * 16, 69 * 16));
    FT8_start_cmd(TAG(34)); //power tab
    FT8_start_cmd(VERTEX2F(185 * 16, 29 * 16));
    FT8_start_cmd(VERTEX2F(225 * 16, 69 * 16));
    FT8_start_cmd(TAG(35)); //link tab
    FT8_start_cmd(VERTEX2F(230 * 16, 29 * 16));
    FT8_start_cmd(VERTEX2F(270 * 16, 69 * 16));
    FT8_start_cmd(TAG(39)); //close settings
    FT8_start_cmd(VERTEX2F(275 * 16, 29 * 16));
    FT8_start_cmd(VERTEX2F(315 * 16, 69 * 16));
    FT8_start_cmd(END());
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawVolSettings() {
    ui_drawSettingsTopBar();
    FT8_start_cmd(SAVE_CONTEXT());
    //controls
    FT8_cmd_fgcolor(ui_themeColor);
    FT8_cmd_bgcolor(0x808080);
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(TAG(40)); //volume sync switch
    FT8_cmd_toggle(216, 92, 30, 26, 0, bm83_abs_vol_supported ? 0xffff : 0, "No\xffYes");
    FT8_start_cmd(TAG(41)); //min volume slider
    FT8_cmd_slider(57, 138, 205, 11, 0, 15 - ((ui_minVolume - 0x0C0) / ui_volumeStep), 15); //TODO: calc is temporary
    FT8_cmd_track(52, 133, 215, 21, 41);
    FT8_start_cmd(TAG(42)); //max volume slider
    FT8_cmd_slider(57, 175, 205, 11, 0, 15 - ((ui_maxVolume - 0x048) / ui_volumeStep), 15); //TODO: calc is temporary
    FT8_cmd_track(52, 170, 215, 21, 42);
    FT8_start_cmd(TAG(43)); //volume step slider
    FT8_cmd_slider(57, 212, 205, 11, 0, 4, 5); //TODO: implement
    FT8_cmd_track(52, 207, 215, 21, 43);
    //text
    FT8_start_cmd(TAG(0));
    FT8_cmd_text(190, 97, 27, FT8_OPT_CENTERY | FT8_OPT_RIGHTX, "Sync with phone");
    FT8_cmd_text(40, 143, 27, FT8_OPT_CENTERY | FT8_OPT_RIGHTX, "Min");
    FT8_cmd_text(40, 180, 27, FT8_OPT_CENTERY | FT8_OPT_RIGHTX, "Max");
    FT8_cmd_text(40, 217, 27, FT8_OPT_CENTERY | FT8_OPT_RIGHTX, "Step");
    FT8_cmd_text(277, 143, 26, FT8_OPT_CENTERY, ui_volToString(ui_minVolume));
    FT8_cmd_text(277, 180, 26, FT8_OPT_CENTERY, ui_volToString(ui_maxVolume));
    FT8_cmd_text(277, 217, 26, FT8_OPT_CENTERY, "2dB"); //TODO: implement
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawFxSettings() {
    ui_drawSettingsTopBar();
    FT8_start_cmd(SAVE_CONTEXT());
    //controls
    FT8_cmd_fgcolor(ui_themeColor);
    FT8_cmd_bgcolor(0x808080);
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(TAG(50)); //eq mode switch
    FT8_cmd_toggle(225, 90, 30, 26, 0, 0, "HiFi\xffPower");
    FT8_start_cmd(TAG(51)); //loudness slider
    char percentText[5] = { 0 };
    snprintf(percentText, 5, "%u%%", dap_loudnessPercent);
    FT8_cmd_slider(78, 128, 178, 11, 0, dap_loudnessPercent, 100);
    FT8_cmd_track(72, 123, 188, 21, 51);
    FT8_start_cmd(TAG(52)); //bass slider
    FT8_cmd_slider(66, 165, 190, 11, 0, 10, 20);
    FT8_cmd_track(61, 160, 200, 21, 52);
    FT8_start_cmd(TAG(53)); //treble slider
    FT8_cmd_slider(66, 202, 190, 11, 0, 10, 20);
    FT8_cmd_track(61, 197, 200, 21, 53);
    //text
    FT8_start_cmd(TAG(0));
    FT8_cmd_text(205, 95, 27, FT8_OPT_CENTERY | FT8_OPT_RIGHTX, "EQ Mode");
    FT8_cmd_text(64, 133, 26, FT8_OPT_CENTERY | FT8_OPT_RIGHTX, "Loudness");
    FT8_cmd_text(52, 170, 27, FT8_OPT_CENTERY | FT8_OPT_RIGHTX, "Bass");
    FT8_cmd_text(52, 207, 27, FT8_OPT_CENTERY | FT8_OPT_RIGHTX, "Treble");
    FT8_cmd_text(270, 170, 26, FT8_OPT_CENTERY, "0dB");
    FT8_cmd_text(270, 207, 26, FT8_OPT_CENTERY, "0dB");
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawDispSettings() {
    ui_drawSettingsTopBar();
    FT8_start_cmd(SAVE_CONTEXT());
    //sliders
    FT8_cmd_fgcolor(ui_themeColor);
    FT8_cmd_bgcolor(0x808080);
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(TAG(60)); //disp brightness slider
    FT8_cmd_slider(66, 88, 190, 11, 0, ui_screenBrightness < 10 ? 0 : ui_screenBrightness - 10, 117);
    FT8_cmd_track(61, 83, 200, 21, 60);
    FT8_start_cmd(TAG(61)); //sleep delay slider
    uint8_t delayIndex = ui_getFadeoutDelayIndex(ui_screenDeactivateDelay);
    FT8_cmd_slider(66, 125, 190, 11, 0, delayIndex, 4);
    FT8_cmd_track(61, 122, 200, 21, 61);
    //text
    FT8_start_cmd(TAG(0));
    ui_drawLightIcon(11, 74);
    FT8_cmd_text(52, 130, 27, FT8_OPT_CENTERY | FT8_OPT_RIGHTX, "Sleep");
    FT8_cmd_text(275, 132, 26, FT8_OPT_CENTERY, ui_fadeoutDelayNames[delayIndex]);
    FT8_cmd_text(112, 166, 27, FT8_OPT_CENTER, "Theme");
    FT8_cmd_text(274, 166, 27, FT8_OPT_CENTER, "Touch");
    //button backgrounds
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(64));
    FT8_start_cmd(VERTEX2F(18 * 16, 190 * 16));
    FT8_start_cmd(VERTEX2F(50 * 16, 222 * 16));
    FT8_start_cmd(VERTEX2F(72 * 16, 190 * 16));
    FT8_start_cmd(VERTEX2F(104 * 16, 222 * 16));
    FT8_start_cmd(VERTEX2F(126 * 16, 190 * 16));
    FT8_start_cmd(VERTEX2F(158 * 16, 222 * 16));
    FT8_start_cmd(VERTEX2F(180 * 16, 190 * 16));
    FT8_start_cmd(VERTEX2F(212 * 16, 222 * 16));
    FT8_start_cmd(LINE_WIDTH(96));
    FT8_start_cmd(VERTEX2F(242 * 16, 192 * 16));
    FT8_start_cmd(VERTEX2F(308 * 16, 220 * 16));
    //button contents
    FT8_start_cmd(LINE_WIDTH(48));
    FT8_start_cmd(TAG(62)); //blue theme button
    FT8_cmd_color(UI_THEME_COLOR_BLUE);
    FT8_start_cmd(VERTEX2F(18 * 16, 190 * 16));
    FT8_start_cmd(VERTEX2F(50 * 16, 222 * 16));
    FT8_start_cmd(TAG(63)); //red theme button
    FT8_cmd_color(UI_THEME_COLOR_RED);
    FT8_start_cmd(VERTEX2F(72 * 16, 190 * 16));
    FT8_start_cmd(VERTEX2F(104 * 16, 222 * 16));
    FT8_start_cmd(TAG(64)); //yellow theme button
    FT8_cmd_color(UI_THEME_COLOR_YELLOW);
    FT8_start_cmd(VERTEX2F(126 * 16, 190 * 16));
    FT8_start_cmd(VERTEX2F(158 * 16, 222 * 16));
    FT8_start_cmd(TAG(65)); //green theme button
    FT8_cmd_color(UI_THEME_COLOR_GREEN);
    FT8_start_cmd(VERTEX2F(180 * 16, 190 * 16));
    FT8_start_cmd(VERTEX2F(212 * 16, 222 * 16));
    FT8_start_cmd(TAG(66)); //calibrate button
    FT8_cmd_color(0x000000);
    FT8_start_cmd(LINE_WIDTH(64));
    FT8_start_cmd(VERTEX2F(242 * 16, 192 * 16));
    FT8_start_cmd(VERTEX2F(308 * 16, 220 * 16));
    FT8_start_cmd(END());
    //calibrate text
    FT8_cmd_color(0xffffff);
    FT8_cmd_text(275, 206, 27, FT8_OPT_CENTER, "Calibrate");
    //divider line
    FT8_start_cmd(TAG(0));
    FT8_start_cmd(BEGIN(FT8_LINES));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0x808080);
    FT8_start_cmd(VERTEX2F(228 * 16, 155 * 16));
    FT8_start_cmd(VERTEX2F(228 * 16, 230 * 16));
    FT8_start_cmd(END());
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawLightSettings() {
    ui_drawSettingsTopBar();
    FT8_start_cmd(SAVE_CONTEXT());
    //controls
    FT8_cmd_fgcolor(ui_themeColor);
    FT8_cmd_bgcolor(0x808080);
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(TAG(70)); //light switch
    FT8_cmd_toggle(185, 90, 30, 26, 0, light_on ? 0xffff : 0, "Off\xffOn");
    FT8_start_cmd(TAG(71)); //s2l switch
    FT8_cmd_toggle(185, 134, 30, 26, 0, light_s2l ? 0xffff : 0, "Off\xffOn");
    FT8_start_cmd(TAG(72)); //light brightness slider
    FT8_cmd_slider(67, 192, 220, 11, 0, (uint16_t)(light_brightness * 400.f), 100);
    FT8_cmd_track(62, 187, 230, 21, 72);
    //text
    FT8_start_cmd(TAG(0));
    FT8_cmd_text(165, 95, 27, FT8_OPT_CENTERY | FT8_OPT_RIGHTX, "Light");
    FT8_cmd_text(165, 149, 27, FT8_OPT_CENTERY | FT8_OPT_RIGHTX, "Sound-to-light");
    ui_drawLightIcon(9, 177);
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawPowerSettings() {
    ui_drawSettingsTopBar();
    FT8_start_cmd(SAVE_CONTEXT());
    
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawLinkSettings() {
    ui_drawSettingsTopBar();
    FT8_start_cmd(SAVE_CONTEXT());
    
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_screenDrawFallbackCb(uintptr_t context) {
    ui_screenUpdating = false;
}

void ui_screenDrawFinishCb(bool success, uintptr_t context) {
    ui_screenUpdating = false;
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
            switch (ui_settingsType) {
                case UI_VOL_SETTINGS:
                    ui_drawVolSettings();
                    break;
                case UI_FX_SETTINGS:
                    ui_drawFxSettings();
                    break;
                case UI_DISP_SETTINGS:
                    ui_drawDispSettings();
                    break;
                case UI_LIGHT_SETTINGS:
                    ui_drawLightSettings();
                    break;
                default:
                    ui_drawMainScreen();
                    break;
            }
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
    //ui_screenUpdating = true;
    //SYS_TIME_CallbackRegisterMS(ui_screenDrawFallbackCb, 0, 100, SYS_TIME_SINGLE);
    //FT8_cmd_execute(ui_screenDrawFinishCb, NULL);
    FT8_cmd_start();
}

/****************************/
/* Touch handling functions */
/****************************/

void ui_unlockTouch(bool success) {
    ui_touchLocked = false;
    ui_screenUpdateNeeded = true;
}

//requires both power up/power down commands to go through before unlocking touch and redrawing
void ui_powerUpDownLock(bool success) {
    static bool triggeredOnce = false;
    if (triggeredOnce) {
        triggeredOnce = false;
        ui_unlockTouch(true);
    } else {
        triggeredOnce = true;
    }
}

void ui_setVol(uint16_t newVol, SUCCESS_CALLBACK cb) {
    if (newVol != dap_volume) DAP_SetVolume(newVol, cb);
    if (bm83_abs_vol_supported) {
        uint8_t absVol = ((uint32_t)(ui_minVolume - newVol) * (uint32_t)0x7F) / (uint32_t)(ui_minVolume - ui_maxVolume);
        BM83_SetAbsVolume(absVol, NULL);
    }
}

void ui_trackReadCallback(bool success, uint32_t data, uintptr_t context) {
    if (!success) return;
    
    uint16_t tag = data & 0xffff;
    uint32_t value = data >> 16;
    
    switch (tag) {
        case 41:
            value = value * 16 / 0xffff;
            if (value > 15) value = 15;
            ui_minVolume = 0x0C0 + ui_volumeStep * (15 - value);
            uint16_t newVol1 = dap_volume;
            if (newVol1 > ui_minVolume) newVol1 = ui_minVolume;
            ui_setVol(newVol1, NULL);
            break;
        case 42:
            value = value * 16 / 0xffff;
            if (value > 15) value = 15;
            ui_maxVolume = 0x048 + ui_volumeStep * (15 - value);
            uint16_t newVol2 = dap_volume;
            if (newVol2 < ui_maxVolume) newVol2 = ui_maxVolume;
            ui_setVol(newVol2, NULL);
            break;
        case 51:
            value = value * 101 / 0xffff;
            if (value > 100) value = 100;
            DAP_SetLoudnessComp(value, NULL);
            break;
        case 60:
            value = value * 118 / 0xffff;
            if (value > 117) value = 117;
            ui_setScreenBrightness(value + 10);
            break;
        case 61:
            value = value * 5 / 0xffff;
            if (value > 4) value = 4;
            ui_screenDeactivateDelay = ui_fadeoutDelays[value];
            ui_screenDeactivateAt = SYS_TIME_CounterGet() + ui_screenDeactivateDelay;
            break;
        case 72:
            value = value * 101 / 0xffff;
            if (value > 100) value = 100;
            light_brightness = (float)value / 400.f;
            break;
        default:
            ui_touchLocked = false;
            return;
    }
    
    ui_touchLocked = false;
    ui_screenUpdateNeeded = true;
}

void ui_tagReadCallback(bool success, uint8_t data, uintptr_t context) {
    if (success) {
        uint64_t tick = SYS_TIME_CounterGet();
        
        ui_screenDeactivateAt = tick + ui_screenDeactivateDelay;
        
        if (!ui_touchReleased) {            
            if (ui_touchedTag != data) {
                ui_longTouch = false;
                ui_longTouchTick = false;
                ui_nextTouchTickAt = tick + 3 * ui_touchTickPause;
            } else if (tick - ui_nextTouchTickAt < UI_TIMEOUT_MAX_DIFF) {
                ui_longTouch = true;
                ui_longTouchTick = true;
                ui_nextTouchTickAt = tick + ui_touchTickPause;
            }                
            ui_touchedTag = data;
        }

        switch (ui_touchedTag) {
            case 1: //power on
                if (ui_initialTouch) {
                    if (bm83_state != BM83_OFF) {
                        ui_screenUpdateNeeded = true;
                        break;
                    }
                    ui_touchLocked = true;
                    DAP_StartUp(ui_unlockTouch);
                    BM83_PowerOn(ui_unlockTouch);
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
                if ((ui_initialTouch || ui_longTouchTick) && !dap_muted && dap_volume < ui_minVolume) {
                    ui_touchLocked = true;
                    uint16_t newVol = dap_volume + ui_volumeStep;
                    ui_setVol(newVol, ui_unlockTouch);
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
                if ((ui_initialTouch || ui_longTouchTick) && !dap_muted && dap_volume > ui_maxVolume) {
                    ui_touchLocked = true;
                    uint16_t newVol = dap_volume - ui_volumeStep;
                    ui_setVol(newVol, ui_unlockTouch);
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
            case 17:
            case 70:
                if (ui_initialTouch) {
                    if (light_on) Light_Off();
                    else Light_On();
                    ui_screenUpdateNeeded = true;
                }
                break;
            case 18:
            case 30:
                if (ui_initialTouch) {
                    ui_settingsType = UI_VOL_SETTINGS;
                    ui_screenUpdateNeeded = true;
                }
                break;
            case 19: //power off
                if (ui_touchReleased) {
                    if (bm83_state == BM83_OFF) {
                        ui_screenUpdateNeeded = true;
                        break;
                    }
                    ui_touchLocked = true;
                    DAP_ShutDown(ui_unlockTouch);
                    BM83_PowerOff(ui_unlockTouch);
                }
                break;
            case 31:
                if (ui_initialTouch) {
                    ui_settingsType = UI_FX_SETTINGS;
                    ui_screenUpdateNeeded = true;
                }
                break;
            case 32:
                if (ui_initialTouch) {
                    ui_settingsType = UI_DISP_SETTINGS;
                    ui_screenUpdateNeeded = true;
                }
                break;
            case 33:
                if (ui_initialTouch) {
                    ui_settingsType = UI_LIGHT_SETTINGS;
                    ui_screenUpdateNeeded = true;
                }
                break;
            case 39:
                if (ui_initialTouch) {
                    ui_settingsType = UI_NO_SETTINGS;
                    ui_screenUpdateNeeded = true;
                }
                break;
            case 62:
                if (ui_initialTouch) {
                    ui_themeColor = UI_THEME_COLOR_BLUE;
                    ui_screenUpdateNeeded = true;
                }
                break;
            case 63:
                if (ui_initialTouch) {
                    ui_themeColor = UI_THEME_COLOR_RED;
                    ui_screenUpdateNeeded = true;
                }
                break;
            case 64:
                if (ui_initialTouch) {
                    ui_themeColor = UI_THEME_COLOR_YELLOW;
                    ui_screenUpdateNeeded = true;
                }
                break;
            case 65:
                if (ui_initialTouch) {
                    ui_themeColor = UI_THEME_COLOR_GREEN;
                    ui_screenUpdateNeeded = true;
                }
                break;
            case 66:
                if (ui_touchReleased) {
                    ui_calibrate();
                }
                break;
            case 71:
                if (ui_initialTouch) {
                    if (light_s2l) Light_S2L_Disable();
                    else Light_S2L_Enable();
                    ui_screenUpdateNeeded = true;
                }
                break;
            case 41:
            case 42:
            case 43:
            case 51:
            case 52:
            case 53:
            case 60:
            case 61:
            case 72:
                if (!ui_touchReleased) {
                    ui_touchLocked = true;
                    FT8_memRead32(REG_TRACKER, ui_trackReadCallback, NULL);
                }
                break;
            default:
                break;
        }

        if (ui_touchReleased) {
            ui_touchedTag = 0;
            ui_longTouch = false;
        }
    }
    ui_initialTouch = false;
    ui_touchReleased = false;
    ui_longTouchTick = false;
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
    if (ui_screenActive) {
        if (newTouched && !ui_touchDown) {
            FT8_memWrite8(REG_INT_MASK, 0x82);
            ui_touchDown = true;
            ui_initialTouch = true;
            ui_nextTouchTickAt = SYS_TIME_CounterGet() + 3 * ui_touchTickPause;
        } else if (!newTouched && ui_touchDown) {
            FT8_memWrite8(REG_INT_MASK, 0x02);
            ui_touchDown = false;
            ui_touchReleased = true;
        }
        if (ui_touchDown) FT8_memRead32(REG_TOUCH_TAG_XY, ui_touchCoordReadCallback, NULL);
        else FT8_memRead8(REG_TOUCH_TAG, ui_tagReadCallback, NULL);
    } else if (newTouched) {
        ui_screenActivateRequested = true;
        ui_screenDeactivateRequested = false;
        ui_screenFadeStep = ui_screenBrightness / 10 + 1;
        ui_nextScreenFadeStepAt = SYS_TIME_CounterGet() + ui_screenFadeStepTime;
    }
}

void ui_doScreenDraw(bool success) {
    ui_screenUpdateNeeded = true;
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
    
    ui_screenUpdateNeeded = true;
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
    
    if (dap_overPower) {
        ui_setVol(dap_volume + (ui_volumeStep == 0x00c ? 0x018 : 0x010), ui_doScreenDraw);
        dap_overPower = false;
    }
    
    if (tick - ui_nextIntAt < UI_TIMEOUT_MAX_DIFF) {
        if (!GPIO_PinRead(GPIO_PIN_RC4) && !ui_touchLocked) {
            FT8_memRead8(REG_INT_FLAGS, NULL, NULL);
            FT8_memRead32(REG_TOUCH_DIRECT_XY, ui_touchReadCallback, NULL);
            ui_nextIntAt = tick + ui_intPause;
        } else {
            ui_nextIntAt = tick;
        }
    }
    
    if (ui_screenActive) {
        if (ui_screenDeactivateDelay != ui_fadeoutDelays[4] && tick - ui_screenDeactivateAt < UI_TIMEOUT_MAX_DIFF) {
            ui_screenActive = false;
            ui_screenActivateRequested = false;
            ui_screenDeactivateRequested = true;
            ui_screenFadeStep = ui_screenBrightness / 10 + 1;
            ui_nextScreenFadeStepAt = tick + ui_screenFadeStepTime;
            return;
        }
        
        if (batteryUpdated) {
            ui_screenUpdateNeeded = true;
            batteryUpdated = false;
        }

        if (tick - ui_nextScreenUpdateAt < UI_TIMEOUT_MAX_DIFF) {
            if (ui_screenUpdateNeeded) {
                ui_nextScreenUpdateAt = tick + ui_screenUpdatePause;
                ui_drawScreen();
                ui_screenUpdateNeeded = false;
            } else {
                ui_nextScreenUpdateAt = tick;
            }
        }
    } else if (ui_screenActivateRequested && tick - ui_nextScreenFadeStepAt < UI_TIMEOUT_MAX_DIFF) {
        uint8_t nextBrightness = ui_screenFadeBrightness + ui_screenFadeStep;
        if (nextBrightness >= ui_screenBrightness) {
            ui_screenActive = true;
            ui_screenActivateRequested = false;
            ui_screenDeactivateRequested = false;
            ui_screenDeactivateAt = tick + ui_screenDeactivateDelay;
            ui_screenUpdateNeeded = true;
            ui_nextScreenUpdateAt = tick + ui_screenUpdatePause;
            ui_setScreenBrightness(ui_screenBrightness);
        } else {
            ui_nextScreenFadeStepAt = tick + ui_screenFadeStepTime;
            ui_setScreenBrightness(nextBrightness);
        }
    } else if (ui_screenDeactivateRequested && tick - ui_nextScreenFadeStepAt < UI_TIMEOUT_MAX_DIFF) {
        if (ui_screenFadeBrightness <= ui_screenFadeStep) {
            ui_screenActivateRequested = false;
            ui_screenDeactivateRequested = false;
            ui_setScreenBrightness(0);
        } else {
            ui_nextScreenFadeStepAt = tick + ui_screenFadeStepTime;
            ui_setScreenBrightness(ui_screenFadeBrightness - ui_screenFadeStep);
        }
    }
}

/*void ui_updateScreenLoop(uintptr_t context) {
    ui_drawScreen();
}*/

/****************************/
/* Initialization functions */
/****************************/

void UI_IO_Init() {
    FT8_IO_Init();
    //SYS_INT_SourceEnable(INT_SOURCE_EXTERNAL_4);
    
    ui_intPause = SYS_TIME_MSToCount(100);
    ui_touchTickPause = SYS_TIME_MSToCount(400);
    ui_screenUpdatePause = SYS_TIME_MSToCount(100);
    ui_screenFadeStepTime = SYS_TIME_MSToCount(20);
    
    ui_fadeoutDelays[0] = SYS_TIME_MSToCount(5000);
    ui_fadeoutDelays[1] = SYS_TIME_MSToCount(10000);
    ui_fadeoutDelays[2] = SYS_TIME_MSToCount(20000);
    ui_fadeoutDelays[3] = SYS_TIME_MSToCount(30000);
    ui_screenDeactivateDelay = ui_fadeoutDelays[4];
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
    //ui_drawScreen();
    BM83_SetStateChangeCallback(ui_bm83StateChange);
    
    uint32_t tick = SYS_TIME_CounterGet();
    ui_nextIntAt = tick;
    ui_nextScreenUpdateAt = tick;
    ui_screenDeactivateDelay = ui_fadeoutDelays[2];
    ui_screenDeactivateAt = tick + ui_screenDeactivateDelay;
    
    if (ui_initCallback != NULL) ui_initCallback(true);
    
    SYS_TIME_CallbackRegisterMS(ui_touchSafetyUnlock, 0, 5000, SYS_TIME_PERIODIC);
    //SYS_TIME_CallbackRegisterMS(ui_screenDrawFallbackCb, 0, 200, SYS_TIME_PERIODIC);
}

void UI_Main_Init(SUCCESS_CALLBACK cb) {
    ui_initCallback = cb;
    FT8_init(ui_ftInitCallback);
}

#endif
/* *****************************************************************************
 End of File
 */
