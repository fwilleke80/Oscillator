#ifndef TOSCILLATOR_H__
#define TOSCILLATOR_H__

enum
{
	OSC_FUNCTION           = 10002,
		FUNC_SINE              = 0,
		FUNC_COSINE            = 1,
		FUNC_SAWTOOTH         = 2,
		FUNC_SQUARE            = 3,
		FUNC_TRIANGLE          = 4,
		FUNC_PULSE             = 5,
		FUNC_PULSERND          = 6,
		FUNC_SAW_ANALOG        = 7,
		FUNC_SHARKTOOTH_ANALOG = 8,
		FUNC_SQUARE_ANALOG     = 9,
		FUNC_ANALOG            = 10,
		FUNC_CUSTOM            = 100,
	OSC_RANGE              = 10003,
		RANGE_01               = 0,
		RANGE_11               = 1,
	OSC_CUSTOMFUNC         = 10004,
	OSC_INVERT             = 10005,
	OSC_PULSEWIDTH         = 10006,
	OSC_INPUTSCALE         = 10007,
	OSC_HARMONICS          = 10008,
	OSC_HARMONICS_INTERVAL = 10009,
	OSC_HARMONICS_OFFSET   = 10010,

	OSC_WAVEFORMPREVIEW = 10100
};

#endif // TOSCILLATOR_H__