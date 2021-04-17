#include "c4d_tools.h"

#include "oscillator.h"


/*
 Information:

 http://en.wikipedia.org/wiki/Sawtooth_wave
 http://en.wikipedia.org/wiki/Square_wave
 http://en.wikipedia.org/wiki/Triangle_wave
 http://en.wikipedia.org/wiki/Wave_equation
 */


static const Float TWOBYPI = 2.0 / PI;

MAXON_ATTRIBUTE_FORCE_INLINE Float FreqToAngularVelocity(Float h)
{
	return h * 2.0 * PI;
}


Float Oscillator::GetSin(Float x, VALUERANGE valueRange, Bool invert) const
{
	Float result = Sin(FreqToAngularVelocity(x));

	if (invert)
		result *= -1.0;

	if (valueRange == VALUERANGE::RANGE01)
		result = result * 0.5 + 0.5;

	return result;
}

Float Oscillator::GetCos(Float x, VALUERANGE valueRange, Bool invert) const
{
	Float result = Cos(FreqToAngularVelocity(x));

	if (invert)
		result *= -1.0;

	if (valueRange == VALUERANGE::RANGE01)
		result = result * 0.5 + 0.5;

	return result;
}

Float	Oscillator::GetSawtooth(Float x, VALUERANGE valueRange, Bool invert) const
{
	Float result = FMod(x, 1.0);

	if (invert)
		result = 1.0 - result;

	if (valueRange == VALUERANGE::RANGE11)
		result = result * 2.0 - 1.0;

	return result;
}

Float Oscillator::GetTriangle(Float x, VALUERANGE valueRange, Bool invert) const
{
	Float result = ASin(Sin(FreqToAngularVelocity(x))) * TWOBYPI;


	if (valueRange == VALUERANGE::RANGE01)
	{
		result = result * 0.5 + 0.5;
		if (invert)
			result = 1.0 - result;
	}
	else
	{
		if (invert)
			result *= -1.0;
	}

	return result;
}

Float Oscillator::GetSquare(Float x, VALUERANGE valueRange, Bool invert) const
{
	Float result = Sin(FreqToAngularVelocity(x)) > 0.0 ? 0.0 : 1.0;

	if (invert)
		result = 1.0 - result;

	if (valueRange == VALUERANGE::RANGE11)
		result = result * 2.0 - 1.0;

	return result;
}

Float Oscillator::GetPulse(Float x, Float pulseWidth, Bool random, VALUERANGE valueRange, Bool invert) const
{
	Float result;

	if (random)
	{
		const Float turbValue = Noise(Vector(x));
		result = (turbValue < pulseWidth) ? 0.0 : 1.0;
	}
	else
	{
		// Generate Sin() [period 1, range 0..1] and quantize it
		result = ((Sin(FreqToAngularVelocity(x)) * 0.5 + 0.5) < pulseWidth) ? 0.0 : 1.0;
	}

	if (invert)
		result = 1.0 - result;

	if (valueRange == VALUERANGE::RANGE11)
		result = result * 2.0 - 1.0;

	return result;
}

Float Oscillator::GetAnalogSaw(Float x, UInt harmonics, VALUERANGE valueRange, Bool invert) const
{
	Float result = 0.0;
	const Float fHarmonics = (Float)harmonics;
	const Float xValue = FreqToAngularVelocity(x);

	for (Float n = 1.0; n < fHarmonics; ++n)
	{
		result += Sin(n * xValue) / n;
	}

	if (invert)
		result *= -1.0;

	if (valueRange == VALUERANGE::RANGE01)
		result = result * 0.5 + 0.5;

	return result;
}
