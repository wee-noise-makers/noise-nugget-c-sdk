#pragma once

//  Page select register
#define AIC3X_PAGE_SELECT 0
//  Software reset register
#define AIC3X_RESET 1
//  Codec Sample rate select register
#define AIC3X_SAMPLE_RATE_SEL_REG 2
//  PLL progrramming register A
#define AIC3X_PLL_PROGA_REG 3
//  PLL progrramming register B
#define AIC3X_PLL_PROGB_REG 4
//  PLL progrramming register C
#define AIC3X_PLL_PROGC_REG 5
//  PLL progrramming register D
#define AIC3X_PLL_PROGD_REG 6
//  Codec datapath setup register
#define AIC3X_CODEC_DATAPATH_REG 7
//  Audio serial data interface control register A
#define AIC3X_ASD_INTF_CTRLA 8
//  Audio serial data interface control register B
#define AIC3X_ASD_INTF_CTRLB 9
//  Audio serial data interface control register C
#define AIC3X_ASD_INTF_CTRLC 10
//  Audio overflow status and PLL R value programming register
#define AIC3X_OVRF_STATUS_AND_PLLR_REG 11
//  Audio codec digital filter control register
#define AIC3X_CODEC_DFILT_CTRL 12
//  Headset/button press detection register
#define AIC3X_HEADSET_DETECT_CTRL_A 13
#define AIC3X_HEADSET_DETECT_CTRL_B 14
//  ADC PGA Gain control registers
#define LADC_VOL 15
#define RADC_VOL 16
//  MIC3 control registers
#define MIC3LR_2_LADC_CTRL 17
#define MIC3LR_2_RADC_CTRL 18
//  Line1 Input control registers
#define LINE1L_2_LADC_CTRL 19
#define LINE1R_2_LADC_CTRL 21
#define LINE1R_2_RADC_CTRL 22
#define LINE1L_2_RADC_CTRL 24
//  Line2 Input control registers
#define LINE2L_2_LADC_CTRL 20
#define LINE2R_2_RADC_CTRL 23
//  MICBIAS Control Register
#define MICBIAS_CTRL 25

//  AGC Control Registers A, B, C
#define LAGC_CTRL_A 26
#define LAGC_CTRL_B 27
#define LAGC_CTRL_C 28
#define RAGC_CTRL_A 29
#define RAGC_CTRL_B 30
#define RAGC_CTRL_C 31

//  DAC Power and Left High Power Output control registers
#define DAC_PWR 37
#define HPLCOM_CFG 37
//  Right High Power Output control registers
#define HPRCOM_CFG 38
//  High Power Output Stage Control Register
#define HPOUT_SC 40
//  DAC Output Switching control registers
#define DAC_LINE_MUX 41
//  High Power Output Driver Pop Reduction registers
#define HPOUT_POP_REDUCTION 42
//  DAC Digital control registers
#define LDAC_VOL 43
#define RDAC_VOL 44
//  Left High Power Output control registers
#define LINE2L_2_HPLOUT_VOL 45
#define PGAL_2_HPLOUT_VOL 46
#define DACL1_2_HPLOUT_VOL 47
#define LINE2R_2_HPLOUT_VOL 48
#define PGAR_2_HPLOUT_VOL 49
#define DACR1_2_HPLOUT_VOL 50
#define HPLOUT_CTRL 51
//  Left High Power COM control registers
#define LINE2L_2_HPLCOM_VOL 52
#define PGAL_2_HPLCOM_VOL 53
#define DACL1_2_HPLCOM_VOL 54
#define LINE2R_2_HPLCOM_VOL 55
#define PGAR_2_HPLCOM_VOL 56
#define DACR1_2_HPLCOM_VOL 57
#define HPLCOM_CTRL 58
//  Right High Power Output control registers
#define LINE2L_2_HPROUT_VOL 59
#define PGAL_2_HPROUT_VOL 60
#define DACL1_2_HPROUT_VOL 61
#define LINE2R_2_HPROUT_VOL 62
#define PGAR_2_HPROUT_VOL 63
#define DACR1_2_HPROUT_VOL 64
#define HPROUT_CTRL 65
//  Right High Power COM control registers
#define LINE2L_2_HPRCOM_VOL 66
#define PGAL_2_HPRCOM_VOL 67
#define DACL1_2_HPRCOM_VOL 68
#define LINE2R_2_HPRCOM_VOL 69
#define PGAR_2_HPRCOM_VOL 70
#define DACR1_2_HPRCOM_VOL 71
#define HPRCOM_CTRL 72
//  Mono Line Output Plus/Minus control registers
#define LINE2L_2_MONOLOPM_VOL 73
#define PGAL_2_MONOLOPM_VOL 74
#define DACL1_2_MONOLOPM_VOL 75
#define LINE2R_2_MONOLOPM_VOL 76
#define PGAR_2_MONOLOPM_VOL 77
#define DACR1_2_MONOLOPM_VOL 78
#define MONOLOPM_CTRL 79
//  Left Line Output Plus/Minus control registers
#define LINE2L_2_LLOPM_VOL 80
#define PGAL_2_LLOPM_VOL 81
#define DACL1_2_LLOPM_VOL 82
#define LINE2R_2_LLOPM_VOL 83
#define PGAR_2_LLOPM_VOL 84
#define DACR1_2_LLOPM_VOL 85
#define LLOPM_CTRL 86
//  Right Line Output Plus/Minus control registers
#define LINE2L_2_RLOPM_VOL 87
#define PGAL_2_RLOPM_VOL 88
#define DACL1_2_RLOPM_VOL 89
#define LINE2R_2_RLOPM_VOL 90
#define PGAR_2_RLOPM_VOL 91
#define DACR1_2_RLOPM_VOL 92
#define RLOPM_CTRL 93

#define MODULE_POWER_STATUS 94

//  GPIO/IRQ registers
#define AIC3X_STICKY_IRQ_FLAGS_REG 96
#define AIC3X_RT_IRQ_FLAGS_REG 97

#define AIC3X_CLOCK_REG 101
//  Clock generation control register
#define AIC3X_CLKGEN_CTRL_REG 102
//  New AGC registers
#define LAGCN_ATTACK 103
#define LAGCN_DECAY 104
#define RAGCN_ATTACK 105
#define RAGCN_DECAY 106
//  New Programmable ADC Digital Path and I2C Bus Condition Register
#define NEW_ADC_DIGITALPATH 107
//  Passive Analog Signal Bypass Selection During Powerdown Register
#define PASSIVE_BYPASS 108
//  DAC Quiescent Current Adjustment Register
#define DAC_ICC_ADJ 109
