#ifndef OSCILLATOR_H__
#define OSCILLATOR_H__

#include "customgui_splinecontrol.h"
#include "c4d_basebitmap.h"
#include "c4d_tools.h"
#include "c4d_general.h"

#include "ge_prepass.h"

/*
 Information:

 http://en.wikipedia.org/wiki/Sawtooth_wave
 http://en.wikipedia.org/wiki/Square_wave
 http://en.wikipedia.org/wiki/Triangle_wave
 http://en.wikipedia.org/wiki/Wave_equation
 */


// Waveform preview settings
static const Int32 g_previewAreaWidth = 400; ///< Waveform preview width
static const Int32 g_previewAreaHeight = 100; ///< Waveform preview height
static const Int32 g_previewAreaOversample = 2; ///< Waveform preview oversampling
static const Float g_previewAreaScaleX = 2.0; ///< Scaling of the preview's X axis
static const Int32 g_previewAreaVerticalGridLines = 8; ///< Vertical grid lines in preview
static const Int32 g_previewAreaTextWidth = 8; ///< Text font Width
static const Int32 g_previewAreaTextHeight = 12; ///< Text font Height
static const Int32 g_previewAreaTextMarginH = 2; ///< Horizontal margin of text
static const Int32 g_previewAreaTextMarginV = 4; ///< Vertical margin of text
static const Int32 g_previewAreaColor_bg_r = 32; ///< Background color
static const Int32 g_previewAreaColor_bg_g = 32; ///< Background color
static const Int32 g_previewAreaColor_bg_b = 32; ///< Background color
static const Int32 g_previewAreaColor_grid1_r = 8; ///< Normal grid line color
static const Int32 g_previewAreaColor_grid1_g = 48; ///< Normal grid line color
static const Int32 g_previewAreaColor_grid1_b = 8; ///< Normal grid line color
static const Int32 g_previewAreaColor_grid2_r = 32; ///< Bold grid line color
static const Int32 g_previewAreaColor_grid2_g = 128; ///< Bold grid line color
static const Int32 g_previewAreaColor_grid2_b = 16; ///< Bold grid line color
static const Int32 g_previewAreaColor_text_r = 32; ///< Bold grid line color
static const Int32 g_previewAreaColor_text_g = 160; ///< Bold grid line color
static const Int32 g_previewAreaColor_text_b = 16; ///< Bold grid line color
static const Int32 g_previewAreaColor_wave_r = 32; ///< Waveform color
static const Int32 g_previewAreaColor_wave_g = 255; ///< Waveform color
static const Int32 g_previewAreaColor_wave_b = 16; ///< Waveform color

static const Float TWOBYPI = 2.0 / PI; ///< We need this in some calculations

///
/// \brief Convert frequency to angular velocity (as input for Sin() and related functions)
///
MAXON_ATTRIBUTE_FORCE_INLINE Float FreqToAngularVelocity(Float f)
{
	return f * PI2;
}

///
/// \brief Draws an X into a BaseBitmap
///
static void DrawX(BaseBitmap* bmp, Int32 x, Int32 y, Int32 w, Int32 h)
{
	bmp->Line(x, y, x + w, y + h); // Top left -> bottom right
	bmp->Line(x, y + h, x + w, y); // Bottom left -> top right
};

///
/// \brief Draws a Y into a BaseBitmap
///
static void DrawY(BaseBitmap* bmp, Int32 x, Int32 y, Int32 w, Int32 h)
{
	bmp->Line(x, y + h, x + w, y); // Bottom left -> top right
	bmp->Line(x, y, x + w / 2, y + h / 2); // Top left -> center
};

///
/// \brief Returns true if two SplineDatas are equal
///
MAXON_ATTRIBUTE_FORCE_INLINE Bool EqualSplineDatas(SplineData* sp1, SplineData* sp2)
{
	if (!sp1 || !sp2)
		return false;

	// Compare knot counts
	if (sp1->GetKnotCount() != sp2->GetKnotCount())
		return false;

	// Compare knots
	for (Int32 knotIndex = 0; knotIndex < sp1->GetKnotCount(); ++knotIndex)
	{
		CustomSplineKnot* k1 = sp1->GetKnot(knotIndex);
		CustomSplineKnot* k2 = sp2->GetKnot(knotIndex);
		if (k1 != k2)
			return false;
	}

	return true;
}

///
/// \brief A class that generates waveforms
///
class Oscillator
{
public:
	///
	/// \brief Available waveform types
	///
	enum class WAVEFORMTYPE
	{
		SINE = 0,
		COSINE = 1,
		SAWTOOTH = 2,
		SQUARE = 3,
		TRIANGLE = 4,
		PULSE = 5,
		PULSERND = 6,
		SAW_ANALOG = 7,
		SHARKTOOTH_ANALOG = 8,
		SQUARE_ANALOG = 9,
		ANALOG = 10,
		CUSTOMSPLINE = 100
	} MAXON_ENUM_LIST_CLASS(WAVEFORMTYPE);

	///
	/// \brief Output value range
	///
	enum class VALUERANGE
	{
		RANGE01 = 0,
		RANGE11 = 1
	} MAXON_ENUM_LIST_CLASS(VALUERANGE);

	///
	/// \brief Parameters for waveform generation
	///
	struct WaveformParameters
	{
		VALUERANGE valueRange; ///< The output value range. Either [0 .. 1] or [-1 .. 1]
		Bool invert; ///< If this is true, the output phase will be inverted
		Float pulseWidth; ///< Defines the pulse width of GetPulse(). [0 .. 1].
		UInt harmonics; ///< Defines the nmber of harmonics in GetAnalogX(). [1 .. infinite]
		Float harmonicInterval; ///< Harmonic multiplication will be increased by this value for each harmonic
		Float harmonicIntervalOffset; ///< Harmonic multiplication will start with this value before it is increased
		SplineData* customCurve; ///< Pointer to a spline for the custom waveform

		/// \brief Default vonstructor
		WaveformParameters() : valueRange(VALUERANGE::RANGE01), invert(false), pulseWidth(0.0), harmonics(0), harmonicInterval(0.0), harmonicIntervalOffset(0.0), customCurve(nullptr)
		{ }

		/// \brief Copy constructor
		WaveformParameters(const WaveformParameters& src) : valueRange(src.valueRange), invert(src.invert), pulseWidth(src.pulseWidth), harmonics(src.harmonics), harmonicInterval(src.harmonicInterval), harmonicIntervalOffset(src.harmonicIntervalOffset), customCurve(src.customCurve)
		{ }

		/// \brief Construct from values
		WaveformParameters(VALUERANGE t_valueRange, Bool t_invert, Float t_pulseWidth, UInt t_harmonics, Float t_harmonicInterval, Float t_harmonicIntervalOffset, SplineData* t_customCurve) : valueRange(t_valueRange), invert(t_invert), pulseWidth(t_pulseWidth), harmonics(t_harmonics), harmonicInterval(t_harmonicInterval), harmonicIntervalOffset(t_harmonicIntervalOffset), customCurve(t_customCurve)
		{ }

		/// \brief Equals operator
		Bool operator ==(const WaveformParameters& c) const
		{
			return valueRange == c.valueRange && invert == c.invert && pulseWidth == c.pulseWidth && harmonics == c.harmonics && harmonicInterval == c.harmonicInterval && harmonicIntervalOffset == c.harmonicIntervalOffset && (customCurve != nullptr && c.customCurve != nullptr) && (EqualSplineDatas(customCurve, c.customCurve));
		}

		/// \brief Not-equals operator
		Bool operator !=(const WaveformParameters& c) const
		{
			return valueRange != c.valueRange || invert != c.invert || pulseWidth != c.pulseWidth || harmonics != c.harmonics || harmonicInterval != c.harmonicInterval || harmonicIntervalOffset != c.harmonicIntervalOffset || (customCurve != c.customCurve) || (!EqualSplineDatas(customCurve, c.customCurve));
		}
	};

public:
	///
	/// \brief Samples a sine wave.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	MAXON_ATTRIBUTE_FORCE_INLINE Float GetSin(Float x, const WaveformParameters& parameters) const
	{
		Float result = Sin(FreqToAngularVelocity(x));

		if (parameters.invert)
			result *= -1.0;

		if (parameters.valueRange == VALUERANGE::RANGE01)
			result = result * 0.5 + 0.5;

		return result;
	}

	///
	/// \brief Samples a cosine wave (90 deg rotated against sine wave).
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	MAXON_ATTRIBUTE_FORCE_INLINE Float GetCos(Float x, const WaveformParameters& parameters) const
	{
		Float result = Cos(FreqToAngularVelocity(x));

		if (parameters.invert)
			result *= -1.0;

		if (parameters.valueRange == VALUERANGE::RANGE01)
			result = result * 0.5 + 0.5;

		return result;
	}

	///
	/// \brief Samples a sawtooth wave.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	MAXON_ATTRIBUTE_FORCE_INLINE Float GetSawtooth(Float x, const WaveformParameters& parameters) const
	{
		Float result = FMod(x, 1.0);

		if (parameters.invert)
			result = 1.0 - result;

		if (parameters.valueRange == VALUERANGE::RANGE11)
			result = result * 2.0 - 1.0;

		return result;
	}

	///
	/// \brief Sampels a triangle wave.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters

	///
	/// \return The waveform value as position x
	///
	MAXON_ATTRIBUTE_FORCE_INLINE Float GetTriangle(Float x, const WaveformParameters& parameters) const
	{
		Float result = ASin(Sin(FreqToAngularVelocity(x))) * TWOBYPI;

		if (parameters.valueRange == VALUERANGE::RANGE01)
		{
			result = result * 0.5 + 0.5;
			if (parameters.invert)
				result = 1.0 - result;
		}
		else if (parameters.invert)
		{
			result *= -1.0;
		}

		return result;
	}

	///
	/// \brief Samples a simple square wave
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	MAXON_ATTRIBUTE_FORCE_INLINE Float GetSquare(Float x, const WaveformParameters& parameters) const
	{
		Float result = Sign(Sin(FreqToAngularVelocity(x)));

		if (parameters.valueRange == VALUERANGE::RANGE01)
		{
			result = result * 0.5 + 0.5;
			if (parameters.invert)
				result = 1.0 - result;
		}
		else if (parameters.invert)
		{
			result *= -1.0;
		}

		return result;
	}

	///
	/// \brief Samples a simple square wave with variable pulse width.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	MAXON_ATTRIBUTE_FORCE_INLINE Float GetPulse(Float x, const WaveformParameters& parameters) const
	{
		// Generate Sin() [period 1, range 0..1] and quantize it
		Float result = ((Sin(FreqToAngularVelocity(x)) * 0.5 + 0.5) < parameters.pulseWidth) ? 0.0 : 1.0;

		if (parameters.invert)
			result = 1.0 - result;

		if (parameters.valueRange == VALUERANGE::RANGE11)
			result = result * 2.0 - 1.0;

		return result;
	}

	///
	/// \brief Samples a simple square wave with variable pulse width.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	MAXON_ATTRIBUTE_FORCE_INLINE Float GetPulseRandom(Float x, const WaveformParameters& parameters) const
	{
		Float result = (Turbulence(Vector(x), 5.0, true) < parameters.pulseWidth) ? 0.0 : 1.0;

		if (parameters.invert)
			result = 1.0 - result;

		if (parameters.valueRange == VALUERANGE::RANGE11)
			result = result * 2.0 - 1.0;

		return result;
	}

	///
	/// \brief Samples a sawtooth wave by overlaying sine waves in harmonic frequencies.
	///
	/// \note This waveform is significantly slower to calculate than the non-analogue ones. In return, it looks sounds much better.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	MAXON_ATTRIBUTE_FORCE_INLINE Float GetAnalogSaw(Float x, const WaveformParameters& parameters) const
	{
		const Float fHarmonics = (Float)(parameters.harmonics + 1);
		const Float xValue = FreqToAngularVelocity(x);

		Float result = 0.0;
		for (Float n = 1.0; n < fHarmonics; n = ++n)
		{
			result += Sin(n * xValue) / n;
		}

		result *= TWOBYPI;

		if (!parameters.invert)
			result *= -1.0;

		if (parameters.valueRange == VALUERANGE::RANGE01)
			result = result * 0.5 + 0.5;

		return result;
	}

	///
	/// \brief Samples a sharktooth wave by overlaying alternating sine and cosine waves.
	///
	/// \note This waveform is significantly slower to calculate than the non-analogue ones. In return, it looks sounds much better.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	MAXON_ATTRIBUTE_FORCE_INLINE Float GetAnalogSharktooth(Float x, const WaveformParameters& parameters) const
	{
		const Float fHarmonics = (Float)(parameters.harmonics + 1);
		const Float xValue = FreqToAngularVelocity(x);

		Float result = 0.0;
		for (Float n = 1.0; n < fHarmonics; ++n)
		{
			if (FMod(n, 2.0) == 0.0)
				result += Sin(n * xValue) / n;
			else
				result -= Cos(n * xValue) / n;
		}

		result *= TWOBYPI;

		if (!parameters.invert)
			result *= -1.0;

		if (parameters.valueRange == VALUERANGE::RANGE01)
			result = result * 0.5 + 0.5;

		return result;
	}

	///
	/// \brief Samples a square wave by overlaying a sine and its odd harmonics.
	///
	/// \note This waveform is significantly slower to calculate than the non-analogue ones. In return, it looks sounds much better.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	MAXON_ATTRIBUTE_FORCE_INLINE Float GetAnalogSquare(Float x, const WaveformParameters& parameters) const
	{
		const Float fHarmonics = (Float)(parameters.harmonics + 1);
		const Float xValue = FreqToAngularVelocity(x);

		Float result = 0.0;
		for (Float n = 1.0; n < fHarmonics; n += 2.0)
		{
			result += Sin(n * xValue) / n;
		}

		result *= TWOBYPI;

		if (!parameters.invert)
			result *= -1.0;

		if (parameters.valueRange == VALUERANGE::RANGE01)
			result = result * 0.5 + 0.5;

		return result;
	}

	///
	/// \brief Samples a waveform by overlaying a sine and its harmonics. Depending on parameters, many interesting waveforms are possible.
	///
	/// \note This waveform is significantly slower to calculate than the non-analogue ones.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	MAXON_ATTRIBUTE_FORCE_INLINE Float GetAnalog(Float x, const WaveformParameters& parameters) const
	{
		const Float fHarmonics = (Float)(parameters.harmonics);
		const Float xValue = FreqToAngularVelocity(x);

		Float result = 0.0;
		for (Float n = parameters.harmonicIntervalOffset; n < fHarmonics * parameters.harmonicInterval; n += parameters.harmonicInterval)
		{
			result += Sin(n * xValue) / n;
		}

		result *= TWOBYPI;

		if (!parameters.invert)
			result *= -1.0;

		if (parameters.valueRange == VALUERANGE::RANGE01)
			result = result * 0.5 + 0.5;

		return result;
	}

	MAXON_ATTRIBUTE_FORCE_INLINE Float GetCustomSpline(Float x, const WaveformParameters& parameters) const
	{
		if (!parameters.customCurve)
			return 0.0;

		Float result = parameters.customCurve->GetPoint(GetSawtooth(x, WaveformParameters())).y;

		if (parameters.invert)
			result = 1.0 - result;

		if (parameters.valueRange == Oscillator::VALUERANGE::RANGE11)
			result = result * 2.0 - 1.0;

		return result;
	}

	///
	/// \brief Returns any of the waveforms, depending on oscType
	///
	MAXON_ATTRIBUTE_FORCE_INLINE Float SampleWaveform(Float x, WAVEFORMTYPE oscType, const WaveformParameters& parameters) const
	{
		switch (oscType)
		{
			case WAVEFORMTYPE::SINE:
				return GetSin(x, parameters);
			case WAVEFORMTYPE::COSINE:
				return GetCos(x, parameters);
			case WAVEFORMTYPE::SAWTOOTH:
				return GetSawtooth(x, parameters);
			case WAVEFORMTYPE::SQUARE:
				return GetSquare(x, parameters);
			case WAVEFORMTYPE::TRIANGLE:
				return GetTriangle(x, parameters);
			case WAVEFORMTYPE::PULSE:
				return GetPulse(x, parameters);
			case WAVEFORMTYPE::PULSERND:
				return GetPulseRandom(x, parameters);
			case WAVEFORMTYPE::SAW_ANALOG:
				return GetAnalogSaw(x, parameters);
			case WAVEFORMTYPE::SHARKTOOTH_ANALOG:
				return GetAnalogSharktooth(x, parameters);
			case WAVEFORMTYPE::SQUARE_ANALOG:
				return GetAnalogSquare(x, parameters);
			case WAVEFORMTYPE::ANALOG:
				return GetAnalog(x, parameters);
			case WAVEFORMTYPE::CUSTOMSPLINE:
				return GetCustomSpline(x, parameters);
		}
		return 0.0;
	}

	///
	/// \brief Renders the waveform to a BaseBitmap. Caller owns the pointed object.
	///
	/// \param[in] w Width of the rendered bitmap
	/// \param[in] h Height of the rendered bitmap
	/// \param[in] oscType Type of oscillator / waveform
	/// \param[in] parameters Waveform generation parameters
	/// \param[in] oversample Oversampling values. Must be >= 1, should be a power of 2 (1, 2, 4, 8, 16, 32, ...). A value of 1 will not apply any oversampling.
	///
	/// \return The rendered bitmap, or nullptr if anything went wrong.
	///
	BaseBitmap* RenderToBitmap(Int32 w, Int32 h, Oscillator::WAVEFORMTYPE oscType, const WaveformParameters& parameters, UInt32 oversample = 1)
	{

		const Int32 wActual = w * oversample;
		const Int32 hActual = h * oversample;
		AutoAlloc<BaseBitmap> bmp;
		if (!bmp)
			return nullptr;

		if (bmp->Init(wActual, hActual) != IMAGERESULT::OK)
			return nullptr;

		// Some precalculated values
		const Int32 wActual1 = wActual - 1;
		const Int32 hActual1 = hActual - 1;
		const Int32 hby2 = hActual / 2;
		const Float iw1 = Inverse((Float)(wActual1));

		// Draw background
		// ---------------
		bmp->Clear(g_previewAreaColor_bg_r, g_previewAreaColor_bg_g, g_previewAreaColor_bg_b);

		// Draw grid
		// ---------
		// Vertical lines
		bmp->SetPen(g_previewAreaColor_grid1_r, g_previewAreaColor_grid1_g, g_previewAreaColor_grid1_b);
		for (Int32 x = 0; x < wActual1; x = x + wActual1 / g_previewAreaVerticalGridLines)
			bmp->Line(x, 0, x, hActual1);

		// X axis
		bmp->SetPen(g_previewAreaColor_grid2_r, g_previewAreaColor_grid2_g, g_previewAreaColor_grid2_b);
		if (parameters.valueRange == Oscillator::VALUERANGE::RANGE11)
		{
			bmp->Line(0, hby2, wActual1, hby2);
		}
		else
		{
			bmp->Line(0, hActual1, wActual1, hActual1);
		}
		// Y axis
		bmp->Line(0, 0, 0, hActual1);

		// Axis labels
		bmp->SetPen(g_previewAreaColor_text_r, g_previewAreaColor_text_g, g_previewAreaColor_text_b);
		if (parameters.valueRange == Oscillator::VALUERANGE::RANGE11)
		{
			DrawX(bmp, wActual1 - g_previewAreaTextWidth - g_previewAreaTextMarginH, hby2 + g_previewAreaTextMarginV, g_previewAreaTextWidth, g_previewAreaTextHeight);
		}
		else
		{
			DrawX(bmp, wActual1 - g_previewAreaTextWidth - g_previewAreaTextMarginH, hActual1 - g_previewAreaTextHeight - g_previewAreaTextMarginV, g_previewAreaTextWidth, g_previewAreaTextHeight);
		}
		DrawY(bmp, 5, 5, g_previewAreaTextWidth, g_previewAreaTextHeight);

		// Draw waveform
		// -------------
		bmp->SetPen(g_previewAreaColor_wave_r, g_previewAreaColor_wave_g, g_previewAreaColor_wave_b);
		Int32 yPrevious = NOTOK;
		for (Int32 x = 0; x < wActual; ++x)
		{
			// Sample waveform
			const Float xSample = (Float)x * iw1 * g_previewAreaScaleX;
			Float y = (Int32)(SampleWaveform(xSample, oscType, parameters) * (Float)(hActual1));

			// Scale Y depending on waveform and value range.
			// The "analog" waveforms cause a bit of work here, as they
			// are inherently refusing to fit into a strict value range.
			// Because of that, we have to do some scaling and offsetting
			// for each type of "analog" waveform.
			switch (oscType)
			{
				case Oscillator::WAVEFORMTYPE::SAW_ANALOG:
				{
					if (parameters.valueRange == Oscillator::VALUERANGE::RANGE11)
						y = y * 0.4 + hActual * 0.5; // Vertically center
					else
						y = y * 0.8 + hActual * 0.1;

					break;
				}

				case Oscillator::WAVEFORMTYPE::SQUARE_ANALOG:
				{
					if (parameters.valueRange == Oscillator::VALUERANGE::RANGE11)
						y = y * 0.75 + hActual * 0.5; // Vertically center
					else
						y = y * 1.5 - hActual * 0.25;

					break;
				}

				case Oscillator::WAVEFORMTYPE::SHARKTOOTH_ANALOG:
				{
					if (parameters.valueRange == Oscillator::VALUERANGE::RANGE11)
						y = y * 0.25 + hActual * 0.5; // Vertically center
					else
						y = y * 0.5 + hActual * 0.25;
					break;
				}

				case Oscillator::WAVEFORMTYPE::ANALOG:
				{
					if (parameters.valueRange == Oscillator::VALUERANGE::RANGE11)
						y = y * 0.5 + hActual * 0.5;  // Vertically center
					break;
				}
			}

			// Avoid drawing outside bitmap bounds
			const Int32 yDraw = ClampValue(hActual1 - (Int32)y, 0, hActual1);

			// Optimization
			if (Abs(yDraw - yPrevious) > 1 && x > 0)
				// If this point is 2 or more pixels away from the previous one, draw a line
				bmp->Line(x - 1, yPrevious, x, yDraw);
			else
				// If this point lies directly beneath the previous one, just draw a pixel
				bmp->SetPixel(x, yDraw, g_previewAreaColor_wave_r, g_previewAreaColor_wave_g, g_previewAreaColor_wave_b);

			// Memorize previous point
			yPrevious = yDraw;
		}

		// Scale down the oversampled bitmap
		if (oversample > 1)
		{
			// Alloc and initialize temporary bitmap
			AutoAlloc<BaseBitmap> tmpBmp;
			if (!tmpBmp)
				return nullptr;
			if (tmpBmp->Init(w, h) != IMAGERESULT::OK)
				return nullptr;

			// Scale down bmp into temporary
			bmp->ScaleBicubic(tmpBmp, 0, 0, wActual1, hActual1, 0, 0, w - 1, h - 1);

			// Free original bitmap, replace with scaled tmpBmp
			bmp.Free();
			bmp.Assign(tmpBmp.Release());
		}

		return bmp.Release();
	}

};

#endif // OSCILLATOR_H__
