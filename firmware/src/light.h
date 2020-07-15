/* ************************************************************************** */


#ifndef _LIGHT_H    /* Guard against multiple inclusion */
#define _LIGHT_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */


#include "stdbool.h"


/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

    
    bool light_on;
    bool light_s2l;
    float light_brightness;
    
    void Light_On();
    void Light_Off();
    void Light_S2L_Enable();
    void Light_S2L_Disable();
    void Light_Tasks();
    void Light_Init();


    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* multiple inclusion */

/* *****************************************************************************
 End of File
 */
