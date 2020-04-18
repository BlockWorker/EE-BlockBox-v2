/* ************************************************************************** */


#ifndef _FLASH_H    /* Guard against multiple inclusion */
#define _FLASH_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include "app.h"



/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

    #define FLASH_USER_PAGE_PHYS 0x1d1fc000 //last page
    #define FLASH_USER_PAGE_VIRT (FLASH_USER_PAGE_PHYS | 0x80000000)

    uint32_t flash_touch_matrix[6];
    
    void FLASH_Read();
    void FLASH_Write();

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* multiple inclusion */

/* *****************************************************************************
 End of File
 */
