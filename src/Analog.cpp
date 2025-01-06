/**
 * @file Analog.cpp
 * @author Gerry Sebb
 * @brief Analog input scaling
 * @version 1.1
 * @date 2025-01-06
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "Analog.h"

/**
 * @brief Analog input scaling
 * Input OGR  / UGR : Values from Input Device e.g. 0-4096
 * Output OGR / UGR : Values physical e.g. 0-150°C
 * @param AnalogIN 
 * @param InputOGR 
 * @param InputUGR 
 * @param OutputOGR 
 * @param OutputUGR 
 * @param Error 
 * @return float 
 */

float analogInScale(int AnalogIN, int InputOGR, int InputUGR, float OutputOGR, float OutputUGR, int& Error)
{
	Error = 0;
	if (InputOGR == InputUGR || InputUGR > InputOGR) {
		Error = ErrorInputValue;			// Input Value Error
		return 0;
	}
	if (OutputOGR == OutputUGR) {
		Error = ErrorOutputValue;				// Output Value Error
		return 0;
	}
	float Steigung = (OutputOGR - OutputUGR) / (InputOGR - InputUGR);
	float Offset = OutputUGR - Steigung * InputUGR;
	float ValueOut = AnalogIN * Steigung + Offset;
	return ValueOut;
}
