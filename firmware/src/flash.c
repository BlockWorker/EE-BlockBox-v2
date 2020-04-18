/* ************************************************************************** */

#include "flash.h"

#define FLASH_TOUCH_MATRIX_ADDRESS (FLASH_USER_PAGE_VIRT | 0x0000)
#define FLASH_TOUCH_MATRIX_BYTES 24

uint32_t flash_touch_matrix[6] = { 0 };

void FLASH_Read() {
    NVM_Read(flash_touch_matrix, FLASH_TOUCH_MATRIX_BYTES, FLASH_TOUCH_MATRIX_ADDRESS);
    while (NVM_IsBusy());
}

void FLASH_Write() {
    while (NVM_IsBusy());
    NVM_PageErase(FLASH_USER_PAGE_VIRT);
    while (NVM_IsBusy());
    int i;
    for (i = 0; i < 6; i++) {
        NVM_WordWrite(flash_touch_matrix[i], FLASH_TOUCH_MATRIX_ADDRESS + 4 * i);
        while (NVM_IsBusy());
    }
}


/* *****************************************************************************
 End of File
 */
