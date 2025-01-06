
/**
 * @brief Analog input scaling V1.0
 * G. Sebb, 24.10.2020
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
