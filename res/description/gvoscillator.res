CONTAINER gvOscillator
{
	NAME gvOscillator;
	INCLUDE GVbase;

	GROUP ID_GVPROPERTIES
	{
		BITMAPBUTTON OSC_WAVEFORMPREVIEW { };
		LONG OSC_FUNCTION
		{
			CYCLE
			{
				FUNC_SAWTOOTH;
				FUNC_SQUARE;
				FUNC_TRIANGLE;
				FUNC_PULSE;
				FUNC_PULSERND;
				FUNC_SINE;
				FUNC_COSINE;
				FUNC_SAW_ANALOG;
				FUNC_SHARKTOOTH_ANALOG;
				FUNC_SQUARE_ANALOG;
				FUNC_ANALOG;
				-1;
				FUNC_CUSTOM;
			}
		}
		LONG OSC_RANGE
		{
			CYCLE
			{
				RANGE_01;
				RANGE_11;
			}
		}
		BOOL OSC_INVERT {  }
		LONG OSC_HARMONICS { MIN 1; }
		SPLINE OSC_CUSTOMFUNC
		{
			HIDDEN;
			INPORT;

			SHOWGRID_H;
			SHOWGRID_V;

			MINSIZE_H 100;
			MINSIZE_V 90;

			EDIT_H;
			EDIT_V;

			X_MIN 0.0;
			X_MAX 1.0;

			Y_MIN 0.0;
			Y_MAX 1.0;

			X_STEPS 0.1;
			Y_STEPS 0.1;

			OPTIMAL_X_MIN 0.0;
			OPTIMAL_X_MAX 1.0;
			OPTIMAL_Y_MIN 0.0;
			OPTIMAL_Y_MAX 1.0;

			USE_OPTIMAL_RANGE;
		}
	}

	GROUP ID_GVPORTS
	{
		REAL INPORT_X { INPORT; NEEDCONNECTION; STATICPORT; CREATEPORT; }
		REAL OSC_INPUTSCALE	 { INPORT; EDITPORT; UNIT PERCENT; MIN 0.0; STEP 0.1; }
		REAL OSC_PULSEWIDTH { INPORT; EDITPORT; UNIT REAL; MIN 0.0; MAX 1.0; STEP 0.001; }
		LONG OSC_HARMONICS { INPORT; EDITPORT; MIN 1; }

		REAL OUTPORT_VALUE { OUTPORT; STATICPORT; CREATEPORT; }
	}
}
