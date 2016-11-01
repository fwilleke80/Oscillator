///////////////////////////////////////////////////
// Oscillator Node Symbol Definition
///////////////////////////////////////////////////
// (c) 2010 Jack's Secret Stash
// All rights reserved.
///////////////////////////////////////////////////

#ifndef _gvOscillator_H_
#define _gvOscillator_H_

enum
{
	INPORT_X						= 10000,
	OUTPORT_VALUE				= 10001,

	OSC_FUNCTION				= 10002,
		FUNC_SINE						= 1,
		FUNC_COSINE					= 2,
		FUNC_SAWTOOTH				= 3,
		FUNC_SQUARE					= 4,
		FUNC_TRIANGLE				= 5,
		FUNC_PULSE					= 6,
		FUNC_PULSERND				= 7,
		FUNC_CUSTOM					= 8,
	OSC_RANGE						= 10003,
		RANGE_01						= 1,
		RANGE_11						= 2,
	OSC_CUSTOMFUNC			= 10004,
	OSC_INVERT					= 10005,
	OSC_PULSEWIDTH			= 10006,
	OSC_INPUTSCALE			= 10007,
	OSC_PREVIEW					= 10008
};

#endif