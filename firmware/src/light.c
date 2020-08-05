/* ************************************************************************** */

#include "light.h"
#include "app.h"
#include "bm83.h"
#include "system/time/src/sys_time_local.h"

#define I2S_RECEIVE_BUFFER_LEN 0x2000
#define I2S_RECEIVE_BUFFER_PHYS_LOC 0x2000
#define I2S_RECEIVE_BUFFER_VIRT_LOC I2S_RECEIVE_BUFFER_PHYS_LOC | 0x80000000

#define clamp(x,l,u) ((x) < (l) ? (l) : (x) > (u) ? (u) : (x))

volatile int32_t i2s_receiveBuffer[I2S_RECEIVE_BUFFER_LEN] __attribute__((address(I2S_RECEIVE_BUFFER_VIRT_LOC), keep, coherent));
static uint16_t i2s_bufferReadPtr = 0;

bool light_on = true;
bool light_s2l = true;
float light_brightness = .125f;

static float light_baseEffect_periodLength;
static float light_baseEffect_periodStart;

static float s2l_chSum = 0.f;
static float s2l_currMaxL = 0.f;
static float s2l_currMaxH = 0.f;
static uint16_t s2l_intSampleCount = 0;

#define S2L_LPF_B0 0.00016820430755615234375f
#define S2L_LPF_B1 0.0003364086151123046875f
#define S2L_LPF_B2 0.00016820430755615234375f
#define S2L_LPF_A1 -1.9629800319671630859375f
#define S2L_LPF_A2 0.96365296840667724609375f
static float s2l_lpf_s1 = 0.f;
static float s2l_lpf_s2 = 0.f;

#define S2L_HPF_B0 0.418163299560546875f
#define S2L_HPF_B1 -0.83632671833038330078125f
#define S2L_HPF_B2 0.418163299560546875f
#define S2L_HPF_A1 -0.4629380702972412109375f
#define S2L_HPF_A2 0.209715366363525390625f
static float s2l_hpf_s1 = 0.f;
static float s2l_hpf_s2 = 0.f;

#define S2L_LONG_AVG_LEN 1000
#define S2L_SHORT_AVG_LEN 30
static float s2l_lastSamplesL[S2L_LONG_AVG_LEN] = { 0.f };
static float s2l_lastSamplesH[S2L_LONG_AVG_LEN] = { 0.f };
static uint16_t s2l_lastSamplePtr = 0;
static float s2l_longSumL = 0.f;
static float s2l_longSumH = 0.f;
static float s2l_shortSumL = 0.f;
static float s2l_shortSumH = 0.f;

static uint16_t s2l_lastLValley = 1000;
static uint16_t s2l_lastHPeak = 1000;
static uint16_t s2l_lastHSharpPeak = 1000;
static uint16_t s2l_lastBigKick = 1000;
static uint16_t s2l_lastTap = 1000;

static float s2l_sharpPeakThreshold = .4f;

#define S2L_PREV_BUMPS_LEN 300
static float s2l_prevBumpsL[S2L_PREV_BUMPS_LEN] = { 0.f };
static float s2l_prevBumpsH[S2L_PREV_BUMPS_LEN] = { 0.f };
static uint16_t s2l_prevBumpsPtr = 0;

static float s2l_mod = 0.f;

/*****************/
/* S2L functions */
/*****************/

void s2l_init() {
    s2l_chSum = 0.f;
    s2l_currMaxL = 0.f;
    s2l_currMaxH = 0.f;
    s2l_intSampleCount = 0;
    
    s2l_lpf_s1 = 0.f;
    s2l_lpf_s2 = 0.f;
    s2l_hpf_s1 = 0.f;
    s2l_hpf_s2 = 0.f;
    
    memset(s2l_lastSamplesL, 0, S2L_LONG_AVG_LEN * sizeof(float));
    memset(s2l_lastSamplesH, 0, S2L_LONG_AVG_LEN * sizeof(float));
    s2l_lastSamplePtr = 0;
    s2l_longSumL = 0.f;
    s2l_longSumH = 0.f;
    s2l_shortSumL = 0.f;
    s2l_shortSumH = 0.f;
    
    s2l_lastLValley = 1000;
    s2l_lastHPeak = 1000;
    s2l_lastHSharpPeak = 1000;
    s2l_lastBigKick = 1000;
    s2l_lastTap = 1000;
    
    s2l_sharpPeakThreshold = .4f;
    
    memset(s2l_prevBumpsL, 0, S2L_PREV_BUMPS_LEN * sizeof(float));
    memset(s2l_prevBumpsH, 0, S2L_PREV_BUMPS_LEN * sizeof(float));
    s2l_prevBumpsPtr = 0;
    
    s2l_mod = 0.f;
    
    i2s_bufferReadPtr = DCH4DPTR / 4;
}

float s2l_lpf_process(float input) {
    float sum1 = input - S2L_LPF_A1 * s2l_lpf_s1 - S2L_LPF_A2 * s2l_lpf_s2;
    float sum2 = S2L_LPF_B0 * sum1 + S2L_LPF_B1 * s2l_lpf_s1 + S2L_LPF_B2 * s2l_lpf_s2;
    s2l_lpf_s2 = s2l_lpf_s1;
    s2l_lpf_s1 = sum1;
    return sum2;
}

float s2l_hpf_process(float input) {
    float sum1 = input - S2L_HPF_A1 * s2l_hpf_s1 - S2L_HPF_A2 * s2l_hpf_s2;
    float sum2 = S2L_HPF_B0 * sum1 + S2L_HPF_B1 * s2l_hpf_s1 + S2L_HPF_B2 * s2l_hpf_s2;
    s2l_hpf_s2 = s2l_hpf_s1;
    s2l_hpf_s1 = sum1;
    return sum2;
}

void s2l_process_sample(float sample) {
    if (s2l_intSampleCount++ % 2 == 0) { //always average two samples together
        s2l_chSum = sample;
        return;
    }
    float avgSample = (sample + s2l_chSum) / 2.f;
    s2l_chSum = 0.f;
    float sampleL = fabsf(s2l_lpf_process(avgSample)); //process in LPF and remove sign, save if >max
    if (sampleL > s2l_currMaxL) s2l_currMaxL = sampleL;
    float sampleH = fabsf(s2l_hpf_process(avgSample)); //process in HPF and remove sign, save if >max
    if (sampleH > s2l_currMaxH) s2l_currMaxH = sampleH;
    
    if (s2l_intSampleCount < 20) return; //further processing only on max of 20 samples (= 10 mono samples)
    
    s2l_mod *= .999f; //mod decay
    
    float sL = s2l_currMaxL * s2l_currMaxL; //quadratically adjusted and scaled samples
    float sH = s2l_currMaxH * s2l_currMaxH * 10.f;
    
    uint16_t lastShortSamplePtr = (s2l_lastSamplePtr + S2L_LONG_AVG_LEN - S2L_SHORT_AVG_LEN) % S2L_LONG_AVG_LEN; //pointer to sample S2L_SHORT_AVG_LENGTH before current
    
    s2l_longSumL -= s2l_lastSamplesL[s2l_lastSamplePtr]; //process low sample in long average
    s2l_longSumL += sL;
    s2l_lastSamplesL[s2l_lastSamplePtr] = sL;
    
    s2l_longSumH -= s2l_lastSamplesH[s2l_lastSamplePtr]; //process high sample in long average
    s2l_longSumH += sH;
    s2l_lastSamplesH[s2l_lastSamplePtr++] = sH;
    s2l_lastSamplePtr %= S2L_LONG_AVG_LEN;
    
    s2l_shortSumL -= s2l_lastSamplesL[lastShortSamplePtr]; //process low sample in short average
    s2l_shortSumL += sL;
    
    s2l_shortSumH -= s2l_lastSamplesH[lastShortSamplePtr]; //process high sample in short average
    s2l_shortSumH += sH;
    
    float longAvgL = s2l_longSumL / (float)S2L_LONG_AVG_LEN; //calculate averages
    float longAvgH = s2l_longSumH / (float)S2L_LONG_AVG_LEN;
    float shortAvgL = s2l_shortSumL / (float)S2L_SHORT_AVG_LEN;
    float shortAvgH = s2l_shortSumH / (float)S2L_SHORT_AVG_LEN;
    
    float bumpL = clamp(shortAvgL - longAvgL, -1.f, 1.f); //calculate bumps
    float bumpH = clamp(shortAvgH - longAvgH, 0.f, 1.f);
    
    uint16_t offsetBumpPtr = s2l_prevBumpsPtr + S2L_PREV_BUMPS_LEN; //offset prev bump pointer to avoid underflow
    
    float pastLpf1 = s2l_prevBumpsL[(offsetBumpPtr - 240) % S2L_PREV_BUMPS_LEN]; //detect low valley
    float pastLpf2 = s2l_prevBumpsL[(offsetBumpPtr - 180) % S2L_PREV_BUMPS_LEN];
    float pastLpf3 = s2l_prevBumpsL[(offsetBumpPtr - 120) % S2L_PREV_BUMPS_LEN];
    float pastLpf4 = s2l_prevBumpsL[(offsetBumpPtr - 60) % S2L_PREV_BUMPS_LEN];
    if (pastLpf1 + pastLpf2 + pastLpf3 + pastLpf4 + bumpL < -.2f) {
        s2l_lastLValley = 0;
    }
    
    float pastHpf1 = s2l_prevBumpsH[(offsetBumpPtr - 150) % S2L_PREV_BUMPS_LEN]; //detect high peak and sharp peak
    float pastHpf2 = s2l_prevBumpsH[(offsetBumpPtr - 90) % S2L_PREV_BUMPS_LEN];
    float pastHpf3 = s2l_prevBumpsH[(offsetBumpPtr - 75) % S2L_PREV_BUMPS_LEN];
    float pastHpf4 = s2l_prevBumpsH[(offsetBumpPtr - 60) % S2L_PREV_BUMPS_LEN];
    float pastHpf5 = s2l_prevBumpsH[(offsetBumpPtr - 45) % S2L_PREV_BUMPS_LEN];
    float pastHpf6 = s2l_prevBumpsH[(offsetBumpPtr - 30) % S2L_PREV_BUMPS_LEN];
    if (pastHpf2 + 2.f * pastHpf3 + pastHpf4 - 2.f * (pastHpf1 + bumpH) > .4f) {
        s2l_lastHPeak = 0;
    }
    float sharpPeakMetric = pastHpf3 + 4.f * pastHpf4 + pastHpf5 - 3.f * pastHpf2 - pastHpf6 - 8.f * pastHpf1 - 4.f * bumpH;
    if (sharpPeakMetric > s2l_sharpPeakThreshold) {
        s2l_lastHSharpPeak = 0;
        float proposedNewThreshold = sharpPeakMetric / 3.f;
        if (s2l_sharpPeakThreshold < proposedNewThreshold) s2l_sharpPeakThreshold = proposedNewThreshold;
    }
    
    if ((s2l_lastHPeak == 0 && s2l_lastLValley < 150) || (s2l_lastHPeak < 150 && s2l_lastLValley == 0)) { //detect big kick
        s2l_mod = 1.f;
        s2l_lastBigKick = 0;
    } else if (s2l_lastHSharpPeak == 0 && s2l_lastTap > 400 && s2l_lastBigKick > 1000) { //detect tap
        float dMod = sharpPeakMetric > 1.5f * s2l_sharpPeakThreshold ? .8f : .6f;
        if (s2l_mod < dMod) s2l_mod = dMod;
        s2l_lastTap = 0;
    }
    
    s2l_prevBumpsL[s2l_prevBumpsPtr] = bumpL; //save bumps
    s2l_prevBumpsH[s2l_prevBumpsPtr++] = bumpH;
    s2l_prevBumpsPtr %= S2L_PREV_BUMPS_LEN;
    
    s2l_lastLValley++;
    if (s2l_lastLValley > 1000) s2l_lastLValley = 1000;
    s2l_lastHPeak++;
    if (s2l_lastHPeak > 1000) s2l_lastHPeak = 1000;
    s2l_lastHSharpPeak++;
    if (s2l_lastHSharpPeak > 1000) s2l_lastHSharpPeak = 1000;
    s2l_lastBigKick++;
    if (s2l_lastBigKick > 10000) s2l_lastBigKick = 10000;
    s2l_lastTap++;
    if (s2l_lastTap > 10000) s2l_lastTap = 10000;
    s2l_sharpPeakThreshold *= .9999f;
    if (s2l_sharpPeakThreshold < .4f) s2l_sharpPeakThreshold = .4f;
    
    s2l_intSampleCount = 0;
    s2l_currMaxL = 0.f;
    s2l_currMaxH = 0.f;
}

/*********************/
/* Utility functions */
/*********************/

void light_setRaw(uint16_t r, uint16_t g, uint16_t b) {
    OCMP2_CompareSecondaryValueSet(g);
    OCMP5_CompareSecondaryValueSet(r);
    OCMP6_CompareSecondaryValueSet(b);
}

void light_set(float r, float g, float b) {
    uint16_t ri = (uint16_t)(2047.f * clamp(r, 0.f, 1.f) * light_brightness);
    uint16_t gi = (uint16_t)(2047.f * clamp(g, 0.f, 1.f) * light_brightness);
    uint16_t bi = (uint16_t)(2047.f * clamp(b, 0.f, 1.f) * light_brightness);
    light_setRaw(ri, gi, bi);
}



void Light_On() {
    if (light_on) return;
    light_baseEffect_periodStart = SYS_TIME_CounterGet();
    if (light_s2l) s2l_init();
    light_on = true;
}

void Light_Off() {
    if (!light_on) return;
    light_setRaw(0, 0, 0);
    light_on = false;
}

void Light_S2L_Enable() {
    if (light_s2l) return;
    s2l_init();
    light_s2l = true;
}

void Light_S2L_Disable() {
    if (!light_s2l) return;
    s2l_mod = 0.f;
    light_s2l = false;
}

void Light_Tasks() {
    if (!light_on) return;
    
    float tick = (float)SYS_TIME_CounterGet();
    
    if (bm83_state == BM83_OFF) {
        light_setRaw(0, 0, 0);
        light_baseEffect_periodStart = tick;
        return;
    }
    
    float periodPos = (tick - light_baseEffect_periodStart) / light_baseEffect_periodLength;
    if (periodPos >= 1.f) {
        light_baseEffect_periodStart = tick;
        float intPart;
        periodPos = modff(periodPos, &intPart);
    }
    float periodMod = .25f * sinf(2.f * M_PI * periodPos);
    
    if (light_s2l) {
        uint16_t counter = 0; //safety counter
        uint16_t writePtr = DCH4DPTR / 4;
        while (writePtr != i2s_bufferReadPtr && writePtr - i2s_bufferReadPtr < 0x1000 && counter < 1000) {
            int32_t rawSample = i2s_receiveBuffer[i2s_bufferReadPtr++];
            i2s_bufferReadPtr %= I2S_RECEIVE_BUFFER_LEN;
            float fSample = (float)rawSample / (float)0x7fffff;
            s2l_process_sample(fSample);
            counter++;
        }
    }
    
    float r = s2l_mod;
    float g = .5f + periodMod - s2l_mod;
    float b = .5f - periodMod - s2l_mod;
    
    light_set(r, g, b);
}

/****************************/
/* Initialization functions */
/****************************/

void i2s_init() {
    IEC3bits.SPI1EIE = 0;
    IEC3bits.SPI1RXIE = 0;
    IEC3bits.SPI1TXIE = 0; //disable SPI interrupts
    IEC4bits.DMA4IE = 0; //disable DMA interrupt
    
    IFS3bits.SPI1RXIF = 0; //reset SPI receive interrupt flag
    
    SPI1CON = 0; //reset SPI config and disable SPI module
    SPI1CON2 = 0; //reset SPI config 2
    SPI1BUF = 0; //clear SPI RX buffer
    SPI1CONbits.DISSDO = 1; //disable SDO
    SPI1CONbits.ENHBUF = 1; //enable enhanced SPI buffer
    SPI1STATbits.SPIROV = 0; //clear SPI RX overflow flag
    SPI1CON2bits.AUDMOD = 0b00; //SPI I2S mode
    SPI1CON2bits.AUDEN = 1; //enable SPI audio mode
    //SPI1CONbits.FRMPOL = 0;
    SPI1CON2bits.IGNROV = 1; //ignore receive overflow (non-critical application)
    SPI1CONbits.MSTEN = 0; //SPI slave mode
    SPI1CONbits.CKP = 1; //inverted SPI clock polarity (required for I2S)
    SPI1CONbits.MODE32 = 1;
    SPI1CONbits.MODE16 = 1; //I2S 24-bit data, 64-bit frame mode
    SPI1CONbits.SRXISEL = 0b01; //interrupt when buffer not empty
    SPI1CON2bits.SPISGNEXT = 1; //sign extend data
    
    DCH4CON = 0; //reset DMA config and disable channel
    DCH4ECON = 0; //reset DMA event config
    DCH4INT = 0; //disable and clear all DMA channel interrupts
    DCH4ECONbits.CHAIRQ = 0xFF; //no IRQ for DMA abort
    DCH4ECONbits.CHSIRQ = _SPI1_RX_VECTOR; //DMA start on SPI RX interrupt
    DCH4ECONbits.SIRQEN = 1; //enable DMA start IRQ
    DCH4SSA = 0x1F821020; //DMA source: SPI1BUF (physical address)
    DCH4DSA = I2S_RECEIVE_BUFFER_PHYS_LOC; //DMA dest: receive buffer (physical address)
    DCH4SSIZ = 4; //DMA source size: 4 bytes
    DCH4DSIZ = 4 * I2S_RECEIVE_BUFFER_LEN; //DMA dest size: (4 * buffer length) bytes
    DCH4CSIZ = 4; //DMA cell size: 8 bytes (stereo sample)
    DCH4CONbits.CHAEN = 1; //DMA auto-enable
    DCH4CONbits.CHEN = 1; //enable DMA channel
    
    SPI1CONbits.ON = 1; //enable SPI/I2S
}

void Light_Init() {
    i2s_init();
    
    light_setRaw(0, 0, 0);
    OCMP2_Enable();
    OCMP5_Enable();
    OCMP6_Enable();
    TMR2_Start();
    
    light_baseEffect_periodLength = SYS_TIME_MSToCount(15000);
}



/* *****************************************************************************
 End of File
 */
