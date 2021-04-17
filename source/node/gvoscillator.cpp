#include "c4d.h"
#include "c4d_operatordata.h"
#include "c4d_basebitmap.h"

// #include "curvepreview.h"
#include "oscillator.h"

#include "main.h"
#include "gvoscillator.h"
#include "c4d_symbols.h"


const Int32 ID_OSCILLATORNODE = 1057105;
const Int32 ID_OSCILLATOR_NODEGROUP = 1057106;


// Use this for the input ports!
static Int32 g_input_ids[] = {
	INPORT_X,
	OSC_INPUTSCALE,
	OSC_PULSEWIDTH,
	OSC_CUSTOMFUNC,
	0
};

class gvOscillator : public GvOperatorData
{
	INSTANCEOF(gvOscillator, GvOperatorData);

public:
	virtual Bool Message(GeListNode* node, Int32 type, void* data) override;
	virtual Bool GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags) override;

	virtual Bool iCreateOperator(GvNode* bn) override;
	virtual const String GetText(GvNode* bn) override;
	virtual Bool InitCalculation(GvNode* bn, GvCalc* c, GvRun* r) override;
	virtual void FreeCalculation(GvNode* bn, GvCalc* c) override;
	virtual Bool Calculate(GvNode* bn, GvPort* port, GvRun* run, GvCalc* calc) override;

private:
	GvValuesInfo _ports;
	Oscillator _osc;

public:
	static NodeData* Alloc()
	{
		return NewObj(gvOscillator) iferr_ignore();
	}
};

///////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////

// Get data of a port as GeData
static GeData GvGetPortGeData(GvNode* node, GvPort* port, GvRun* run)
{
	iferr_scope_handler
	{
		ApplicationOutput("@", err.GetMessage());
		return GeData();
	};

	if (!node || !port || !run)
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "Any of the arguments is NULL!"_s));

	GvPortDescription portDescription;
	if (!node->GetPortDescription(port->GetIO(), port->GetMainID(), &portDescription))
		iferr_throw(maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "GetPortDescription() failed!"_s));

	GvDataInfo* dataInfo = GvGetWorld()->GetDataTypeInfo(portDescription.data_id);
	if (!dataInfo)
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "GetDataTypeInfo() returned NULL!"_s));

	GvDynamicData data;
	GvAllocDynamicData(node, data, dataInfo);

	if (!port->GetData(data.data, data.info->value_handler->value_id, run))
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "GetData() failed!"_s));

	CUSTOMDATATYPEPLUGIN* datatypePlugin = FindCustomDataTypePlugin(data.info->data_handler->data_id);
	if (!datatypePlugin)
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "FindCustomDataTypePlugin() returned NULL!"_s));

	GeData geData;
	if (!CallCustomDataType(datatypePlugin, ConvertGvToGeData)(data.data, 0, geData))
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "CallCustomDataType() failed!"_s));

	return geData;
}

// Shows or hides a description from the node.
static Bool HideDescription(GeListNode* node, Description* description, Int32 descID, Bool hide)
{
	AutoAlloc<AtomArray> ar;
	if (!ar)
		return false;
	ar->Append(static_cast<C4DAtom*>(node));

	BaseContainer *bc = description->GetParameterI(DescLevel(descID), ar);
	if (bc)
		bc->SetBool(DESC_HIDE, hide);
	else
		return false;

	return true;
}

// Sets a default spline into a SplineData (e.g. in SplineCurve GUI description)
static Bool SetDefaultSplineCurve(BaseContainer* data, const Int32 DescID)
{
	GeData gdCurve (CUSTOMDATATYPE_SPLINE, DEFAULTVALUE);
	SplineData* splineCurve = (SplineData*)gdCurve.GetCustomDataType(CUSTOMDATATYPE_SPLINE);
	if (!splineCurve)
		return false;

	// Set default spline
	splineCurve->MakeLinearSplineBezier();
	splineCurve->InsertKnot(0.0, 0.0, 0);
	splineCurve->InsertKnot(1.0, 1.0, 0);

	data->SetData(DescID, gdCurve);

	return true;
}


///////////////////////////////////////////////////////////////////////////
// Node class members
///////////////////////////////////////////////////////////////////////////

Bool gvOscillator::iCreateOperator(GvNode* bn)
{
	BaseContainer* dataPtr = bn->GetOpContainerInstance();
	if (!dataPtr)
		return false;

	dataPtr->SetInt32(OSC_FUNCTION, FUNC_SAWTOOTH);
	dataPtr->SetInt32(OSC_RANGE, RANGE_01);
	dataPtr->SetFloat(OSC_PULSEWIDTH, 0.3);
	dataPtr->SetFloat(OSC_INPUTSCALE, 1.0);
	dataPtr->SetUInt32(OSC_HARMONICS, 2);
	SetDefaultSplineCurve(dataPtr, OSC_CUSTOMFUNC);

	return SUPER::iCreateOperator(bn);
}

// Get node text (when "port names" if OFF)
const String gvOscillator::GetText(GvNode* bn)
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

Bool gvOscillator::InitCalculation(GvNode* bn, GvCalc* c, GvRun* r)
{
	return GvBuildInValuesTable(bn, _ports, c, r, g_input_ids);
}

Bool gvOscillator::Message(GeListNode* node, Int32 type, void* data)
{
//	GvNode* nodePtr = static_cast<GvNode*>(node);
//
//	DescriptionCommand *dc = (DescriptionCommand*)data;
//	BaseContainer* dataPtr = nodePtr->GetOpContainerInstance();
//
//	switch (type)
//	{
//		case MSG_DESCRIPTION_POSTSETPARAMETER:
//		{
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

void gvOscillator::FreeCalculation(GvNode *bn, GvCalc *c)
{
	GvFreeValuesTable(bn, _ports);
}

Bool gvOscillator::Calculate(GvNode *bn, GvPort *port, GvRun *run, GvCalc *calc)
{
	iferr_scope_handler
	{
		ApplicationOutput("@", err.GetMessage());
		return false;
	};

	// Check for nullptr
	if (!port || !bn || !run || !calc)
		return false;

	// Calculate input ports
	// ---------------------

	// Get PortValue from Port 0, trigger calculation
	GvValue* portValue = _ports.in_values[0];
	if (!portValue)
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "Input port value is NULL!"_s));
	if (!portValue->Calculate(bn, GV_PORT_INPUT_OR_GEDATA, run, calc, 0))
		iferr_throw(maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "Input port value could not be calculated!"_s));

	// Get PortValue from Port 1, trigger calculation
	portValue = _ports.in_values[1];
	if (!portValue)
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "Input port value is NULL!"_s));
	if (!portValue->Calculate(bn, GV_PORT_INPUT_OR_GEDATA, run, calc, 0))
		iferr_throw(maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "Input port value could not be calculated!"_s));

	// Get PortValue from Port 2, trigger calculation
	portValue = _ports.in_values[2];
	if (!portValue)
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "Input port value is NULL!"_s));
	if (!portValue->Calculate(bn, GV_PORT_INPUT_OR_GEDATA, run, calc, 0))
		iferr_throw(maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "Input port value could not be calculated!"_s));

	// Get PortValue from Port 3, trigger calculation
	portValue = _ports.in_values[3];
	if (!portValue)
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "Input port value is NULL!"_s));
	if (!portValue->Calculate(bn, GV_PORT_INPUT_OR_GEDATA, run, calc, 0))
		iferr_throw(maxon::UnexpectedError(MAXON_SOURCE_LOCATION, "Input port value could not be calculated!"_s));


	// Get node's BaseContainer
	BaseContainer* dataPtr = bn->GetOpContainerInstance();
	if (!dataPtr)
		iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "GetOpContainerInstance() returned NULL!"_s));

	const Oscillator::VALUERANGE outputRange = (Oscillator::VALUERANGE)dataPtr->GetInt32(OSC_RANGE);
	const Bool outputInvert = dataPtr->GetBool(OSC_INVERT);

	// Input from port X
	GvPort* iptX = bn->GetInPortFirstMainID(INPORT_X);
	GeData iptdataX = GvGetPortGeData(bn, iptX, run);
	const Float inputValue = iptdataX.GetFloat();

	// Input from port INPUT SCALE
	GvPort* iptScale = bn->GetInPortFirstMainID(OSC_INPUTSCALE);
	GeData iptdataScale = GvGetPortGeData(bn, iptScale, run);
	Float inputScale = 0.0;
	if (iptScale)
		inputScale = iptdataScale.GetFloat();
	else
		inputScale = dataPtr->GetFloat(OSC_INPUTSCALE);

	// Input from port PULSEWIDTH
	GvPort* iptPW = bn->GetInPortFirstMainID(OSC_PULSEWIDTH);
	GeData iptdataPW = GvGetPortGeData(bn, iptPW, run);
	Float pulseWidth = 0.0;
	if (iptPW)
		pulseWidth = iptdataPW.GetFloat();
	else
		pulseWidth = dataPtr->GetFloat(OSC_PULSEWIDTH);

	// Input from port CUSTOMFUNC
	GvPort* iptCV = bn->GetInPortFirstMainID(OSC_CUSTOMFUNC);
	GeData iptdataCV = GvGetPortGeData(bn, iptCV, run);
	SplineData* customFuncCurve = nullptr;
	if (iptCV)
		customFuncCurve = static_cast<SplineData*>(iptdataCV.GetCustomDataType(CUSTOMDATATYPE_SPLINE));
	else
		customFuncCurve = (SplineData*)(dataPtr->GetCustomDataType(OSC_CUSTOMFUNC, CUSTOMDATATYPE_SPLINE));

	switch (port->GetMainID())
	{
		case OUTPORT_VALUE:
		{
			switch (dataPtr->GetInt32(OSC_FUNCTION))
			{
				case FUNC_SINE:
					return port->SetFloat(_osc.GetSin(inputValue * inputScale, outputRange, outputInvert), run);

				case FUNC_COSINE:
					return port->SetFloat(_osc.GetCos(inputValue * inputScale, outputRange, outputInvert), run);

				case FUNC_SAWTOOTH:
					return port->SetFloat(_osc.GetSawtooth(inputValue * inputScale, outputRange, outputInvert), run);

				case FUNC_SQUARE:
					return port->SetFloat(_osc.GetSquare(inputValue * inputScale, outputRange, outputInvert), run);

				case FUNC_TRIANGLE:
					return port->SetFloat(_osc.GetTriangle(inputValue * inputScale, outputRange, outputInvert), run);

				case FUNC_PULSE:
					return port->SetFloat(_osc.GetPulse(inputValue * inputScale, pulseWidth, false, outputRange, outputInvert), run);

				case FUNC_PULSERND:
					return port->SetFloat(_osc.GetPulse(inputValue * inputScale, pulseWidth, true, outputRange, outputInvert), run);

				case FUNC_SAW_ANALOG:
					return port->SetFloat(_osc.GetAnalogSaw(inputValue * inputScale, dataPtr->GetUInt32(OSC_HARMONICS), outputRange, outputInvert), run);

				case FUNC_CUSTOM:
				{
					if (!customFuncCurve)
						iferr_throw(maxon::NullptrError(MAXON_SOURCE_LOCATION, "customFuncCurve is NULL!"_s));

					Float result = customFuncCurve->GetPoint(_osc.GetSawtooth(iptdataX.GetFloat() * inputScale, Oscillator::VALUERANGE::RANGE01)).y;

					if (dataPtr->GetBool(OSC_INVERT))
						result = 1.0 - result;

					if (dataPtr->GetInt32(OSC_RANGE) == RANGE_11)
						result = result * 2.0 - 1.0;

					return port->SetFloat(result, run);
				}
			}
			break;
		}
	}

	return false;
}


Bool gvOscillator::GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags)
{
	if (!description->LoadDescription(ID_OSCILLATORNODE))
		return false;

	flags |= DESCFLAGS_DESC::LOADED;

	GvNode *nodePtr  = static_cast<GvNode*>(node);
	BaseContainer *dataPtr = nodePtr->GetOpContainerInstance();

	const Int32 func = dataPtr->GetInt32(OSC_FUNCTION);

	HideDescription(node, description, OSC_CUSTOMFUNC, func != FUNC_CUSTOM);
	HideDescription(node, description, OSC_PULSEWIDTH, func != FUNC_PULSE && func != FUNC_PULSERND);
	HideDescription(node, description, OSC_HARMONICS, func != FUNC_SAW_ANALOG);
	HideDescription(node, description, OUTPORT_VALUE, true);
	HideDescription(node, description, INPORT_X, true);

	return true;
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
	static String mygroup(GeLoadString(IDS_OSCILLATOR_NODEGROUP));
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

	return GvRegisterOperatorPlugin(ID_OSCILLATORNODE, name, 0, gvOscillator::Alloc, "gvoscillator"_s, 0, ID_GV_OPCLASS_TYPE_GENERAL, ID_OSCILLATOR_NODEGROUP, ID_GV_IGNORE_OWNER, AutoBitmap("gvoscillator.tif"_s));
}
