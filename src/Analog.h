/*
Analog input scaling V1.0

Input OGR  / UGR : Values from Input Device e.g. 0-4096
Output OGR / UGR : Values physical e.g. 0-150°C

G. Sebb, 24.10.2020

*/

enum Erorstatus{
ErrorInputValue = 1,
ErrorOutputValue
};

float analogInScale
(int AnalogIN, // input
int InputOGR, // max value input
int InputUGR, // min value input
float OutputOGR, // max value phys. output
float OutputUGR, // min value phys. output
int& Error);
