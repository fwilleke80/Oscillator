#include "wsOscillator.h"

/*
Information:
http://en.wikipedia.org/wiki/Sawtooth_wave
http://en.wikipedia.org/wiki/Square_wave
http://en.wikipedia.org/wiki/Triangle_wave
http://en.wikipedia.org/wiki/Wave_equation
*/

///////////////////////////////////////////////////////////////////////////
// Defines
///////////////////////////////////////////////////////////////////////////

// Round function
#define Round(x) Floor(x + RCO 0.5)

///////////////////////////////////////////////////////////////////////////
// Oscillator class members
///////////////////////////////////////////////////////////////////////////

// Return sine value
Real	wsOscillator::GetSin(const Real &x, const OSCILLATORRANGE &range, const Bool &invert)
{
	Real result = Sin(x * pi * RCO 2.0);

	if (invert)
		result *= RCO -1.0;

	if (range == OSCILLATORRANGE_01)
		result = result * RCO 0.5 + RCO 0.5;

	return result;
}

// Return cosine value
Real	wsOscillator::GetCos(const Real &x, const OSCILLATORRANGE &range, const Bool &invert)
{
	Real result = Cos(x * pi * RCO 2.0);

	if (invert)
		result *= RCO -1.0;

	if (range == OSCILLATORRANGE_01)
		result = result * RCO 0.5 + RCO 0.5;

	return result;
}

// Return sawtooth value
Real	wsOscillator::GetSawtooth(const Real &x, const OSCILLATORRANGE &range, const Bool &invert)
{
	Real result = FMod(x, RCO 1.0);

	if (invert)
		result = RCO 1.0 - result;

	if (range == OSCILLATORRANGE_11)
		result = result * RCO 2.0 - RCO 1.0;

	return result;
}

// Return triangle value
Real	wsOscillator::GetTriangle(const Real &x, const OSCILLATORRANGE &range, const Bool &invert)
{
	Real result = (FMod(x, RCO 1.0) < 0.5 ? FMod(x, RCO 1.0) : (RCO 1.0 - FMod(x, RCO 1.0))) * RCO 2.0;

	if (invert)
		result = RCO 1.0 - result;

	if (range == OSCILLATORRANGE_11)
		result = result * RCO 2.0 - RCO 1.0;

	return result;
}

// Return square value
Real	wsOscillator::GetSquare(const Real &x, const OSCILLATORRANGE &range, const Bool &invert)
{
	Real result = Round(sin(x * pi * RCO 2.0) * RCO 0.5 + RCO 0.5);

	if (invert)
		result = RCO 1.0 - result;

	if (range == OSCILLATORRANGE_11)
		result = result * RCO 2.0 - RCO 1.0;

	return result;
}

// Return pulse value
Real	wsOscillator::GetPulse(const Real &x, const Real &pw, const Bool &random, const OSCILLATORRANGE &range, const Bool &invert)
{
	// Generate sin() [period 1, range 0..1] and quantize it
	Real result;

	if (random)
		result = (noise->Noise(NOISE_TURBULENCE, TRUE, Vector(x)) < pw) ? RCO 0.0 : RCO 1.0;
	else
		result = ((sin(x * pi * RCO 2.0) * RCO 0.5 + RCO 0.5) < pw) ? RCO 0.0 : RCO 1.0;

	if (invert)
		result = RCO 1.0 - result;

	if (range == OSCILLATORRANGE_11)
		result = result * RCO 2.0 - RCO 1.0;

	return result;
}