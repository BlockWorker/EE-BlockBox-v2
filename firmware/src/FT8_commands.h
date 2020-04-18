
#ifndef FT8_COMMANDS_H_
#define FT8_COMMANDS_H_

#include "app.h"
#include "FT8_config.h"

typedef void (*VOID_CALLBACK)(bool success, uintptr_t context);
typedef void (*INT8_CALLBACK)(bool success, uint8_t data, uintptr_t context);
typedef void (*INT16_CALLBACK)(bool success, uint16_t data, uintptr_t context);
typedef void (*INT32_CALLBACK)(bool success, uint32_t data, uintptr_t context);

int FT8_cmdWrite(uint8_t cmd, uint8_t arg);
int FT8_cmdWriteC(uint8_t cmd, uint8_t arg, VOID_CALLBACK cb, uintptr_t context);
int FT8_memSendAddrRaw(uint32_t ftAddress, bool read);
int FT8_memRead8(uint32_t ftAddress, INT8_CALLBACK cb, uintptr_t context);
int FT8_memRead16(uint32_t ftAddress, INT16_CALLBACK cb, uintptr_t context);
int FT8_memRead32(uint32_t ftAddress, INT32_CALLBACK cb, uintptr_t context);
int FT8_memReadBuffer(uint32_t ftAddress, uint8_t* buffer, uint32_t length, VOID_CALLBACK cb, uintptr_t context);
int FT8_memWrite8(uint32_t ftAddress, uint8_t ftData8);
int FT8_memWrite8C(uint32_t ftAddress, uint8_t ftData8, VOID_CALLBACK cb, uintptr_t context);
int FT8_memWrite8Raw(uint8_t ftData8);
int FT8_memWrite8RawC(uint8_t ftData8, VOID_CALLBACK cb, uintptr_t context);
int FT8_memWrite16(uint32_t ftAddress, uint16_t ftData16);
int FT8_memWrite16C(uint32_t ftAddress, uint16_t ftData16, VOID_CALLBACK cb, uintptr_t context);
int FT8_memWrite16Raw(uint16_t ftData16);
int FT8_memWrite16RawC(uint16_t ftData16, VOID_CALLBACK cb, uintptr_t context);
int FT8_memWrite32(uint32_t ftAddress, uint32_t ftData32);
int FT8_memWrite32C(uint32_t ftAddress, uint32_t ftData32, VOID_CALLBACK cb, uintptr_t context);
int FT8_memWrite32Raw(uint32_t ftData32);
int FT8_memWrite32RawC(uint32_t ftData32, VOID_CALLBACK cb, uintptr_t context);
int FT8_memWriteBuffer(uint32_t ftAddress, uint8_t* buffer, uint32_t length);
int FT8_memWriteBufferC(uint32_t ftAddress, uint8_t* buffer, uint32_t length, VOID_CALLBACK cb, uintptr_t context);
int FT8_memWriteBufferRaw(uint8_t* buffer, uint32_t length);
int FT8_memWriteBufferRawC(uint8_t* buffer, uint32_t length, VOID_CALLBACK cb, uintptr_t context);
//void FT8_memWrite_flash_buffer(uint32_t ftAddress, const uint8_t *data, uint16_t len);
int FT8_busy(VOID_CALLBACK cb, uintptr_t context);
void FT8_start_cmd(uint32_t command);

int FT8_get_cmdoffset(VOID_CALLBACK cb, uintptr_t context);
uint16_t FT8_report_cmdoffset();
int FT8_get_touch_tag(INT32_CALLBACK cb, uintptr_t context);
int FT8_cmd_start();
int FT8_cmd_execute(VOID_CALLBACK cb, uintptr_t context);

/* commands to draw graphics objects: */
int FT8_cmd_text(int16_t x0, int16_t y0, int16_t font, uint16_t options, const char* text);
int FT8_cmd_button(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text);
int FT8_cmd_clock(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t hours, uint16_t minutes, uint16_t seconds, uint16_t millisecs);
int FT8_cmd_bgcolor(uint32_t color);
int FT8_cmd_fgcolor(uint32_t color);
int FT8_cmd_gradcolor(uint32_t color);
int FT8_cmd_gauge(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t major, uint16_t minor, uint16_t val, uint16_t range);
int FT8_cmd_gradient(int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1);
int FT8_cmd_keys(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* text);
int FT8_cmd_progress(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t range);
int FT8_cmd_scrollbar(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t size, uint16_t range);
int FT8_cmd_slider(int16_t x0, int16_t y0, int16_t w0, int16_t h0, uint16_t options, uint16_t val, uint16_t range);
int FT8_cmd_dial(int16_t x0, int16_t y0, int16_t r0, uint16_t options, uint16_t val);
int FT8_cmd_toggle(int16_t x0, int16_t y0, int16_t w0, int16_t font, uint16_t options, uint16_t state, const char* text);
int FT8_cmd_number(int16_t x0, int16_t y0, int16_t font, uint16_t options, int32_t number);

#ifdef FT8_81X_ENABLE
int FT8_cmd_setbase(uint32_t _base);
int FT8_cmd_setbitmap(uint32_t addr, uint16_t fmt, uint16_t width, uint16_t height);
#endif

/* commands to operate on memory: */
int FT8_cmd_memzero(uint32_t ptr, uint32_t num);
int FT8_cmd_memset(uint32_t ptr, uint8_t value, uint32_t num);
/*(void FT8_cmd_memwrite(uint32_t dest, uint32_t num, const uint8_t *data); */
int FT8_cmd_memcpy(uint32_t dest, uint32_t src, uint32_t num);
int FT8_cmd_append(uint32_t ptr, uint32_t num);


/* commands for loading image data into FT8xx memory: */
int FT8_cmd_inflate(uint32_t ptr, const uint8_t *data, uint16_t len);
int FT8_cmd_loadimage(uint32_t ptr, uint32_t options, const uint8_t *data, uint16_t len);
#ifdef FT8_81X_ENABLE
int FT8_cmd_mediafifo(uint32_t ptr, uint32_t size);
#endif

/* commands for setting the bitmap transform matrix: */
int FT8_cmd_translate(int32_t tx, int32_t ty);
int FT8_cmd_scale(int32_t sx, int32_t sy);
int FT8_cmd_rotate(int32_t ang);
int FT8_cmd_getmatrix(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f);


/* other commands: */
int FT8_cmd_calibrate();
int FT8_cmd_interrupt(uint32_t ms);
int FT8_cmd_setfont(uint32_t font, uint32_t ptr);
#ifdef FT8_81X_ENABLE
int FT8_cmd_romfont(uint32_t font, uint32_t romslot);
int FT8_cmd_setfont2(uint32_t font, uint32_t ptr, uint32_t firstchar);
int FT8_cmd_setrotate(uint32_t r);
int FT8_cmd_setscratch(uint32_t handle);
#endif
int FT8_cmd_sketch(int16_t x0, int16_t y0, uint16_t w0, uint16_t h0, uint32_t ptr, uint16_t format);
int FT8_cmd_snapshot(uint32_t ptr);
#ifdef FT8_81X_ENABLE
int FT8_cmd_snapshot2(uint32_t fmt, uint32_t ptr, int16_t x0, int16_t y0, int16_t w0, int16_t h0);
#endif
int FT8_cmd_spinner(int16_t x0, int16_t y0, uint16_t style, uint16_t scale);
int FT8_cmd_track(int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t tag);


/* commands that return values by writing to the command-fifo */
uint16_t FT8_cmd_memcrc(uint32_t ptr, uint32_t num);
uint16_t FT8_cmd_getptr();
uint16_t FT8_cmd_regread(uint32_t ptr);
uint16_t FT8_cmd_getprops(uint32_t ptr);


/* meta-commands, sequences of several display-list entries condensed into simpler to use functions at the price of some overhead */
int FT8_cmd_begindisplay(bool color, bool stencil, bool tag, uint32_t clearColor);
int FT8_cmd_begindisplay_limited(bool color, bool stencil, bool tag, uint32_t clearColor, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
int FT8_cmd_color(uint32_t color);
int FT8_cmd_point(int16_t x0, int16_t y0, uint16_t size);
int FT8_cmd_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t w0);
int FT8_cmd_rect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t corner);
int FT8_cmd_enddisplay();
int FT8_cmd_tag(uint8_t tag);
int FT8_cmd_tagmask(bool enableTag);
int FT8_cmd_scissor(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/* startup FT8xx: */
void FT8_init(SUCCESS_CALLBACK cb);
void FT8_IO_Init();
void FT8_Tasks();

#endif /* FT8_COMMANDS_H_ */