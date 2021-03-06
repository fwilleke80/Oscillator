#include "customgui_bitmapbutton.h"
#include "customgui_splinecontrol.h"
#include "customgui_priority.h"
#include "c4d_tagplugin.h"
#include "c4d_tagdata.h"
#include "c4d_basetag.h"
#include "c4d_basecontainer.h"
#include "c4d_basedocument.h"
#include "c4d_resource.h"
#include "c4d_general.h"
#include "ge_prepass.h"

#include "oscillator.h"
#include "functions.h"

#include "main.h"
#include "c4d_symbols.h"
#include "toscillator.h"


static const Int32 ID_OSCILLATORTAG = 1057129;


class OscillatorTag : public TagData
{
	INSTANCEOF(OscillatorTag, TagData);

public:
	virtual Bool Init(GeListNode* node) override;
	virtual Bool Message(GeListNode* node, Int32 type, void* data) override;
	virtual Bool GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags) override;
	virtual Bool GetDParameter(GeListNode* node, const DescID& id, GeData& t_data, DESCFLAGS_GET& flags) override;

	virtual EXECUTIONRESULT Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags) override;

private:
	Oscillator _osc; // Oscillator instance
	Int32 _dirty; // Dirty count (used to make the waveform preview bitmapbutton update)

public:
	static NodeData* Alloc()
	{
		return NewObj(OscillatorTag) iferr_ignore();
	}

	OscillatorTag() : _dirty(0)
	{ }
};


Bool OscillatorTag::Init(GeListNode* node)
{
	iferr_scope_handler
	{
		ApplicationOutput("@", err.GetMessage());
		return false;
	};

	BaseTag* tagPtr	= static_cast<BaseTag*>(node);
	BaseContainer& dataRef = tagPtr->GetDataInstanceRef();

	// Set default attribute values
	dataRef.SetInt32(OSC_FUNCTION, FUNC_SAWTOOTH);
	dataRef.SetInt32(OSC_RANGE, RANGE_01);
	dataRef.SetFloat(OSC_PULSEWIDTH, 0.3);
	dataRef.SetFloat(OSC_INPUTSCALE, 1.0);
	dataRef.SetUInt32(OSC_HARMONICS, 4);
	dataRef.SetFloat(OSC_HARMONICS_INTERVAL, 1.0);
	dataRef.SetFloat(OSC_HARMONICS_OFFSET, 1.0);

	dataRef.SetInt32(FILTER_MODE, FILTER_MODE_NONE);
	dataRef.SetFloat(FILTER_SLEW_RATE_UP, 0.0);
	dataRef.SetFloat(FILTER_SLEW_RATE_DOWN, 0.0);
	dataRef.SetFloat(FILTER_INERTIA_DAMPEN, 0.5);
	dataRef.SetFloat(FILTER_INERTIA_INERTIA, 0.5);

	dataRef.SetBool(OSCTAG_OUTPUT_POS_ENABLE, true);
	dataRef.SetVector(OSCTAG_OUTPUT_POS, Vector(0.0, 100.0, 0.0));
	dataRef.SetBool(OSCTAG_OUTPUT_SCALE_ENABLE, true);
	dataRef.SetVector(OSCTAG_OUTPUT_SCALE, Vector(1.0, 1.0, 1.0));
	dataRef.SetBool(OSCTAG_OUTPUT_ROT_ENABLE, true);
	dataRef.SetVector(OSCTAG_OUTPUT_ROT, Vector(DegToRad(360.0)));

	// Set default spline
	GeData gdCurve(CUSTOMDATATYPE_SPLINE, DEFAULTVALUE);
	SplineData* splineCurve = static_cast<SplineData*>(gdCurve.GetCustomDataType(CUSTOMDATATYPE_SPLINE));
	if (!splineCurve)
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "splineCurve is nullptr!"_s));
	splineCurve->MakeLinearSplineBezier();
	splineCurve->InsertKnot(0.0, 0.0, 0);
	splineCurve->InsertKnot(1.0, 1.0, 0);
	dataRef.SetData(OSC_CUSTOMFUNC, gdCurve);

	return SUPER::Init(node);
}

Bool OscillatorTag::Message(GeListNode* node, Int32 type, void* data)
{
	iferr_scope_handler
	{
		ApplicationOutput("@", err.GetMessage());
		return false;
	};

	BaseTag* tagPtr = static_cast<BaseTag*>(node);

	switch (type)
	{
		case MSG_DESCRIPTION_GETBITMAP:
		{
			const BaseContainer& dataRef = tagPtr->GetDataInstanceRef();

			Oscillator::WAVEFORMTYPE oscType = (Oscillator::WAVEFORMTYPE)dataRef.GetInt32(OSC_FUNCTION);
			const Oscillator::VALUERANGE valueRange = (Oscillator::VALUERANGE)dataRef.GetInt32(OSC_RANGE);
			const Bool invert = dataRef.GetBool(OSC_INVERT);
			const Float pulseWidth = dataRef.GetFloat(OSC_PULSEWIDTH);
			const UInt harmonics = dataRef.GetUInt32(OSC_HARMONICS);
			const Float harmonicInterval = Max(dataRef.GetFloat(OSC_HARMONICS_INTERVAL), 0.1);
			const Float harmonicIntervalOffset = dataRef.GetFloat(OSC_HARMONICS_OFFSET);
			const Oscillator::FILTERTYPE filterType = (Oscillator::FILTERTYPE)dataRef.GetInt32(FILTER_MODE);
			const Float slewUp = dataRef.GetFloat(FILTER_SLEW_RATE_UP);
			const Float slewDown = dataRef.GetFloat(FILTER_SLEW_RATE_DOWN);
			const Float inertiaDampen = dataRef.GetFloat(FILTER_INERTIA_DAMPEN);
			const Float inertiaInertia = dataRef.GetFloat(FILTER_INERTIA_INERTIA);

			SplineData* customFuncCurve = (SplineData*)(dataRef.GetCustomDataType(OSC_CUSTOMFUNC, CUSTOMDATATYPE_SPLINE));
			if (!customFuncCurve)
				iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "customFuncCurve is nullptr!"_s));

			Oscillator::WaveformParameters parameters(valueRange, invert, pulseWidth, harmonics, harmonicInterval, harmonicIntervalOffset, filterType, slewUp, slewDown, inertiaDampen, inertiaInertia, customFuncCurve);

			DescriptionGetBitmap* dgb = (DescriptionGetBitmap*)data;
			dgb->_width = g_previewAreaWidth;
			dgb->_bmpflags = ICONDATAFLAGS::NONE;

			BaseBitmap* _previewBitmap = _osc.RenderToBitmap(g_previewAreaWidth, g_previewAreaHeight, oscType, parameters, g_previewAreaOversample);

			dgb->_bmp = _previewBitmap;

			return true;
		}
	}
	return SUPER::Message(node, type, data);
}

Bool OscillatorTag::GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags)
{
	if (!description->LoadDescription(ID_OSCILLATORTAG))
		return false;

	flags |= DESCFLAGS_DESC::LOADED;

	BaseTag* tagPtr = static_cast<BaseTag*>(node);
	const BaseContainer& dataRef = tagPtr->GetDataInstanceRef();

	const Int32 func = dataRef.GetInt32(OSC_FUNCTION);
	const Oscillator::FILTERTYPE filterType = (Oscillator::FILTERTYPE)dataRef.GetInt32(FILTER_MODE);

	HideDescriptionElement(node, description, OSC_CUSTOMFUNC, func != FUNC_CUSTOM);
	HideDescriptionElement(node, description, OSC_PULSEWIDTH, func != FUNC_PULSE && func != FUNC_PULSERND);
	HideDescriptionElement(node, description, OSC_HARMONICS, func != FUNC_SAW_ANALOG && func != FUNC_SHARKTOOTH_ANALOG && func != FUNC_SQUARE_ANALOG && func != FUNC_ANALOG);
	HideDescriptionElement(node, description, OSC_HARMONICS_INTERVAL, func != FUNC_ANALOG);
	HideDescriptionElement(node, description, OSC_HARMONICS_OFFSET, func != FUNC_ANALOG);
	HideDescriptionElement(node, description, FILTER_SLEW_RATE_UP, filterType != Oscillator::FILTERTYPE::SLEW);
	HideDescriptionElement(node, description, FILTER_SLEW_RATE_DOWN, filterType != Oscillator::FILTERTYPE::SLEW);
	HideDescriptionElement(node, description, FILTER_INERTIA_DAMPEN, filterType != Oscillator::FILTERTYPE::INERTIA);
	HideDescriptionElement(node, description, FILTER_INERTIA_INERTIA, filterType != Oscillator::FILTERTYPE::INERTIA);

	return SUPER::GetDDescription(node, description, flags);
}

Bool OscillatorTag::GetDParameter(GeListNode* node, const DescID& id, GeData& t_data, DESCFLAGS_GET& flags)
{
	switch (id[0].id)
	{
		case OSC_WAVEFORMPREVIEW:
		{
			++_dirty;
			BitmapButtonStruct bbs(static_cast<BaseList2D*>(node), id, _dirty);
			t_data = GeData(CUSTOMDATATYPE_BITMAPBUTTON, bbs);
			flags |= DESCFLAGS_GET::PARAM_GET;
			break;
		}
	}

	return SUPER::GetDParameter(node, id, t_data, flags);
}

EXECUTIONRESULT OscillatorTag::Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags)
{
	const BaseContainer& dataRef = tag->GetDataInstanceRef();

	const Oscillator::VALUERANGE outputRange = (Oscillator::VALUERANGE)dataRef.GetInt32(OSC_RANGE);
	const Bool outputInvert = dataRef.GetBool(OSC_INVERT);
	const Float pulseWidth = dataRef.GetFloat(OSC_PULSEWIDTH);
	const UInt32 harmonics = dataRef.GetUInt32(OSC_HARMONICS);
	const Float harmonicsInterval = dataRef.GetFloat(OSC_HARMONICS_INTERVAL);
	const Float harmonicsOffset = dataRef.GetFloat(OSC_HARMONICS_OFFSET);
	const Float inputFrequency = dataRef.GetFloat(OSC_INPUTSCALE);
	const Oscillator::FILTERTYPE filterType = (Oscillator::FILTERTYPE)dataRef.GetInt32(FILTER_MODE);
	const Float slewUp = dataRef.GetFloat(FILTER_SLEW_RATE_UP);
	const Float slewDown = dataRef.GetFloat(FILTER_SLEW_RATE_DOWN);
	const Float inertiaInertia = dataRef.GetFloat(FILTER_INERTIA_INERTIA);
	const Float inertiaDampen = dataRef.GetFloat(FILTER_INERTIA_DAMPEN);

	// Time
	const Float fps = doc->GetFps();
	const BaseTime currentTime = doc->GetTime();
	const Float inputTime = currentTime.Get();

	SplineData* customFuncCurve = (SplineData*)(dataRef.GetCustomDataType(OSC_CUSTOMFUNC, CUSTOMDATATYPE_SPLINE));
	if (!customFuncCurve)
	{
		ApplicationOutput("customFuncCurve is nullptr!");
		return EXECUTIONRESULT::OUTOFMEMORY;
	}

	// Osillator input data
	Oscillator::WaveformParameters waveformParameters(outputRange, outputInvert, pulseWidth, harmonics, harmonicsInterval, harmonicsOffset, filterType, slewUp, slewDown, inertiaDampen, inertiaInertia, customFuncCurve);
	const Oscillator::WAVEFORMTYPE waveformType = (Oscillator::WAVEFORMTYPE)dataRef.GetInt32(OSC_FUNCTION);
	const Float unfilteredWaveformValue(_osc.SampleWaveform(inputTime * inputFrequency, waveformType, waveformParameters));

	// Reset filter if necessary
	if (currentTime.GetFrame(fps) == doc->GetMinTime().GetFrame(doc->GetFps()))
		_osc.SetFilter(unfilteredWaveformValue);

	// Sample waveform
	const Float waveformValue = _osc.GetFiltered(unfilteredWaveformValue, waveformParameters, filterType);

	// Apply result to object
	const Bool enablePos = dataRef.GetBool(OSCTAG_OUTPUT_POS_ENABLE);
	const Vector vectorPos = dataRef.GetVector(OSCTAG_OUTPUT_POS);
	const Bool enableScale = dataRef.GetBool(OSCTAG_OUTPUT_SCALE_ENABLE);
	const Vector vectorScale = dataRef.GetVector(OSCTAG_OUTPUT_SCALE);
	const Bool enableRot = dataRef.GetBool(OSCTAG_OUTPUT_ROT_ENABLE);
	const Vector vectorRot = dataRef.GetVector(OSCTAG_OUTPUT_ROT);

	if (enablePos)
	{
		Vector opPos = op->GetRelPos();
		opPos = waveformValue * vectorPos;
		op->SetRelPos(opPos);
	}

	if (enableScale)
	{
		Vector opScale = op->GetRelScale();
		opScale = Vector(1.0) + waveformValue * vectorScale;
		op->SetRelScale(opScale);
	}

	if (enableRot)
	{
		Vector opRot = op->GetRelRot();
		opRot = waveformValue * vectorRot;
		op->SetRelRot(opRot);
	}

	return EXECUTIONRESULT::OK;
}


Bool RegisterOscillatorTag()
{
	return RegisterTagPlugin(ID_OSCILLATORTAG, GeLoadString(IDS_OSCILLATORTAG), TAG_EXPRESSION | TAG_VISIBLE, OscillatorTag::Alloc, "toscillator"_s, AutoBitmap("oscillator.tif"_s), 0);
}
