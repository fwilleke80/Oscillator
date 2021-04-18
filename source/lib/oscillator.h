#ifndef OSCILLATOR_H__
#define OSCILLATOR_H__


#include "c4d_general.h"
#include "ge_prepass.h"


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
		SHARKTOOTH_ANALOG = 8
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

		/// \brief Default vonstructor
		WaveformParameters() : valueRange(VALUERANGE::RANGE01), invert(false), random(false), pulseWidth(0.0), harmonics(0)
		{ }

		/// \brief Copy constructor
		WaveformParameters(const WaveformParameters& src) : valueRange(src.valueRange), invert(src.invert), random(src.random), pulseWidth(src.pulseWidth), harmonics(src.harmonics)
		{ }

		/// \brief Construct from values
		WaveformParameters(VALUERANGE t_valueRange, Bool t_invert, Bool t_random, Float t_pulseWidth, UInt t_harmonics) : valueRange(t_valueRange), invert(t_invert), random(t_random), pulseWidth(t_pulseWidth), harmonics(t_harmonics)
		{ }

		/// \brief Equals operator
		Bool operator ==(const WaveformParameters& c) const
		{
			return valueRange == c.valueRange && invert == c.invert && random == c.random && pulseWidth == c.pulseWidth && harmonics == c.harmonics;
		}

		/// \brief Not-equals operator
		Bool operator !=(const WaveformParameters& c) const
		{
			return valueRange != c.valueRange || invert != c.invert || random != c.random || pulseWidth != c.pulseWidth || harmonics != c.harmonics;
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
	Float GetSin(Float x, const WaveformParameters& parameters) const;

	///
	/// \brief Samples a cosine wave (90 deg rotated against sine wave).
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	Float GetCos(Float x, const WaveformParameters& parameters) const;

	///
	/// \brief Samples a sawtooth wave.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	Float GetSawtooth(Float x, const WaveformParameters& parameters) const;

	///
	/// \brief Sampels a triangle wave.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters

	///
	/// \return The waveform value as position x
	///
	Float GetTriangle(Float x, const WaveformParameters& parameters) const;

	///
	/// \brief Samples a simple square wave
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	Float GetSquare(Float x, const WaveformParameters& parameters) const;

	///
	/// \brief Samples a simple square wave with variable pulse width.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	Float GetPulse(Float x, const WaveformParameters& parameters) const;

	///
	/// \brief Samples a simple square wave with variable pulse width.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	Float GetPulseRandom(Float x, const WaveformParameters& parameters) const;

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
	Float GetAnalogSaw(Float x, const WaveformParameters& parameters) const;

	///
	/// \brief Samples a sharktooth wave by overlaying sine and cosine waves.
	///
	/// \note This waveform is significantly slower to calculate than the non-analogue ones. In return, it looks sounds much better.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] parameters The waveform parameters
	///
	/// \return The waveform value as position x
	///
	Float GetAnalogSharktooth(Float x, const WaveformParameters& parameters) const;

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
		}
		return 0.0;
	}

	///
	/// \brief Renders the waveform to a BaseBitmap. Caller owns the pointed object.
	///
	BaseBitmap* RenderToBitmap(Int32 w, Int32 h, Oscillator::WAVEFORMTYPE oscType, const WaveformParameters& parameters);
};

#endif // OSCILLATOR_H__


