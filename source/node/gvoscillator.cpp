#include "customgui_bitmapbutton.h"
#include "customgui_splinecontrol.h"
#include "c4d_operatordata.h"
#include "c4d_basebitmap.h"
#include "c4d_customdatatype.h"
#include "c4d_general.h"
#include "ge_prepass.h"

#include "oscillator.h"
#include "functions.h"

#include "main.h"
#include "gvoscillator.h"
#include "c4d_symbols.h"


/*
	Lots of this code was inspired from different threads on https://plugincafe.maxon.net.
	Even some of the comments in the code were taken from there.
*/


const Int32 ID_OSCILLATORNODE = 1057105; ///< Plugin ID for Oscillator node
const Int32 ID_OSCILLATOR_NODEGROUP = 1057106; ///< Plugin ID for Oscillator group


// Use for custom selection or assuring a certain order of values/ports in GvBuildValuesTable()
// The values info table will be sorted and indexed like this array (see the enums below, defining the indexes for later use in Calculate())
static Int32 g_input_ids[] = {
	INPORT_X,
	OSC_INPUTSCALE,
	OSC_PULSEWIDTH,
	OSC_HARMONICS,
	OSC_HARMONICS_INTERVAL,
	OSC_HARMONICS_OFFSET,
	FILTER_SLEW_RATE_UP,
	FILTER_SLEW_RATE_DOWN,
	FILTER_INERTIA_DAMPEN,
	FILTER_INERTIA_INERTIA,
	0
};


///
/// \brief Implements the Oscillator XPresso node
///
class OscillatorNode : public GvOperatorData
{
	INSTANCEOF(gvOscillator, GvOperatorData);

public:
	virtual Bool iCreateOperator(GvNode* bn) override;
	virtual Bool Message(GeListNode* node, Int32 type, void* data) override;
	virtual Bool GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags) override;
	virtual Bool GetDParameter(GeListNode* node, const DescID& id, GeData& t_data, DESCFLAGS_GET& flags) override;

	virtual const String GetText(GvNode* bn) override;

	virtual Bool InitCalculation(GvNode* bn, GvCalc* c, GvRun* r) override;
	virtual void FreeCalculation(GvNode* bn, GvCalc* c) override;
	virtual Bool Calculate(GvNode* bn, GvPort* port, GvRun* run, GvCalc* calc) override;

private:
	GvValuesInfo _ports; // Inports and outports
	Oscillator _osc; // Oscillator instance
	Int32 _dirty; // Dirty count (used to make the waveform preview bitmapbutton update)

public:
	static NodeData* Alloc()
	{
		return NewObj(OscillatorNode) iferr_ignore();
	}

	OscillatorNode() : _dirty(0)
	{ }
};


Bool OscillatorNode::iCreateOperator(GvNode* bn)
{
	iferr_scope_handler
	{
		ApplicationOutput("@", err.GetMessage());
		return false;
	};

	BaseContainer* dataPtr = bn->GetOpContainerInstance();
	if (!dataPtr)
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "GetOpContainerInstance() returned nullptr!"_s));

	// Set default attribute values
	dataPtr->SetInt32(OSC_FUNCTION, FUNC_SAWTOOTH);
	dataPtr->SetInt32(OSC_RANGE, RANGE_01);
	dataPtr->SetFloat(OSC_PULSEWIDTH, 0.3);
	dataPtr->SetFloat(OSC_INPUTSCALE, 1.0);
	dataPtr->SetUInt32(OSC_HARMONICS, 4);
	dataPtr->SetFloat(OSC_HARMONICS_INTERVAL, 1.0);
	dataPtr->SetFloat(OSC_HARMONICS_OFFSET, 1.0);

	dataPtr->SetInt32(FILTER_MODE, FILTER_MODE_NONE);
	dataPtr->SetFloat(FILTER_SLEW_RATE_UP, 0.0);
	dataPtr->SetFloat(FILTER_SLEW_RATE_DOWN, 0.0);
	dataPtr->SetFloat(FILTER_INERTIA_INERTIA, 0.5);
	dataPtr->SetFloat(FILTER_INERTIA_DAMPEN, 0.5);

	// Set default spline
	GeData gdCurve(CUSTOMDATATYPE_SPLINE, DEFAULTVALUE);
	SplineData* splineCurve = static_cast<SplineData*>(gdCurve.GetCustomDataType(CUSTOMDATATYPE_SPLINE));
	if (!splineCurve)
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "splineCurve is nullptr!"_s));
	splineCurve->MakeLinearSplineBezier();
	splineCurve->InsertKnot(0.0, 0.0, 0);
	splineCurve->InsertKnot(1.0, 1.0, 0);
	dataPtr->SetData(OSC_CUSTOMFUNC, gdCurve);

	return SUPER::iCreateOperator(bn);
}

Bool OscillatorNode::Message(GeListNode* node, Int32 type, void* data)
{
	iferr_scope_handler
	{
		ApplicationOutput("@", err.GetMessage());
		return false;
	};

	GvNode* nodePtr = static_cast<GvNode*>(node);

	switch (type)
	{
		case MSG_DESCRIPTION_GETBITMAP:
		{
			BaseContainer* dataPtr = nodePtr->GetOpContainerInstance();

			Oscillator::WAVEFORMTYPE oscType = (Oscillator::WAVEFORMTYPE)dataPtr->GetInt32(OSC_FUNCTION);
			const Oscillator::VALUERANGE valueRange = (Oscillator::VALUERANGE)dataPtr->GetInt32(OSC_RANGE);
			const Bool invert = dataPtr->GetBool(OSC_INVERT);
			const Float pulseWidth = dataPtr->GetFloat(OSC_PULSEWIDTH);
			const UInt harmonics = dataPtr->GetUInt32(OSC_HARMONICS);
			const Float harmonicInterval = Max(dataPtr->GetFloat(OSC_HARMONICS_INTERVAL), 0.1);
			const Float harmonicIntervalOffset = dataPtr->GetFloat(OSC_HARMONICS_OFFSET);
			const Oscillator::FILTERTYPE filterType = (Oscillator::FILTERTYPE)dataPtr->GetInt32(FILTER_MODE);
			const Float slewUp = dataPtr->GetFloat(FILTER_SLEW_RATE_UP);
			const Float slewDown = dataPtr->GetFloat(FILTER_SLEW_RATE_DOWN);
			const Float inertiaInertia = dataPtr->GetFloat(FILTER_INERTIA_INERTIA);
			const Float inertiaDampen = dataPtr->GetFloat(FILTER_INERTIA_DAMPEN);

			SplineData* customFuncCurve = (SplineData*)(dataPtr->GetCustomDataType(OSC_CUSTOMFUNC, CUSTOMDATATYPE_SPLINE));
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

Bool OscillatorNode::GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags)
{
	if (!description->LoadDescription(ID_OSCILLATORNODE))
		return false;

	flags |= DESCFLAGS_DESC::LOADED;

	GvNode *nodePtr  = static_cast<GvNode*>(node);
	BaseContainer *dataPtr = nodePtr->GetOpContainerInstance();

	const Int32 func = dataPtr->GetInt32(OSC_FUNCTION);
	const Oscillator::FILTERTYPE filterType = (Oscillator::FILTERTYPE)dataPtr->GetInt32(FILTER_MODE);

	HideDescriptionElement(node, description, OSC_CUSTOMFUNC, func != FUNC_CUSTOM);
	HideDescriptionElement(node, description, OSC_PULSEWIDTH, func != FUNC_PULSE && func != FUNC_PULSERND);
	HideDescriptionElement(node, description, OSC_HARMONICS, func != FUNC_SAW_ANALOG && func != FUNC_SHARKTOOTH_ANALOG && func != FUNC_SQUARE_ANALOG && func != FUNC_ANALOG);
	HideDescriptionElement(node, description, OSC_HARMONICS_INTERVAL, func != FUNC_ANALOG);
	HideDescriptionElement(node, description, OSC_HARMONICS_OFFSET, func != FUNC_ANALOG);
	HideDescriptionElement(node, description, OUTPORT_VALUE, true);
	HideDescriptionElement(node, description, INPORT_X, true);
	HideDescriptionElement(node, description, FILTER_SLEW_RATE_UP, filterType != Oscillator::FILTERTYPE::SLEW);
	HideDescriptionElement(node, description, FILTER_SLEW_RATE_DOWN, filterType != Oscillator::FILTERTYPE::SLEW);
	HideDescriptionElement(node, description, FILTER_INERTIA_DAMPEN, filterType != Oscillator::FILTERTYPE::INERTIA);
	HideDescriptionElement(node, description, FILTER_INERTIA_INERTIA, filterType != Oscillator::FILTERTYPE::INERTIA);

	return SUPER::GetDDescription(node, description, flags);
}

Bool OscillatorNode::GetDParameter(GeListNode* node, const DescID& id, GeData& t_data, DESCFLAGS_GET& flags)
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

// Get node text (when "port names" if OFF)
const String OscillatorNode::GetText(GvNode* bn)
{
	switch (bn->GetOpContainerInstance()->GetInt32(OSC_FUNCTION))
	{
		case FUNC_SINE:
			return GeLoadString(IDS_FUNC_SINE);
		case FUNC_COSINE:
			return GeLoadString(IDS_FUNC_COSINE);
		case FUNC_TRIANGLE:
			return GeLoadString(IDS_FUNC_TRIANGLE);
		case FUNC_SQUARE:
			return GeLoadString(IDS_FUNC_SQUARE);
		case FUNC_SAWTOOTH:
			return GeLoadString(IDS_FUNC_SAWTOOTH);
		case FUNC_PULSE:
			return GeLoadString(IDS_FUNC_PULSE);
		case FUNC_PULSERND:
			return GeLoadString(IDS_FUNC_PULSERND);
		case FUNC_SAW_ANALOG:
			return GeLoadString(IDS_FUNC_SAW_ANALOG);
		case FUNC_SHARKTOOTH_ANALOG:
			return GeLoadString(IDS_FUNC_SHARKTOOTH_ANALOG);
		case FUNC_SQUARE_ANALOG:
			return GeLoadString(IDS_FUNC_SQUARE_ANALOG);
		case FUNC_ANALOG:
			return GeLoadString(IDS_FUNC_ANALOG);
		case FUNC_CUSTOM:
			return GeLoadString(IDS_FUNC_CUSTOM);
	}

	return GeLoadString(IDS_OSCILLATORNODE);
}

Bool OscillatorNode::InitCalculation(GvNode* bn, GvCalc* calc, GvRun* run)
{
	return GvBuildInValuesTable(bn, _ports, calc, run, g_input_ids); // or GV_EXISTING_PORTS or GV_DEFINED_PORTS instead of input_ids
}

void OscillatorNode::FreeCalculation(GvNode *bn, GvCalc *c)
{
	// Don't forget to free the values table(s) and dynamic data at the end of the calculation!
	GvFreeValuesTable(bn, _ports);
}

Bool OscillatorNode::Calculate(GvNode *bn, GvPort *port, GvRun *run, GvCalc *calc)
{
	iferr_scope_handler
	{
		ApplicationOutput("@", err.GetMessage());
		return false;
	};

	// Check for nullptr
	if (!bn || !run || !calc)
		return false;

	// Calculate input ports
	// ---------------------

	// First get all input values calculated
	// Note: In-values may also be calculated separately,
	//       for example if one needs only a few of them calculated depending on a mode parameter.
	//       In this case use _ports.in_values[idx]->Calculate()
	if (!GvCalculateInValuesTable(bn, run, calc, _ports))
		return false;


	// Get node's BaseContainer
	BaseContainer* dataPtr = bn->GetOpContainerInstance();
	if (!dataPtr)
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "GetOpContainerInstance() returned nullptr!"_s));

	const Oscillator::VALUERANGE outputRange = (Oscillator::VALUERANGE)dataPtr->GetInt32(OSC_RANGE);
	const Oscillator::FILTERTYPE filterType = (Oscillator::FILTERTYPE)dataPtr->GetInt32(FILTER_MODE);
	const Bool outputInvert = dataPtr->GetBool(OSC_INVERT);

	// With multiple output ports, Calculate() may be called multiple times per calculation
	// port == nullptr means all ports requested
	if (port && port->GetMainID() == OUTPORT_VALUE)
	{
		// Now get all ports needed for calculation
		// Note: Another option would be to do a switch (_ports.in_values[idxPort]->GetMainID()),
		//       be warned though, this won't work with ports defined with MULTIPLE
		GvPort* const portInputValue = _ports.in_values[0]->GetPort();
		Float inputValue = 0.0;
		if (portInputValue)
		{
			if (!portInputValue->GetFloat(&inputValue, run))
				return false;
		}

		GvPort* const portFrequency = _ports.in_values[1]->GetPort();
		Float frequency = 0.0;
		if (portFrequency)
		{
			if (!portFrequency->GetFloat(&frequency, run))
				return false;
		}

		GvPort* const portPulseWidth = _ports.in_values[2]->GetPort();
		Float pulseWidth = 0.0;
		if (portPulseWidth)
		{
			if (!portPulseWidth->GetFloat(&pulseWidth, run))
				return false;
		}

		GvPort* const portHarmonics = _ports.in_values[3]->GetPort();
		Int32 harmonics = 0;
		if (portHarmonics)
		{
			if (!portHarmonics->GetInteger(&harmonics, run))
				return false;
		}

		GvPort* const portHarmonicsInterval = _ports.in_values[4]->GetPort();
		Float harmonicsInterval = 0.0;
		if (portHarmonicsInterval)
		{
			if (!portHarmonicsInterval->GetFloat(&harmonicsInterval, run))
				return false;
			harmonicsInterval = Max(harmonicsInterval, 0.1);
		}

		GvPort* const portHarmonicsOffset = _ports.in_values[5]->GetPort();
		Float harmonicsOffset = 0.0;
		if (portHarmonicsOffset)
		{
			if (!portHarmonicsOffset->GetFloat(&harmonicsOffset, run))
				return false;
		}

		GvPort* const portSlewUp = _ports.in_values[6]->GetPort();
		Float slewUp = 0.0;
		if (portSlewUp)
		{
			if (!portSlewUp->GetFloat(&slewUp, run))
				return false;
		}

		GvPort* const portSlewDown = _ports.in_values[7]->GetPort();
		Float slewDown = 0.0;
		if (portSlewDown)
		{
			if (!portSlewDown->GetFloat(&slewDown, run))
				return false;
		}

		GvPort* const portDampen = _ports.in_values[8]->GetPort();
		Float inertiaDampen = 0.0;
		if (portDampen)
		{
			if (!portDampen->GetFloat(&inertiaDampen, run))
				return false;
		}

		GvPort* const portInertia = _ports.in_values[9]->GetPort();
		Float inertiaInertia = 0.0;
		if (portInertia)
		{
			if (!portInertia->GetFloat(&inertiaInertia, run))
				return false;
		}

		SplineData* customFuncCurve = (SplineData*)(dataPtr->GetCustomDataType(OSC_CUSTOMFUNC, CUSTOMDATATYPE_SPLINE));
		if (!customFuncCurve)
			iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "customFuncCurve is nullptr!"_s));

		// Osillator input data
		Oscillator::WaveformParameters waveformParameters(outputRange, outputInvert, pulseWidth, harmonics, harmonicsInterval, harmonicsOffset, filterType, slewUp, slewDown, inertiaDampen, inertiaInertia, customFuncCurve);
		const Oscillator::WAVEFORMTYPE waveformType = (Oscillator::WAVEFORMTYPE)dataPtr->GetInt32(OSC_FUNCTION);

		// Sample waveform
		Float waveformValue = _osc.GetFiltered(_osc.SampleWaveform(inputValue * frequency, waveformType, waveformParameters), waveformParameters, filterType);

		// Set waveform value to output port
		port->SetFloat(waveformValue, run);

		return true;
	}

	return false;
}


// Create empty dummy icon, since it's never used by C4D anyway
static BaseBitmap* GetMyGroupIcon()
{
	static AutoAlloc<BaseBitmap> icon;
	if (!icon)
		return nullptr;
	if (icon->GetBw() == 0)
	{
		icon->Init(24, 24);
		icon->Clear(200, 0, 0);
	}
	return icon;
}

// Return pointer to node's Group name
static const String* GetMyGroupName()
{
	static const String mygroup(GeLoadString(IDS_OSCILLATOR_NODEGROUP));
	return &mygroup;
}

// Register node function
Bool RegisterGvOscillator()
{
	// Don't continue without name
	const maxon::String name = GeLoadString(IDS_OSCILLATORNODE);
	if (name.IsEmpty())
		return true;

	// Create Oscillator node group (submenu in XPresso editor)
	static GV_OPGROUP_HANDLER mygroup;
	mygroup.group_id = ID_OSCILLATOR_NODEGROUP;
	mygroup.GetName = GetMyGroupName;
	mygroup.GetIcon = GetMyGroupIcon;
	if (!GvRegisterOpGroupType(&mygroup, sizeof(mygroup)))
		return false;

	return GvRegisterOperatorPlugin(ID_OSCILLATORNODE, name, 0, OscillatorNode::Alloc, "gvoscillator"_s, 0, ID_GV_OPCLASS_TYPE_GENERAL, ID_OSCILLATOR_NODEGROUP, ID_GV_IGNORE_OWNER, AutoBitmap("oscillator.tif"_s));
}
