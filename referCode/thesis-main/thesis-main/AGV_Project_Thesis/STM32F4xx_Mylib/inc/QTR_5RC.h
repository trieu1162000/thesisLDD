/*
 * Encoder.h
 *
 *  Created on: Jun 21, 2020
 *      Author: THIEN
 */

#ifndef QTR_5RC_H_
#define QTR_5RC_H_

#include "main.h"

void QTRPins_OUTPUT_Mode(void);
void QTRPins_INPUT_Mode(void);
void QTRSensorsRead(uint16_t *sensorValues);
void QTRSensorsCalibrate(void);
uint16_t QTRSensorsReadCalibrated(uint16_t *_sensorValues);
uint16_t QTRSensorsReadLine(uint16_t *_sensorValues);
void TIM7_INT_Init(void);

#endif /* QTR_5RC_H_ */
