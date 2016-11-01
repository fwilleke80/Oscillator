///////////////////////////////////////////////////
// Oscillator Node Description Resource
///////////////////////////////////////////////////
// (c) 2010 Jack's Secret Stash
// All rights reserved.
///////////////////////////////////////////////////

CONTAINER gvOscillator
{
	NAME gvOscillator;
	INCLUDE GVbase;

	GROUP ID_GVPROPERTIES
	{
		LONG					OSC_FUNCTION	{ CYCLE { FUNC_SAWTOOTH; FUNC_SQUARE; FUNC_TRIANGLE; FUNC_PULSE; FUNC_PULSERND; FUNC_SINE; FUNC_COSINE; FUNC_CUSTOM; } }
		LONG					OSC_RANGE			{ CYCLE { RANGE_01; RANGE_11; } }
		BOOL					OSC_INVERT		{  }
		SPLINE				OSC_CUSTOMFUNC		{	HIDDEN;
																			INPORT; 
																			SHOWGRID_H; 
																			SHOWGRID_V; 
																			GRIDSIZE_H 10; 
																			GRIDSIZE_V 10; 
																		  
																			HAS_PRESET_BTN; 
																		  
																			MINSIZE_H 100;
																			MINSIZE_V 90; 
																		  
																			EDIT_H; 
																			EDIT_V; 
																		  
																			HAS_ROUND_SLIDER;
																		  
																			X_MIN 0.0; 
																			X_MAX 1.0; 
																		  
																			Y_MIN 0.0; 
																			Y_MAX 1.0; 
																		  
																			X_STEPS 2; 
																			Y_STEPS 2; 
																		}
		SEPARATOR										{  }

		CURVEPREVIEW	OSC_PREVIEW		{  }
	}

	GROUP ID_GVPORTS
	{
		REAL			INPORT_X					{ INPORT; STATICPORT; CREATEPORT; EDITPORT; }
		REAL			OSC_INPUTSCALE		{ INPORT; EDITPORT; UNIT PERCENT; MIN 0.0; STEP 0.1; }
		REAL			OSC_PULSEWIDTH		{ HIDDEN; INPORT; UNIT REAL; MIN 0.0; MAX 1.0; STEP 0.001; }

		REAL			OUTPORT_VALUE			{ OUTPORT; STATICPORT; CREATEPORT; }
	}
}
