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
	enum class VALUERANGE
	{
		RANGE01 = 0,
		RANGE11 = 1
	} MAXON_ENUM_LIST_CLASS(VALUERANGE);

public:
	///
	/// \brief Samples a sine wave.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] valueRange Sets the result value range to either [0 .. 1] or [-1 .. 1].
	/// \param[in] invert Inverts the result value.
	///
	/// \return The waveform value as position x
	///
	Float GetSin(Float x, VALUERANGE valueRange = VALUERANGE::RANGE01, Bool invert = false) const;

	///
	/// \brief Samples a cosine wave (90 deg rotated against sine wave).
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] valueRange Sets the result value range to either [0 .. 1] or [-1 .. 1].
	/// \param[in] invert Inverts the result value.
	///
	/// \return The waveform value as position x
	///
	Float GetCos(Float x, VALUERANGE valueRange = VALUERANGE::RANGE01, Bool invert = false) const;

	///
	/// \brief Samples a sawtooth wave.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] valueRange Sets the result value range to either [0 .. 1] or [-1 .. 1].
	/// \param[in] invert Inverts the result value.
	///
	/// \return The waveform value as position x
	///
	Float GetSawtooth(Float x, VALUERANGE valueRange = VALUERANGE::RANGE01, Bool invert = false) const;

	///
	/// \brief Sampels a triangle wave.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] valueRange Sets the result value range to either [0 .. 1] or [-1 .. 1].
	/// \param[in] invert Inverts the result value.
	///
	/// \return The waveform value as position x
	///
	Float GetTriangle(Float x, VALUERANGE valueRange = VALUERANGE::RANGE01, Bool invert = false) const;

	///
	/// \brief Samples a simple square wave
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] valueRange Sets the result value range to either [0 .. 1] or [-1 .. 1].
	/// \param[in] invert Inverts the result value.
	///
	/// \return The waveform value as position x
	///
	Float GetSquare(Float x, VALUERANGE valueRange = VALUERANGE::RANGE01, Bool invert = false) const;

	///
	/// \brief Samples a simple square wave with variable pulse width.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] pulseWidth The pulse width [0 .. 1]
	/// \param[in] random Set to true to get a quantized noise instead of a sine base
	/// \param[in] valueRange Sets the result value range to either [0 .. 1] or [-1 .. 1].
	/// \param[in] invert Inverts the result value.
	///
	/// \return The waveform value as position x
	///
	Float GetPulse(Float x, Float pulseWidth, Bool random = false, VALUERANGE valueRange = VALUERANGE::RANGE01, Bool invert = false) const;

	///
	/// \brief Samples a sawtooth wave by overlaying sine waves in harmonic frequencies.
	///
	/// \note This waveform is significantly slower to calculate than the others. In return, it looks sounds much better.
	///
	/// \param[in] x The sample position (aka. time)
	/// \param[in] harmonics The number of harmonic frequencies added to the base frequency. Low values will calculate fast and produce softer results, high values will calculate slower and produce an increasingly sawtooth-like waveform.
	/// \param[in] valueRange Sets the result value range to either [0 .. 1] or [-1 .. 1].
	/// \param[in] invert Inverts the result value.
	///
	/// \return The waveform value as position x
	///
	Float GetAnalogSaw(Float x, UInt harmonics, VALUERANGE valueRange = VALUERANGE::RANGE01, Bool invert = false) const;
};

#endif // OSCILLATOR_H__


