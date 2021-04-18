#include "c4d_tools.h"
#include "c4d_basebitmap.h"

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


Float Oscillator::GetSin(Float x, const WaveformParameters& parameters) const
{
	Float result = Sin(FreqToAngularVelocity(x));

	if (parameters.invert)
		result *= -1.0;

	if (parameters.valueRange == VALUERANGE::RANGE01)
		result = result * 0.5 + 0.5;

	return result;
}

Float Oscillator::GetCos(Float x, const WaveformParameters& parameters) const
{
	Float result = Cos(FreqToAngularVelocity(x));

	if (parameters.invert)
		result *= -1.0;

	if (parameters.valueRange == VALUERANGE::RANGE01)
		result = result * 0.5 + 0.5;

	return result;
}

Float	Oscillator::GetSawtooth(Float x, const WaveformParameters& parameters) const
{
	Float result = FMod(x, 1.0);

	if (parameters.invert)
		result = 1.0 - result;

	if (parameters.valueRange == VALUERANGE::RANGE11)
		result = result * 2.0 - 1.0;

	return result;
}

Float Oscillator::GetTriangle(Float x, const WaveformParameters& parameters) const
{
	Float result = ASin(Sin(FreqToAngularVelocity(x))) * TWOBYPI;


	if (parameters.valueRange == VALUERANGE::RANGE01)
	{
		result = result * 0.5 + 0.5;
		if (parameters.invert)
			result = 1.0 - result;
	}
	else
	{
		if (parameters.invert)
			result *= -1.0;
	}

	return result;
}

Float Oscillator::GetSquare(Float x, const WaveformParameters& parameters) const
{
	Float result = Sin(FreqToAngularVelocity(x)) > 0.0 ? 0.0 : 1.0;

	if (parameters.invert)
		result = 1.0 - result;

	if (parameters.valueRange == VALUERANGE::RANGE11)
		result = result * 2.0 - 1.0;

	return result;
}

Float Oscillator::GetPulse(Float x, const WaveformParameters& parameters) const
{
	// Generate Sin() [period 1, range 0..1] and quantize it
	Float result = ((Sin(FreqToAngularVelocity(x)) * 0.5 + 0.5) < parameters.pulseWidth) ? 0.0 : 1.0;

	if (parameters.invert)
		result = 1.0 - result;

	if (parameters.valueRange == VALUERANGE::RANGE11)
		result = result * 2.0 - 1.0;

	return result;
}

Float Oscillator::GetPulseRandom(Float x, const WaveformParameters& parameters) const
{
	Float result = (Turbulence(Vector(x), 5.0, true) < parameters.pulseWidth) ? 0.0 : 1.0;

	if (parameters.invert)
		result = 1.0 - result;

	if (parameters.valueRange == VALUERANGE::RANGE11)
		result = result * 2.0 - 1.0;

	return result;
}

Float Oscillator::GetAnalogSaw(Float x, const WaveformParameters& parameters) const
{
	Float result = 0.0;
	const Float fHarmonics = (Float)(parameters.harmonics + 1);
	const Float xValue = FreqToAngularVelocity(x);

	for (Float n = 1.0; n < fHarmonics; ++n)
	{
		result += Sin(n * xValue) / n;
	}

	result *= TWOBYPI;

	if (parameters.invert)
		result *= -1.0;

	if (parameters.valueRange == VALUERANGE::RANGE01)
		result = result * 0.5 + 0.5;

	return result;
}

BaseBitmap* Oscillator::RenderToBitmap(Int32 w, Int32 h, Oscillator::OSCTYPE oscType, const WaveformParameters& parameters)
{
	BaseBitmap* bmp = BaseBitmap::Alloc();
	if (!bmp)
		return nullptr;

	if (bmp->Init(w, h) != IMAGERESULT::OK)
	{
		BaseBitmap::Free(bmp);
		return nullptr;
	}

	// Background
	bmp->Clear(20, 20, 20);

	// Grid
	if (parameters.valueRange == Oscillator::VALUERANGE::RANGE11)
		bmp->Line(0, h / 2, w - 1, h / 2);
	else
		bmp->Line(0, h - 1, w - 1, h - 1);
	bmp->Line(0, 0, 0, h - 1);

	static const Float previewScaleX = 2.0;
	const Float iw1 = Inverse((Float)(w - 1));

	for (Int32 x = 0; x < w; ++x)
	{
		const Float xSample = (Float)x * iw1 * previewScaleX;
		Float y = (Int32)(SampleWaveform(xSample, oscType, parameters) * (Float)(h - 1));
		if (parameters.valueRange == Oscillator::VALUERANGE::RANGE11)
			y = y * 0.5 + h * 0.5;

		bmp->SetPixel(x, h - 1 - (Int32)y, 0, 255, 0);
	}

	return bmp;
}
