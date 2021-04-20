#ifndef FILTER_H__
#define FILTER_H__

#include "c4d_general.h"
#include "ge_prepass.h"


namespace Filter
{

	class Slew
	{
	public:
		///
		/// \brief Initializes the Slew object and resets all internal values.
		///
		void Init()
		{
			_previousValue = 0.0;
		}

		///
		/// \brief Sets the state value of the filter.
		///
		void Set(Float value)
		{
			_previousValue = value;
		}

		///
		/// \brief Filters a value.
		///
		MAXON_ATTRIBUTE_FORCE_INLINE Float Filter(Float value, Float slewRate)
		{
			const Float delta = (value - _previousValue);
			_previousValue = _previousValue + delta * (1.0 - slewRate);
			return _previousValue;
		}

		///
		/// \brief Filters a value.
		///
		MAXON_ATTRIBUTE_FORCE_INLINE Float Filter(Float value, Float slewRateUp, Float slewRateDown)
		{
			const Float delta = (value - _previousValue);
			const Bool up = (delta >= 0.0);
			_previousValue = _previousValue + delta * (up ? (1.0 - slewRateUp) : (1.0 - slewRateDown));
			return _previousValue;
		}

	private:
		Float _previousValue;

	public:
		Slew() : _previousValue(0.0)
		{ }
	};

}

#endif // FILTER_H__
