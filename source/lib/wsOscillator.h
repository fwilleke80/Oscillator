#ifndef _WS_OSCILLATOR_
#define _WS_OSCILLATOR_

#include "c4d.h"
#include "lib_noise.h"

enum OSCILLATORRANGE
{
	OSCILLATORRANGE_01	= 1,
	OSCILLATORRANGE_11	= 2
};

class wsOscillator
{
private:
	C4DNoise	*noise;

public:
	Real	GetSin(const Real &x, const OSCILLATORRANGE &range = OSCILLATORRANGE_01, const Bool &invert = FALSE);
	Real	GetCos(const Real &x, const OSCILLATORRANGE &range = OSCILLATORRANGE_01, const Bool &invert = FALSE);
	Real	GetSawtooth(const Real &x, const OSCILLATORRANGE &range = OSCILLATORRANGE_01, const Bool &invert = FALSE);
	Real	GetTriangle(const Real &x, const OSCILLATORRANGE &range = OSCILLATORRANGE_01, const Bool &invert = FALSE);
	Real	GetSquare(const Real &x, const OSCILLATORRANGE &range = OSCILLATORRANGE_01, const Bool &invert = FALSE);
	Real	GetPulse(const Real &x, const Real &pw, const Bool &random = FALSE, const OSCILLATORRANGE &range = OSCILLATORRANGE_01, const Bool &invert = FALSE);

	wsOscillator(void)
	{
		noise = C4DNoise::Alloc(54321);
	}

	~wsOscillator(void)
	{
		C4DNoise::Free(noise);
	}

};

#endif
