/* ************************************************************************** */


#ifndef _UI_TOUCH_DRAWS_H    /* Guard against multiple inclusion */
#define _UI_TOUCH_DRAWS_H

#include "ui.h"
#ifdef UI_TOUCH

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

    uint32_t ui_themeColor;
    
    void ui_drawCog(uint16_t x, uint16_t y);
    void ui_drawSpeaker28(uint16_t x, uint16_t y);
    void ui_drawSpeaker40(uint16_t x, uint16_t y, uint32_t bgColor);
    void ui_drawPowerOffButton(uint16_t x, uint16_t y);
    void ui_drawBulb(uint16_t x, uint16_t y, uint32_t bgColor);
    void ui_drawChain(uint16_t x, uint16_t y, uint32_t bgColor);
    void ui_drawBluetooth(uint16_t x, uint16_t y);
    void ui_drawLightning(uint16_t x, uint16_t y);
    void ui_drawBackButton(uint16_t x, uint16_t y);
    void ui_drawForwardButton(uint16_t x, uint16_t y);
    void ui_drawPlayButton(uint16_t x, uint16_t y);
    void ui_drawPauseButton(uint16_t x, uint16_t y);
    void ui_drawLightIcon(uint16_t x, uint16_t y);
    void ui_drawBigOnButton();

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif

#endif /* multiple inclusion */

/* *****************************************************************************
 End of File
 */
