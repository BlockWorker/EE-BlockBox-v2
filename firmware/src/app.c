/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "i2s.h"
#include "bm83.h"
#include "dap.h"
#include "ui.h"
#include "flash.h"

APP_DATA appData;

//batt voltage logging:
//48200 counts = 15.84v ==> division factor 3042
//51900 counts = 16.38v ==> division factor 3168
//52600 counts = 16.43v ==> division factor 3201
//53000 counts = 16.47v ==> division factor 3218
//50600 counts = 15.72v ==> division factor 3218
//50000 counts = 16.04v ==> division factor 3118
//49500 counts = 16.02v ==> division factor 3090
//--- capacitors added ---
//48060c = 15.97v ==> x = 3009 //discharge, power ~= 0
//51350c = 16.45v ==> x = 3121 //fast charge
//51500c = 16.50v ==> x = 3121
//52500c = 16.67v ==> x = 3150
//50300c = 16.31v ==> x = 3084 //charger disconnected
//50000c = 16.287v ==> x = 3070

#define BATTERY_VOLTAGE_AVG_LENGTH 2048
uint16_t batteryVoltageCounts[BATTERY_VOLTAGE_AVG_LENGTH] = { 0 };
uint32_t batteryVoltageCountSum = 0;
uint16_t batteryVoltageCountAverage = 0;
#define BATTERY_CURR_AVG_LENGTH 2048
uint16_t batteryCurrentCounts[BATTERY_CURR_AVG_LENGTH] = { 0 };
uint32_t batteryCurrentCountSum = 0;
uint16_t batteryCurrentCountAverage = 0;

#define BATTERY_VOLTAGE_CONV_FACTOR 3100.0
double batteryVolts = 0.0;
double batteryPercentRaw = 100.0;
uint8_t batteryPercent = 100.0;
double batteryAmps = 0.0;
bool batteryUpdated = false;

/********************/
/* Helper functions */
/********************/

//resets the microcontroller
void software_reset() {
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA; //unlock
    RSWRST = 1; //set software reset bit
    (void)RSWRST; //read RSWRST to cause actual reset
}

/*void sendStuff(uintptr_t context) {
    switch (context) {
        case 0:
            BM83_PowerOn(NULL);
            SYS_TIME_CallbackRegisterMS(sendStuff, 1, 2000, SYS_TIME_SINGLE);
            break;
        case 1:
            BM83_EnterPairing(NULL);
            DAP_MUTE_N_Set();
            break;
    }
}*/

//approximation for remaining battery energy (in Wh) based on voltage (in V) and current (in A)
//also see: https://www.desmos.com/calculator/nvqbapyxn2
double getRemainingBatteryEnergy(double totalVolts, double totalAmps) {
    double v = totalVolts / 4.0; //single cell voltage
    if (v < 3.0) v = 3.0;
    else if (v > 4.2) v = 4.2;
    
    double i = totalAmps / 4.0; //single cell current
    if (i < 0.2) i = 0.2;
    else if (i > 3.0) i = 3.0;
    
    double ov = 0.066 * i - 0.0132; //voltage-axis offset
    double oe = 1.8 * sqrt(sqrt(i - 0.08)) - 1.05941914429777626; //energy-axis offset
    
    double v1 = 3.48 - ov; //function crossover voltage 1
    double e1 = -1.5 * v1 + 7.5; //energy at v1
    double v2 = 3.645 - ov; //function crossover voltage 2
    double e2 = -1.5 * v2 + 10.6; //energy at v2
    double v3 = 4.098 - ov; //function crossover voltage 3
    double e3 = 10.2; //energy at v3
    
    double e;
    if (v < v1) { //lower exponential approximation
        double b = (e2 - e1) / ((v2 - v1) * (e1 - oe));
        double a = (e1 - oe) / expm1(b * (v1 - 3));
        e = a * expm1(b * (v - 3)) + oe;
    } else if (v < v2) { //first linear approximation
        e = (e2 - e1) / (v2 - v1) * (v - v1) + e1;
    } else if (v < v3) { //first linear approximation
        e = (e3 - e2) / (v3 - v2) * (v - v2) + e2;
    } else { //upper exponential approximation
        double d = (e3 - e2) / ((v3 - v2) * (10.53 - e3));
        double c = (e3 - 10.53) / expm1(d * (v3 - 4.2));
        e = c * expm1(d * (v - 4.2)) + 10.53;
    }
    
    return 16 * (e - oe); //total energy in all 16 cells
}

/**************************/
/* Runtime task functions */
/**************************/

void batVoltageADCCallback() {
    static uint16_t arrayPos = 0;
    
    batteryVoltageCountSum -= batteryVoltageCounts[arrayPos];
    batteryVoltageCounts[arrayPos] = ADCFLTR1bits.FLTRDATA;
    batteryVoltageCountSum += batteryVoltageCounts[arrayPos++];
    arrayPos %= BATTERY_VOLTAGE_AVG_LENGTH;
    
    batteryVoltageCountAverage = batteryVoltageCountSum / BATTERY_VOLTAGE_AVG_LENGTH;
    batteryVolts = batteryVoltageCountAverage / BATTERY_VOLTAGE_CONV_FACTOR;
    
    batteryPercentRaw = (batteryVolts - 12.0) / 0.048;
    if (batteryPercentRaw < 0.0) batteryPercentRaw = 0.0;
    else if (batteryPercentRaw > 100.0) batteryPercentRaw = 100.0;
    
    double diff = batteryPercent - batteryPercentRaw;
    if (diff < -0.1 || diff > 0.1) { //update percent with hysteresis
        batteryPercent = (uint8_t)rint(batteryPercentRaw);
    }
}

void batCurrentADCCallback() {
    static uint16_t arrayPos = 0;
    batteryCurrentCountSum -= batteryCurrentCounts[arrayPos];
    batteryCurrentCounts[arrayPos] = ADCFLTR2bits.FLTRDATA;
    batteryCurrentCountSum += batteryCurrentCounts[arrayPos++];
    arrayPos %= BATTERY_CURR_AVG_LENGTH;
    batteryCurrentCountAverage = batteryCurrentCountSum / BATTERY_CURR_AVG_LENGTH;
}

void millisecondCallback(uintptr_t context) {
    static uint32_t msCount = 0;
    
    ADCHS_ChannelConversionStart(ADCHS_CH1);
    ADCHS_ChannelConversionStart(ADCHS_CH2);
    
    if (msCount % BATTERY_VOLTAGE_AVG_LENGTH == 0) {
        batteryUpdated = true;
    }
    
    msCount++;
}

void generalTasks() {
    GPIO_PinWrite(AMP_RESET_N_PIN, DAP_VALID_Get());
    
    
}

void APP_Tasks() {
    switch(appData.state) {
        case APP_STATE_INIT_UI:
            UI_Tasks();
            break;        
        case APP_STATE_INIT_DAP:
            UI_Tasks();
            DAP_Tasks();
            break;
        case APP_STATE_INIT_BM83:
            UI_Tasks();
            DAP_Tasks();
            BM83_Tasks();
            break;
        case APP_STATE_SERVICE_TASKS:
            UI_Tasks();
            DAP_Tasks();
            BM83_Tasks();
            generalTasks();
            break;
        default:
            break;
    }
}

/****************************/
/* Initialization functions */
/****************************/

//callback for BM83 init
void init_bm83_callback(bool success) {
    static bool bmFailedBefore = false;
    
    if (success) { //success: BM init done
        appData.state = APP_STATE_SERVICE_TASKS;
        SYS_TIME_CallbackRegisterMS(millisecondCallback, NULL, 1, SYS_TIME_PERIODIC);
        //SYS_TIME_CallbackRegisterMS(sendStuff, 0, 1000, SYS_TIME_SINGLE);
    } else if (!bmFailedBefore) { //failed once: try again
        bmFailedBefore = true;
        BM83_Module_Init(init_bm83_callback);
    } else { //failed twice: reset
        software_reset();
    }
}

void init_dap_callback(bool success);

void init_dap_retry(uintptr_t context) {
    DAP_Chip_Init(init_dap_callback);
}

void init_dap_callback(bool success) {
    //static bool dapFailedBefore = false;
    static uint8_t dapFailedBefore = 0;
    
    if (success) { //success: DAP init done, start BM83 init
        appData.state = APP_STATE_INIT_BM83;
        BM83_Module_Init(init_bm83_callback);
        DAP_ShutDown(NULL);
    } else if (/*!dapFailedBefore*/dapFailedBefore++ < 20) { //failed once: try again
        SYS_TIME_CallbackRegisterMS(init_dap_retry, 0, 1000, SYS_TIME_SINGLE);
        //dapFailedBefore = true;
        //DAP_Chip_Init(init_dap_callback);
    } else { //failed twice: reset
        software_reset();
    }
}

void init_ui_callback(bool success) {
    static bool uiFailedBefore = false;
    
    if (success) { //success: DAP init done, start BM83 init
        appData.state = APP_STATE_INIT_DAP;
        SYS_TIME_CallbackRegisterMS(millisecondCallback, NULL, 1, SYS_TIME_PERIODIC);
        DAP_Chip_Init(init_dap_callback);
    } else if (!uiFailedBefore) { //failed once: try again
        uiFailedBefore = true;
        UI_Main_Init(init_ui_callback);
    } else { //failed twice: reset
        software_reset();
    }
}

void initBatADC() {
    ADCFLTR1 = 0; //clear filter 1 config
    ADCFLTR1bits.CHNLID = 1; //source: channel 1
    ADCFLTR1bits.DFMODE = 0; //oversampling mode
    ADCFLTR1bits.OVRSAM = 0b011; //256x oversampling
    ADCFLTR1bits.AFGIEN = 1; //enable interrupt
    ADCFLTR1bits.AFEN = 1; //enable filter 1
    
    ADCFLTR2 = 0; //clear filter 2 config
    ADCFLTR2bits.CHNLID = 2; //source: channel 2
    ADCFLTR2bits.DFMODE = 0; //oversampling mode
    ADCFLTR2bits.OVRSAM = 0b011; //256x oversampling
    ADCFLTR2bits.AFGIEN = 1; //enable interrupt
    ADCFLTR2bits.AFEN = 1; //enable filter 2
    
    SYS_INT_SourceEnable(INT_SOURCE_ADC_DF1);
    SYS_INT_SourceEnable(INT_SOURCE_ADC_DF2);
}

void APP_Initialize() {    
    UI_IO_Init();
    I2S_Init();
    DAP_IO_Init();
    BM83_IO_Init();
    FLASH_Read();
    
    initBatADC();
    
    appData.state = APP_STATE_INIT_UI;
    //BM83_Module_Init(init_bm83_callback);
    //DAP_Chip_Init(init_dap_callback);
    UI_Main_Init(init_ui_callback);
}


/*******************************************************************************
 End of File
 */
