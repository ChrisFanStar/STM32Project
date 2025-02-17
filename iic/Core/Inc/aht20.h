/*
 * aht20.h
 *
 *  Created on: Jun 26, 2023
 *      Author: keysk
 */

#ifndef INC_AHT20_H_
#define INC_AHT20_H_

#include "i2c.h"

void AHT20_Init();

void AHT20_Read(float *Temperature, float *Humidity);

void AHT20_Measure();

void AHT20_Get();

void AHT20_Analysis(float *Temperature, float *Humidity);

#endif /* INC_AHT20_H_ */
