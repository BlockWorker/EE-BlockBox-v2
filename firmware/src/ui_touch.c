
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

/*****************************/
/* Variables and definitions */
/*****************************/

SUCCESS_CALLBACK ui_initCallback;

bool ui_interruptReceived = false;
uint8_t ui_touchedTag = 0;
bool ui_touchDown = false;

uint32_t ui_themeColor = 0x0040ff;

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

void ui_drawCog(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    //center circle white
    FT8_start_cmd(BEGIN(FT8_POINTS));
    FT8_start_cmd(POINT_SIZE(94));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(320, 320));
    //center circle black
    FT8_start_cmd(POINT_SIZE(62));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(320, 320));
    FT8_start_cmd(END());
    //outer line
    FT8_start_cmd(BEGIN(FT8_LINE_STRIP));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(291, 112));
    FT8_start_cmd(VERTEX2F(276, 163));
    FT8_start_cmd(VERTEX2F(241, 177));
    FT8_start_cmd(VERTEX2F(194, 152));
    FT8_start_cmd(VERTEX2F(153, 192));
    FT8_start_cmd(VERTEX2F(178, 239));
    FT8_start_cmd(VERTEX2F(164, 274));
    FT8_start_cmd(VERTEX2F(112, 290));
    FT8_start_cmd(VERTEX2F(112, 346));
    FT8_start_cmd(VERTEX2F(162, 362));
    FT8_start_cmd(VERTEX2F(177, 399));
    FT8_start_cmd(VERTEX2F(152, 446));
    FT8_start_cmd(VERTEX2F(192, 486));
    FT8_start_cmd(VERTEX2F(239, 461));
    FT8_start_cmd(VERTEX2F(275, 476));
    FT8_start_cmd(VERTEX2F(291, 527));
    FT8_start_cmd(VERTEX2F(348, 527));
    FT8_start_cmd(VERTEX2F(363, 477));
    FT8_start_cmd(VERTEX2F(400, 461));
    FT8_start_cmd(VERTEX2F(447, 486));
    FT8_start_cmd(VERTEX2F(488, 445));
    FT8_start_cmd(VERTEX2F(462, 398));
    FT8_start_cmd(VERTEX2F(478, 361));
    FT8_start_cmd(VERTEX2F(527, 346));
    FT8_start_cmd(VERTEX2F(527, 290));
    FT8_start_cmd(VERTEX2F(476, 274));
    FT8_start_cmd(VERTEX2F(461, 239));
    FT8_start_cmd(VERTEX2F(486, 192));
    FT8_start_cmd(VERTEX2F(446, 152));
    FT8_start_cmd(VERTEX2F(398, 177));
    FT8_start_cmd(VERTEX2F(364, 163));
    FT8_start_cmd(VERTEX2F(348, 112));
    FT8_start_cmd(VERTEX2F(291, 112));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawSpeaker28(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    FT8_cmd_scissor(x, y, 28, 28);
    //right wave: outer
    FT8_start_cmd(BEGIN(FT8_POINTS));
    FT8_start_cmd(POINT_SIZE(168));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(224, 224));
    //right wave: inner
    FT8_start_cmd(POINT_SIZE(146));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(224, 224));
    //middle wave: outer
    FT8_start_cmd(POINT_SIZE(129));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(212, 224));
    //middle wave: inner
    FT8_start_cmd(POINT_SIZE(107));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(212, 224));
    //left wave: outer
    FT8_start_cmd(POINT_SIZE(90));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(197, 224));
    //left wave: inner
    FT8_start_cmd(POINT_SIZE(68));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(197, 224));
    FT8_start_cmd(END());
    //wave cutoff
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_L));
    FT8_start_cmd(LINE_WIDTH(6));
    FT8_start_cmd(VERTEX2F(325, 40));
    FT8_start_cmd(VERTEX2F(182, 224));
    FT8_start_cmd(VERTEX2F(325, 404));
    FT8_start_cmd(END());
    //body rect
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(64, 98));
    FT8_start_cmd(VERTEX2F(170, 350));
    FT8_start_cmd(END());
    //body cutouts
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_L));
    FT8_start_cmd(LINE_WIDTH(6));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(185, 72));
    FT8_start_cmd(VERTEX2F(118, 139));
    FT8_start_cmd(VERTEX2F(45, 139));
    FT8_start_cmd(VERTEX2F(45, 308));
    FT8_start_cmd(VERTEX2F(118, 308));
    FT8_start_cmd(VERTEX2F(185, 375));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawSpeaker40(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    FT8_cmd_scissor(x, y, 40, 40);
    //right wave: outer
    FT8_start_cmd(BEGIN(FT8_POINTS));
    FT8_start_cmd(POINT_SIZE(240));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(320, 320));
    //right wave: inner
    FT8_start_cmd(POINT_SIZE(208));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(320, 320));
    //middle wave: outer
    FT8_start_cmd(POINT_SIZE(184));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(303, 320));
    //middle wave: inner
    FT8_start_cmd(POINT_SIZE(152));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(303, 320));
    //left wave: outer
    FT8_start_cmd(POINT_SIZE(128));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(282, 320));
    //left wave: inner
    FT8_start_cmd(POINT_SIZE(96));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(282, 320));
    FT8_start_cmd(END());
    //wave cutoff
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_L));
    FT8_start_cmd(LINE_WIDTH(8));
    FT8_start_cmd(VERTEX2F(438, 92));
    FT8_start_cmd(VERTEX2F(260, 320));
    FT8_start_cmd(VERTEX2F(438, 548));
    FT8_start_cmd(END());
    //body rect
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(77, 112));
    FT8_start_cmd(VERTEX2F(285, 528));
    FT8_start_cmd(END());
    //body cutouts
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_L));
    FT8_start_cmd(LINE_WIDTH(8));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(264, 103));
    FT8_start_cmd(VERTEX2F(168, 199));
    FT8_start_cmd(VERTEX2F(65, 199));
    FT8_start_cmd(VERTEX2F(65, 440));
    FT8_start_cmd(VERTEX2F(168, 440));
    FT8_start_cmd(VERTEX2F(264, 537));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawPowerOffButton(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    //outer circle
    FT8_start_cmd(BEGIN(FT8_POINTS));
    FT8_start_cmd(POINT_SIZE(208));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(320, 349));
    //inner cirle
    FT8_start_cmd(POINT_SIZE(176));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(320, 349));
    FT8_start_cmd(END());
    //black line
    FT8_start_cmd(BEGIN(FT8_LINES));
    FT8_start_cmd(LINE_WIDTH(48));
    FT8_start_cmd(VERTEX2F(320, 120));
    FT8_start_cmd(VERTEX2F(320, 192));
    //white line
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(320, 97));
    FT8_start_cmd(VERTEX2F(320, 273));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawBulb(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    //outer circle
    FT8_start_cmd(BEGIN(FT8_POINTS));
    FT8_start_cmd(POINT_SIZE(129));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(320, 321));
    //inner cirle
    FT8_start_cmd(POINT_SIZE(97));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(320, 321));
    FT8_start_cmd(END());
    //circle cut
    FT8_start_cmd(BEGIN(FT8_LINES));
    FT8_start_cmd(LINE_WIDTH(40));
    FT8_start_cmd(VERTEX2F(320, 392));
    FT8_start_cmd(VERTEX2F(320, 446));
    //rays
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(156, 268));
    FT8_start_cmd(VERTEX2F(105, 241));
    FT8_start_cmd(VERTEX2F(218, 180));
    FT8_start_cmd(VERTEX2F(182, 134));
    FT8_start_cmd(VERTEX2F(320, 150));
    FT8_start_cmd(VERTEX2F(320, 92));
    FT8_start_cmd(VERTEX2F(421, 180));
    FT8_start_cmd(VERTEX2F(457, 134));
    FT8_start_cmd(VERTEX2F(483, 268));
    FT8_start_cmd(VERTEX2F(534, 241));
    FT8_start_cmd(END());
    //socket edge
    FT8_start_cmd(BEGIN(FT8_LINE_STRIP));
    FT8_start_cmd(LINE_WIDTH(20));
    FT8_start_cmd(VERTEX2F(273, 424));
    FT8_start_cmd(VERTEX2F(273, 495));
    FT8_start_cmd(VERTEX2F(288, 526));
    FT8_start_cmd(VERTEX2F(350, 526));
    FT8_start_cmd(VERTEX2F(366, 495));
    FT8_start_cmd(VERTEX2F(366, 424));
    FT8_start_cmd(END());
    //socket fill
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_start_cmd(VERTEX2F(296, 440));
    FT8_start_cmd(VERTEX2F(344, 513));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawChain(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    //outer lines
    FT8_start_cmd(BEGIN(FT8_LINES));
    FT8_start_cmd(LINE_WIDTH(88));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(178, 178));
    FT8_start_cmd(VERTEX2F(239, 239));
    FT8_start_cmd(VERTEX2F(400, 400));
    FT8_start_cmd(VERTEX2F(461, 461));
    //inner lines
    FT8_start_cmd(LINE_WIDTH(56));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(178, 178));
    FT8_start_cmd(VERTEX2F(239, 239));
    FT8_start_cmd(VERTEX2F(400, 400));
    FT8_start_cmd(VERTEX2F(461, 461));
    //connection
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(260, 260));
    FT8_start_cmd(VERTEX2F(379, 379));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawBluetooth(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    //lines
    FT8_start_cmd(BEGIN(FT8_LINE_STRIP));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(207, 207));
    FT8_start_cmd(VERTEX2F(435, 435));
    FT8_start_cmd(VERTEX2F(320, 550));
    FT8_start_cmd(VERTEX2F(320, 89));
    FT8_start_cmd(VERTEX2F(435, 204));
    FT8_start_cmd(VERTEX2F(207, 432));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawLightning(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    //lines
    FT8_start_cmd(BEGIN(FT8_LINE_STRIP));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(281, 121));
    FT8_start_cmd(VERTEX2F(444, 121));
    FT8_start_cmd(VERTEX2F(344, 279));
    FT8_start_cmd(VERTEX2F(454, 279));
    FT8_start_cmd(VERTEX2F(208, 526));
    FT8_start_cmd(VERTEX2F(279, 325));
    FT8_start_cmd(VERTEX2F(195, 325));
    FT8_start_cmd(VERTEX2F(281, 121));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}

/*void ui_drawBackButtonOld(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    FT8_cmd_scissor(x, y, 50, 50);
    //outer rect
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(72, 170));
    FT8_start_cmd(VERTEX2F(728, 630));
    //inner rect
    FT8_cmd_color(ui_themeColor);
    FT8_start_cmd(VERTEX2F(100, 220));
    FT8_start_cmd(VERTEX2F(690, 580));
    FT8_start_cmd(END());
    //top inner cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_A));
    FT8_start_cmd(LINE_WIDTH(8));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(175, 359));
    FT8_start_cmd(VERTEX2F(435, 208));
    FT8_start_cmd(VERTEX2F(435, 361));
    FT8_start_cmd(VERTEX2F(700, 205));
    FT8_start_cmd(END());
    //bottom inner cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_B));
    FT8_start_cmd(VERTEX2F(175, 440));
    FT8_start_cmd(VERTEX2F(435, 591));
    FT8_start_cmd(VERTEX2F(435, 438));
    FT8_start_cmd(VERTEX2F(700, 594));
    FT8_start_cmd(END());
    //top outer cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_A));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(195, 320));
    FT8_start_cmd(VERTEX2F(464, 164));
    FT8_start_cmd(VERTEX2F(464, 314));
    FT8_start_cmd(VERTEX2F(715, 168));
    FT8_start_cmd(END());
    //bottom outer cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_B));
    FT8_start_cmd(VERTEX2F(195, 479));
    FT8_start_cmd(VERTEX2F(464, 635));
    FT8_start_cmd(VERTEX2F(464, 485));
    FT8_start_cmd(VERTEX2F(715, 631));
    FT8_start_cmd(END());
    //bar cutoff rects
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_start_cmd(VERTEX2F(50, 0));
    FT8_start_cmd(VERTEX2F(210, 160));
    FT8_start_cmd(VERTEX2F(50, 640));
    FT8_start_cmd(VERTEX2F(210, 800));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}*/

void ui_drawBackButton(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    FT8_cmd_scissor(x, y, 50, 50);
    //inner rect
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(ui_themeColor);
    FT8_start_cmd(VERTEX2F(91, 218));
    FT8_start_cmd(VERTEX2F(691, 582));
    //top cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_A));
    FT8_start_cmd(LINE_WIDTH(8));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(86, 209));
    FT8_start_cmd(VERTEX2F(175, 209));
    FT8_start_cmd(VERTEX2F(175, 361));
    FT8_start_cmd(VERTEX2F(435, 206));
    FT8_start_cmd(VERTEX2F(435, 360));
    FT8_start_cmd(VERTEX2F(708, 195));
    FT8_start_cmd(END());
    //bottom cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_B));
    FT8_start_cmd(VERTEX2F(86, 590));
    FT8_start_cmd(VERTEX2F(175, 590));
    FT8_start_cmd(VERTEX2F(175, 438));
    FT8_start_cmd(VERTEX2F(435, 593));
    FT8_start_cmd(VERTEX2F(435, 439));
    FT8_start_cmd(VERTEX2F(708, 604));
    FT8_start_cmd(END());
    //outline
    FT8_start_cmd(BEGIN(FT8_LINE_STRIP));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(86, 209));
    FT8_start_cmd(VERTEX2F(175, 209));
    FT8_start_cmd(VERTEX2F(175, 361));
    FT8_start_cmd(VERTEX2F(435, 206));
    FT8_start_cmd(VERTEX2F(435, 360));
    FT8_start_cmd(VERTEX2F(695, 203));
    FT8_start_cmd(VERTEX2F(695, 596));
    FT8_start_cmd(VERTEX2F(435, 439));
    FT8_start_cmd(VERTEX2F(435, 593));
    FT8_start_cmd(VERTEX2F(175, 438));
    FT8_start_cmd(VERTEX2F(175, 590));
    FT8_start_cmd(VERTEX2F(86, 590));
    FT8_start_cmd(VERTEX2F(86, 209));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}

/*void ui_drawForwardButtonOld(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    FT8_cmd_scissor(x, y, 50, 50);
    //outer rect
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(72, 170));
    FT8_start_cmd(VERTEX2F(728, 630));
    //inner rect
    FT8_cmd_color(ui_themeColor);
    FT8_start_cmd(VERTEX2F(110, 220));
    FT8_start_cmd(VERTEX2F(700, 580));
    FT8_start_cmd(END());
    //top inner cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_A));
    FT8_start_cmd(LINE_WIDTH(8));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(624, 359));
    FT8_start_cmd(VERTEX2F(364, 208));
    FT8_start_cmd(VERTEX2F(364, 361));
    FT8_start_cmd(VERTEX2F(99, 205));
    FT8_start_cmd(END());
    //bottom inner cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_B));
    FT8_start_cmd(VERTEX2F(624, 440));
    FT8_start_cmd(VERTEX2F(364, 591));
    FT8_start_cmd(VERTEX2F(364, 438));
    FT8_start_cmd(VERTEX2F(99, 594));
    FT8_start_cmd(END());
    //top outer cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_A));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(604, 320));
    FT8_start_cmd(VERTEX2F(335, 164));
    FT8_start_cmd(VERTEX2F(335, 314));
    FT8_start_cmd(VERTEX2F(84, 168));
    FT8_start_cmd(END());
    //bottom outer cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_B));
    FT8_start_cmd(VERTEX2F(604, 479));
    FT8_start_cmd(VERTEX2F(335, 635));
    FT8_start_cmd(VERTEX2F(335, 485));
    FT8_start_cmd(VERTEX2F(84, 631));
    FT8_start_cmd(END());
    //bar cutoff rects
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_start_cmd(VERTEX2F(590, 0));
    FT8_start_cmd(VERTEX2F(750, 160));
    FT8_start_cmd(VERTEX2F(590, 640));
    FT8_start_cmd(VERTEX2F(750, 800));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}*/

void ui_drawForwardButton(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    FT8_cmd_scissor(x, y, 50, 50);
    //inner rect
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(ui_themeColor);
    FT8_start_cmd(VERTEX2F(109, 218));
    FT8_start_cmd(VERTEX2F(709, 582));
    //top cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_A));
    FT8_start_cmd(LINE_WIDTH(8));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(713, 209));
    FT8_start_cmd(VERTEX2F(624, 209));
    FT8_start_cmd(VERTEX2F(624, 361));
    FT8_start_cmd(VERTEX2F(364, 206));
    FT8_start_cmd(VERTEX2F(364, 360));
    FT8_start_cmd(VERTEX2F(91, 195));
    FT8_start_cmd(END());
    //bottom cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_B));
    FT8_start_cmd(VERTEX2F(713, 590));
    FT8_start_cmd(VERTEX2F(624, 590));
    FT8_start_cmd(VERTEX2F(624, 438));
    FT8_start_cmd(VERTEX2F(364, 593));
    FT8_start_cmd(VERTEX2F(364, 439));
    FT8_start_cmd(VERTEX2F(91, 604));
    FT8_start_cmd(END());
    //outline
    FT8_start_cmd(BEGIN(FT8_LINE_STRIP));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(713, 209));
    FT8_start_cmd(VERTEX2F(624, 209));
    FT8_start_cmd(VERTEX2F(624, 361));
    FT8_start_cmd(VERTEX2F(364, 206));
    FT8_start_cmd(VERTEX2F(364, 360));
    FT8_start_cmd(VERTEX2F(104, 203));
    FT8_start_cmd(VERTEX2F(104, 596));
    FT8_start_cmd(VERTEX2F(364, 439));
    FT8_start_cmd(VERTEX2F(364, 593));
    FT8_start_cmd(VERTEX2F(624, 438));
    FT8_start_cmd(VERTEX2F(624, 590));
    FT8_start_cmd(VERTEX2F(713, 590));
    FT8_start_cmd(VERTEX2F(713, 209));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}

/*void ui_drawPlayButtonOld(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    FT8_cmd_scissor(x, y, 50, 50);
    //outer rect
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(142, 80));
    FT8_start_cmd(VERTEX2F(703, 720));
    //inner rect
    FT8_cmd_color(ui_themeColor);
    FT8_start_cmd(VERTEX2F(177, 120));
    FT8_start_cmd(VERTEX2F(657, 680));
    FT8_start_cmd(END());
    //inner cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_R));
    FT8_start_cmd(LINE_WIDTH(8));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(159, 108));
    FT8_start_cmd(VERTEX2F(661, 400));
    FT8_start_cmd(VERTEX2F(159, 691));
    FT8_start_cmd(END());
    //outer cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_R));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(131, 62));
    FT8_start_cmd(VERTEX2F(712, 400));
    FT8_start_cmd(VERTEX2F(131, 737));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}*/

void ui_drawPlayButton(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    FT8_cmd_scissor(x, y, 50, 50);
    //inner rect
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(ui_themeColor);
    FT8_start_cmd(VERTEX2F(172, 127));
    FT8_start_cmd(VERTEX2F(648, 673));
    //cutout
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_R));
    FT8_start_cmd(LINE_WIDTH(8));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(150, 100));
    FT8_start_cmd(VERTEX2F(665, 400));
    FT8_start_cmd(VERTEX2F(150, 699));
    FT8_start_cmd(END());
    //outline
    FT8_start_cmd(BEGIN(FT8_LINE_STRIP));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(167, 110));
    FT8_start_cmd(VERTEX2F(665, 400));
    FT8_start_cmd(VERTEX2F(167, 689));
    FT8_start_cmd(VERTEX2F(167, 110));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}

void ui_drawPauseButton(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    //outer rects
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(32));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(207, 126));
    FT8_start_cmd(VERTEX2F(305, 674));
    FT8_start_cmd(VERTEX2F(495, 126));
    FT8_start_cmd(VERTEX2F(593, 674));
    //inner rects
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(ui_themeColor);
    FT8_start_cmd(VERTEX2F(207, 126));
    FT8_start_cmd(VERTEX2F(305, 674));
    FT8_start_cmd(VERTEX2F(495, 126));
    FT8_start_cmd(VERTEX2F(593, 674));
    FT8_start_cmd(END());
    //cleanup
    FT8_start_cmd(RESTORE_CONTEXT());
}

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

void ui_drawBigOnButton() {
    FT8_start_cmd(SAVE_CONTEXT());
    //outer white circle
    FT8_start_cmd(BEGIN(FT8_POINTS));
    FT8_start_cmd(POINT_SIZE(16 * 90 + 4));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(16 * 160, 16 * 145));
    //themed circle
    FT8_start_cmd(POINT_SIZE(16 * 87 + 8));
    FT8_cmd_color(ui_themeColor);
    FT8_start_cmd(VERTEX2F(16 * 160, 16 * 145));
    //inner white circle
    FT8_start_cmd(POINT_SIZE(16 * 60 + 8));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(16 * 160, 16 * 145));
    //inner black circle
    FT8_start_cmd(POINT_SIZE(16 * 57 + 12));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(16 * 160, 16 * 145));
    FT8_start_cmd(END());
    //white edges of circle cut
    FT8_start_cmd(BEGIN(FT8_RECTS));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(16 * 138, 16 * 59));
    FT8_start_cmd(VERTEX2F(16 * 182, 16 * 90));
    //black circle cut
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(16 * 141, 16 * 55));
    FT8_start_cmd(VERTEX2F(16 * 179, 16 * 92));
    //white edges of I bar
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(16 * 144, 16 * 32));
    FT8_start_cmd(VERTEX2F(16 * 176, 16 * 120));
    //themed part of I bar
    FT8_cmd_color(ui_themeColor);
    FT8_start_cmd(VERTEX2F(16 * 147, 16 * 35));
    FT8_start_cmd(VERTEX2F(16 * 173, 16 * 117));
    FT8_start_cmd(END());
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
    ui_drawPlayButton(137, 101);
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
    FT8_cmd_text(68, 216, 26, FT8_OPT_CENTERX, "-40dB");
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
    ui_drawMainScreen();
    
    /*switch (bm83_state) {
        case BM83_NOT_INITIALIZED:
            FT8_cmd_color(0xffffff);
            FT8_cmd_text(160, 70, 29, FT8_OPT_CENTER, "Wait!");
            FT8_cmd_spinner(160, 140, 0, 0);
            break;
        case BM83_OFF:
            ui_drawBigOnButton();
            break;
        case BM83_IDLE:
            ui_drawIdleScreen();
            break;
    }*/
    
    
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

void ui_bm83StateChange(BM83_STATE_CHANGE_TYPE changeType) {
    ui_drawScreen();
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
        ui_drawScreen();
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
    ui_drawScreen();
    BM83_SetStateChangeCallback(ui_bm83StateChange);
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
