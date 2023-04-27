/*
Analog input scaling V1.0

Input OGR  / UGR : Values from Input Device e.g. 0-4096
Output OGR / UGR : Values physical e.g. 0-150°C

G. Sebb, 24.10.2020

*/

#include "Analog.h"

float analogInScale(int AnalogIN, int InputOGR, int InputUGR, float OutputOGR, float OutputUGR, int& Error)
{
	Error = 0;
	if (InputOGR == InputUGR) {
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
