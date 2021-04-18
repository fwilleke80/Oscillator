#include "customgui_bitmapbutton.h"
#include "customgui_splinecontrol.h"
#include "c4d_operatordata.h"
#include "c4d_basebitmap.h"
#include "c4d_customdatatype.h"
#include "c4d_general.h"
#include "ge_prepass.h"

#include "oscillator.h"

#include "main.h"
#include "gvoscillator.h"
#include "c4d_symbols.h"


/*
	Lots of this code was inspired from different threads on https://plugincafe.maxon.net.
	Even some of the comments in the code were taken from there.
*/


const Int32 ID_OSCILLATORNODE = 1057105; ///< Plugin ID for Oscillator node
const Int32 ID_OSCILLATOR_NODEGROUP = 1057106; ///< Plugin ID for Oscillator group


///
/// \brief Shows or hides a description element from the node.
///
/// \param[in] node Pointer to the GeListNode that owns the description
/// \param[in] description Pointer to the Desctiption instance
/// \param[in] descId ID of the element to hide
/// \param[in] hide Set to true to hide the element, set to false to show it.
///
static Bool HideDescriptionElement(GeListNode* node, Description* description, Int32 descId, Bool hide)
{
	AutoAlloc<AtomArray> ar;
	if (!ar)
		return false;
	ar->Append(static_cast<C4DAtom*>(node));

	BaseContainer *bc = description->GetParameterI(DescLevel(descId), ar);
	if (bc)
		bc->SetBool(DESC_HIDE, hide);
	else
		return false;

	return true;
}


// Use for custom selection or assuring a certain order of values/ports in GvBuildValuesTable()
// The values info table will be sorted and indexed like this array (see the enums below, defining the indexes for later use in Calculate())
static Int32 g_input_ids[] = {
	INPORT_X,
	OSC_INPUTSCALE,
	OSC_PULSEWIDTH,
	OSC_HARMONICS,
	0
};


///
/// \brief Implements the Oscillator XPresso node
///
class OscillatorNode : public GvOperatorData
{
	INSTANCEOF(gvOscillator, GvOperatorData);

public:
	virtual Bool Message(GeListNode* node, Int32 type, void* data) override;
	virtual Bool GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags) override;
	virtual Bool GetDParameter(GeListNode* node, const DescID& id, GeData& t_data, DESCFLAGS_GET& flags) override;

	virtual Bool iCreateOperator(GvNode* bn) override;
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
		case FUNC_CUSTOM:
			return GeLoadString(IDS_FUNC_CUSTOM);
	}

	return GeLoadString(IDS_OSCILLATORNODE);
}

Bool OscillatorNode::InitCalculation(GvNode* bn, GvCalc* calc, GvRun* run)
{
	return GvBuildInValuesTable(bn, _ports, calc, run, g_input_ids); // or GV_EXISTING_PORTS or GV_DEFINED_PORTS instead of input_ids
}

Bool OscillatorNode::Message(GeListNode* node, Int32 type, void* data)
{
	GvNode* nodePtr = static_cast<GvNode*>(node);

	switch (type)
	{
		case MSG_DESCRIPTION_GETBITMAP:
		{
			BaseContainer* dataPtr = nodePtr->GetOpContainerInstance();

			Oscillator::OSCTYPE oscType = (Oscillator::OSCTYPE)dataPtr->GetInt32(OSC_FUNCTION);
			const Oscillator::VALUERANGE valueRange = (Oscillator::VALUERANGE)dataPtr->GetInt32(OSC_RANGE);
			const Bool invert = dataPtr->GetBool(OSC_INVERT);
			const Float pulseWidth = dataPtr->GetFloat(OSC_PULSEWIDTH);
			const UInt harmonics = dataPtr->GetUInt32(OSC_HARMONICS);

			Oscillator::WaveformParameters parameters(valueRange, invert, false, pulseWidth, harmonics);

			DescriptionGetBitmap* dgb = (DescriptionGetBitmap*)data;
			dgb->_width = 400;
			dgb->_bmpflags = ICONDATAFLAGS::NONE;

			BaseBitmap* _previewBitmap = _osc.RenderToBitmap(400, 100, oscType, parameters);

			dgb->_bmp = _previewBitmap;

			return true;
		}

	}

//	switch (type)
//	{
//		case MSG_DESCRIPTION_POSTSETPARAMETER:
//		{
//			DescriptionCommand *dc = (DescriptionCommand*)data;
//			CurveDrawData cdd;
//			cdd.bInvert = dataPtr->GetBool(OSC_INVERT);
//			cdd.lCurveType = dataPtr->GetInt32(OSC_FUNCTION);
//			cdd.range = (Oscillator::VALUERANGE)dataPtr->GetInt32(OSC_RANGE);
//			cdd.rPulseWidth = dataPtr->GetFloat(OSC_PULSEWIDTH);

//			GeData gd (CUSTOMDATATYPE_CURVEPREVIEW, DEFAULTVALUE);
//			CurvePreviewData *cpd = (CurvePreviewData*) gd.GetCustomDataType(CUSTOMDATATYPE_CURVEPREVIEW);
//			cpd->SetCurveDrawData(cdd);
//			dataPtr->SetData(OSC_PREVIEW, gd);
//			break;
//		}
//	}

	return true;
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

		SplineData* customFuncCurve = (SplineData*)(dataPtr->GetCustomDataType(OSC_CUSTOMFUNC, CUSTOMDATATYPE_SPLINE));
		if (!customFuncCurve)
			iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "customFuncCurve is nullptr!"_s));

		//		// Do the sum calculation for the long output
		//		if (run->IsIterationPath())
		//			GePrint("Node is connected to an iterator"); // maybe want to act special here

		const Int32 oscFunction = dataPtr->GetInt32(OSC_FUNCTION);

		Oscillator::WaveformParameters waveformParameters(outputRange, outputInvert, oscFunction == FUNC_PULSERND, pulseWidth, harmonics);

		Float waveformValue = 0.0;
		switch (oscFunction)
		{
			case FUNC_SINE:
			{
				waveformValue = _osc.GetSin(inputValue * frequency, waveformParameters);
				break;
			}

			case FUNC_COSINE:
			{
				waveformValue = _osc.GetCos(inputValue * frequency, waveformParameters);
				break;
			}

			case FUNC_SAWTOOTH:
			{
				waveformValue = _osc.GetSawtooth(inputValue * frequency, waveformParameters);
				break;
			}

			case FUNC_SQUARE:
			{
				waveformValue = _osc.GetSquare(inputValue * frequency, waveformParameters);
				break;
			}

			case FUNC_TRIANGLE:
			{
				waveformValue = _osc.GetTriangle(inputValue * frequency, waveformParameters);
				break;
			}

			case FUNC_PULSE:
			{
				waveformValue = _osc.GetPulse(inputValue * frequency, waveformParameters);
				break;
			}

			case FUNC_PULSERND:
			{
				waveformValue = _osc.GetPulseRandom(inputValue * frequency, waveformParameters);
				break;
			}

			case FUNC_SAW_ANALOG:
			{
				waveformValue = _osc.GetAnalogSaw(inputValue * frequency, waveformParameters);
				break;
			}

			case FUNC_CUSTOM:
			{
				if (!customFuncCurve)
					iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "customFuncCurve is nullptr!"_s));

				waveformValue = customFuncCurve->GetPoint(_osc.GetSawtooth(inputValue * frequency, Oscillator::WaveformParameters(Oscillator::VALUERANGE::RANGE01, false, false, 0.0, 0))).y;

				if (outputInvert)
					waveformValue = 1.0 - waveformValue;

				if (outputRange == Oscillator::VALUERANGE::RANGE11)
					waveformValue = waveformValue * 2.0 - 1.0;

				break;
			}
		}

		// Set waveform value to output port
		port->SetFloat(waveformValue, run);

		return true;
	}

	return false;
}


Bool OscillatorNode::GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags)
{
	if (!description->LoadDescription(ID_OSCILLATORNODE))
		return false;

	flags |= DESCFLAGS_DESC::LOADED;

	GvNode *nodePtr  = static_cast<GvNode*>(node);
	BaseContainer *dataPtr = nodePtr->GetOpContainerInstance();

	const Int32 func = dataPtr->GetInt32(OSC_FUNCTION);

	HideDescriptionElement(node, description, OSC_CUSTOMFUNC, func != FUNC_CUSTOM);
	HideDescriptionElement(node, description, OSC_PULSEWIDTH, func != FUNC_PULSE && func != FUNC_PULSERND);
	HideDescriptionElement(node, description, OSC_HARMONICS, func != FUNC_SAW_ANALOG);
	HideDescriptionElement(node, description, OUTPORT_VALUE, true);
	HideDescriptionElement(node, description, INPORT_X, true);

	return true;
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


///////////////////////////////////////////////////////////////////////////
// Register stuff
///////////////////////////////////////////////////////////////////////////

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

	return GvRegisterOperatorPlugin(ID_OSCILLATORNODE, name, 0, OscillatorNode::Alloc, "gvoscillator"_s, 0, ID_GV_OPCLASS_TYPE_GENERAL, ID_OSCILLATOR_NODEGROUP, ID_GV_IGNORE_OWNER, AutoBitmap("gvoscillator.tif"_s));
}
