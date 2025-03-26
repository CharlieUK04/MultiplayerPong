

//Headers to include
#include "MainUtils.h"

//Lerp converts a float from 0 to 1 and converts it to be between 2 numbers (0 would be min, 1 would be max)
float Lerp(float min, float max, float value)
{
	value = value < 0 ? 0 : value > 1 ? 1 : value;	//Clamps the input value between 0 and 1
	return min * (1.0 - value) + (max * value);
}

//Inverse lerp converts a value between 2 numbers into a float from 0 to 1 (min would be 0, max would be 1)
float InverseLerp(float min, float max, float value)
{
	//Outputs the min if the input is smaller than the min and the max if the input is bigger than the max
	if (value < min)
		return 0;
	else if (value > max)
		return 1;
	else
		return (value - min) / (max - min);
}
