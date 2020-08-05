
#include "ui_touch_draws.h"
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
#include "bm83.h"
#include "dap.h"

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

void ui_drawSpeaker40(uint16_t x, uint16_t y, uint32_t bgColor) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    FT8_cmd_scissor(x + 4, y, 36, 40);
    //right wave: outer
    FT8_start_cmd(BEGIN(FT8_POINTS));
    FT8_start_cmd(POINT_SIZE(240));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(320, 320));
    //right wave: inner
    FT8_start_cmd(POINT_SIZE(208));
    FT8_cmd_color(bgColor);
    FT8_start_cmd(VERTEX2F(320, 320));
    //middle wave: outer
    FT8_start_cmd(POINT_SIZE(184));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(303, 320));
    //middle wave: inner
    FT8_start_cmd(POINT_SIZE(152));
    FT8_cmd_color(bgColor);
    FT8_start_cmd(VERTEX2F(303, 320));
    //left wave: outer
    FT8_start_cmd(POINT_SIZE(128));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(282, 320));
    //left wave: inner
    FT8_start_cmd(POINT_SIZE(96));
    FT8_cmd_color(bgColor);
    FT8_start_cmd(VERTEX2F(282, 320));
    FT8_start_cmd(END());
    //wave cutoff
    FT8_start_cmd(BEGIN(FT8_EDGE_STRIP_L));
    FT8_start_cmd(LINE_WIDTH(8));
    FT8_start_cmd(VERTEX2F(457, 68));
    FT8_start_cmd(VERTEX2F(260, 320));
    FT8_start_cmd(VERTEX2F(457, 571));
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
    FT8_cmd_color(bgColor);
    FT8_start_cmd(VERTEX2F(278, 89));
    FT8_start_cmd(VERTEX2F(168, 199));
    FT8_start_cmd(VERTEX2F(65, 199));
    FT8_start_cmd(VERTEX2F(65, 440));
    FT8_start_cmd(VERTEX2F(168, 440));
    FT8_start_cmd(VERTEX2F(278, 551));
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

void ui_drawBulb(uint16_t x, uint16_t y, uint32_t bgColor, bool rays) {
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
    FT8_cmd_color(bgColor);
    FT8_start_cmd(VERTEX2F(320, 321));
    FT8_start_cmd(END());
    //circle cut
    FT8_start_cmd(BEGIN(FT8_LINES));
    FT8_start_cmd(LINE_WIDTH(40));
    FT8_start_cmd(VERTEX2F(320, 392));
    FT8_start_cmd(VERTEX2F(320, 446));
    //rays
    FT8_cmd_color(0xffffff);
    if (rays) {
        FT8_start_cmd(LINE_WIDTH(16));
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
    }
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

void ui_drawChain(uint16_t x, uint16_t y, uint32_t bgColor) {
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
    FT8_cmd_color(bgColor);
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
    FT8_cmd_color(bm83_state == BM83_CONNECTED ? ui_themeColor : 0xffffff);
    FT8_start_cmd(VERTEX2F(207, 207));
    FT8_start_cmd(VERTEX2F(435, 435));
    FT8_start_cmd(VERTEX2F(320, 550));
    FT8_start_cmd(VERTEX2F(320, 89));
    FT8_start_cmd(VERTEX2F(435, 204));
    FT8_start_cmd(VERTEX2F(207, 432));
    FT8_start_cmd(END());
    //points for pairing
    if (bm83_pairing) {
        FT8_start_cmd(BEGIN(FT8_POINTS));
        FT8_start_cmd(POINT_SIZE(32));
        FT8_start_cmd(VERTEX2F(188, 320));
        FT8_start_cmd(VERTEX2F(452, 320));
        FT8_start_cmd(END());
    }
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

void ui_drawLightIcon(uint16_t x, uint16_t y) {
    //setup
    FT8_start_cmd(SAVE_CONTEXT());
    FT8_start_cmd(VERTEX_TRANSLATE_X(16 * x));
    FT8_start_cmd(VERTEX_TRANSLATE_Y(16 * y));
    //lines
    FT8_start_cmd(BEGIN(FT8_LINES));
    FT8_start_cmd(LINE_WIDTH(16));
    FT8_cmd_color(0xffffff);
    FT8_start_cmd(VERTEX2F(320, 87));
    FT8_start_cmd(VERTEX2F(320, 156));
    FT8_start_cmd(VERTEX2F(443, 197));
    FT8_start_cmd(VERTEX2F(492, 148));
    FT8_start_cmd(VERTEX2F(484, 320));
    FT8_start_cmd(VERTEX2F(553, 320));
    FT8_start_cmd(VERTEX2F(443, 443));
    FT8_start_cmd(VERTEX2F(492, 492));
    FT8_start_cmd(VERTEX2F(320, 484));
    FT8_start_cmd(VERTEX2F(320, 553));
    FT8_start_cmd(VERTEX2F(197, 443));
    FT8_start_cmd(VERTEX2F(148, 492));
    FT8_start_cmd(VERTEX2F(87, 320));
    FT8_start_cmd(VERTEX2F(156, 320));
    FT8_start_cmd(VERTEX2F(197, 197));
    FT8_start_cmd(VERTEX2F(148, 148));
    FT8_start_cmd(END());
    //white circle
    FT8_start_cmd(BEGIN(FT8_POINTS));
    FT8_start_cmd(POINT_SIZE(121));
    FT8_start_cmd(VERTEX2F(320, 320));
    //black circle
    FT8_start_cmd(POINT_SIZE(89));
    FT8_cmd_color(0x000000);
    FT8_start_cmd(VERTEX2F(320, 320));
    FT8_start_cmd(END());    
    //cleanup
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
    //touch rect
    FT8_start_cmd(COLOR_A(0));
    FT8_start_cmd(TAG(1)); //power on
    FT8_start_cmd(VERTEX2F(64 * 16, 23 * 16));
    FT8_start_cmd(VERTEX2F(256 * 16, 240 * 16));
    FT8_start_cmd(END());
    FT8_start_cmd(RESTORE_CONTEXT());
}

#endif
/* *****************************************************************************
 End of File
 */
