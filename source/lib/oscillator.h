#ifndef OSCILLATOR_H__
#define OSCILLATOR_H__

#include "c4d_tools.h"
#include "c4d_basebitmap.h"
#include "c4d_general.h"

#include "ge_prepass.h"

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
		ANALOG = 10
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
		Bool random; ///< If this is true, GetPulse() will return a noise-based pulse
		Float pulseWidth; ///< Defines the pulse width of GetPulse(). [0 .. 1].
		UInt harmonics; ///< Defines the nmber of harmonics in GetAnalogX(). [1 .. infinite]
		Float harmonicInterval; ///< Harmonic multiplication will be increased by this value for each harmonic
		Float harmonicIntervalOffset; ///< Harmonic multiplication will start with this value before it is increased

		/// \brief Default vonstructor
		WaveformParameters() : valueRange(VALUERANGE::RANGE01), invert(false), random(false), pulseWidth(0.0), harmonics(0), harmonicInterval(0.0), harmonicIntervalOffset(0.0)
		{ }

		/// \brief Copy constructor
		WaveformParameters(const WaveformParameters& src) : valueRange(src.valueRange), invert(src.invert), random(src.random), pulseWidth(src.pulseWidth), harmonics(src.harmonics), harmonicInterval(src.harmonicInterval), harmonicIntervalOffset(src.harmonicIntervalOffset)
		{ }

		/// \brief Construct from values
		WaveformParameters(VALUERANGE t_valueRange, Bool t_invert, Bool t_random, Float t_pulseWidth, UInt t_harmonics, Float t_harmonicInterval, Float t_harmonicIntervalOffset) : valueRange(t_valueRange), invert(t_invert), random(t_random), pulseWidth(t_pulseWidth), harmonics(t_harmonics), harmonicInterval(t_harmonicInterval), harmonicIntervalOffset(t_harmonicIntervalOffset)
		{ }

		/// \brief Equals operator
		Bool operator ==(const WaveformParameters& c) const
		{
			return valueRange == c.valueRange && invert == c.invert && random == c.random && pulseWidth == c.pulseWidth && harmonics == c.harmonics && harmonicInterval == c.harmonicInterval && harmonicIntervalOffset == c.harmonicIntervalOffset;
		}

		/// \brief Not-equals operator
		Bool operator !=(const WaveformParameters& c) const
		{
			return valueRange != c.valueRange || invert != c.invert || random != c.random || pulseWidth != c.pulseWidth || harmonics != c.harmonics || harmonicInterval != c.harmonicInterval || harmonicIntervalOffset != c.harmonicIntervalOffset;
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

		if (parameters.invert)
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
		Float result = 0.0;
		const Float fHarmonics = (Float)(parameters.harmonics + 1);
		const Float xValue = FreqToAngularVelocity(x);

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
		Float result = 0.0;
		const Float fHarmonics = (Float)(parameters.harmonics + 1);
		const Float xValue = FreqToAngularVelocity(x);

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
		Float result = 0.0;
		const Float fHarmonics = (Float)(parameters.harmonics + 1);
		const Float xValue = FreqToAngularVelocity(x);

		for (Float n = 1.0; n < fHarmonics; n = n + 2.0)
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


	MAXON_ATTRIBUTE_FORCE_INLINE Float GetAnalog(Float x, const WaveformParameters& parameters) const
	{
		const Float fHarmonics = (Float)(parameters.harmonics + 1);
		const Float xValue = FreqToAngularVelocity(x);

		Float result = Sin(xValue); // Fundamental

		// Harmonics
		for (Float n = 2.0; n < fHarmonics; n = n + 2.0)
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
		}
		return 0.0;
	}

	///
	/// \brief Renders the waveform to a BaseBitmap. Caller owns the pointed object.
	///
	BaseBitmap* RenderToBitmap(Int32 w, Int32 h, Oscillator::WAVEFORMTYPE oscType, const WaveformParameters& parameters)
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
		bmp->Clear(32, 32, 32);

		// Grid
		bmp->SetPen(8, 48, 8);
		static const Int32 verticalGridLines = 4;
		if (parameters.valueRange == Oscillator::VALUERANGE::RANGE11)
			bmp->Line(0, h / 2, w - 1, h / 2);
		else
			bmp->Line(0, h - 1, w - 1, h - 1);
		for (Int32 x = 0; x < w - 1; x = x + w / verticalGridLines)
		{
			bmp->Line(x, 0, x, h - 1);
		}

		// Waveform
		bmp->SetPen(32, 255, 16);
		static const Float previewScaleX = 2.0;
		const Float iw1 = Inverse((Float)(w - 1));
		Int32 yPrevious = NOTOK;
		for (Int32 x = 0; x < w; ++x)
		{
			const Float xSample = (Float)x * iw1 * previewScaleX;
			Float y = (Int32)(SampleWaveform(xSample, oscType, parameters) * (Float)(h - 1));
			if (oscType == Oscillator::WAVEFORMTYPE::SAW_ANALOG || oscType == Oscillator::WAVEFORMTYPE::SHARKTOOTH_ANALOG || oscType == Oscillator::WAVEFORMTYPE::SQUARE_ANALOG)
			{
				y *= 0.8; // Scale down, so we don't draw outside of area
				if (parameters.valueRange == Oscillator::VALUERANGE::RANGE11)
					y = y * 0.4 + h * 0.5; // Vertically center
			}
			else if (parameters.valueRange == Oscillator::VALUERANGE::RANGE11)
			{
				y *= 0.9; // Scale down a little, so we don't draw outside of area
				y = y * 0.45 + h * 0.5;  // Vertically center
			}

			const Int32 yDraw = h - 1 - (Int32)y;

			if (Abs(yDraw - yPrevious) > 1 && x > 0)
				bmp->Line(x - 1, yPrevious, x, yDraw);
			else
				bmp->SetPixel(x, yDraw, 0, 255, 0);

			yPrevious = yDraw;
		}

		return bmp;
	}

};

#endif // OSCILLATOR_H__


