/////////////////////////////////////////////////////////////
// Oscillator :: Oscillator Node
/////////////////////////////////////////////////////////////
// (c) 2010 Jack's Secret Stash
// All rights reserved.
/////////////////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_symbols.h"
#include "c4d_operatordata.h"
#include "gvOscillator.h"
#include "oscillator_ids.h"
#include "curvepreview.h"
#include "wsOscillator.h"

#define API_VERSION_R12		12000

/***************************************************************************
  Class
***************************************************************************/

static LONG input_ids[] = { INPORT_X, OSC_INPUTSCALE, OSC_PULSEWIDTH, OSC_CUSTOMFUNC, 0 }; // Use this for the input ports!

class gvOscillator : public GvOperatorData
{
	// Defines super
	INSTANCEOF(gvOscillator, GvOperatorData)

	private:
		GvValuesInfo	ports;
		wsOscillator	osc;

		gvOscillator(void)
		{
		}

		~gvOscillator(void)
		{
		}

	public:
		Bool Message(GeListNode *node, LONG type, void *data);
		virtual Bool GetDDescription(GeListNode *node, Description *description, DESCFLAGS_DESC &flags);

		virtual Bool iCreateOperator(GvNode *bn);
		virtual const String GetText(GvNode *bn);
		virtual Bool InitCalculation(GvNode *bn, GvCalc *c, GvRun *r);
		virtual void FreeCalculation(GvNode *bn, GvCalc *c);
		virtual Bool Calculate(GvNode *bn, GvPort *port, GvRun *run, GvCalc *calc);

  	static NodeData* Alloc(void) { return gNew gvOscillator; }
};

///////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////

// Get data of a port as GeData
GeData GvGetPortGeData(GvNode* node, GvPort* port, GvRun* run)
{ 
	if (!node || !port) return GeData();

	GvPortDescription pd;
	if (!node->GetPortDescription(port->GetIO(), port->GetMainID(), &pd)) return GeData();

	GvDataInfo* info = GvGetWorld()->GetDataTypeInfo(pd.data_id);
	if (!info) return GeData();

	GvDynamicData data;
	GvAllocDynamicData(node, data, info);

	if (!port->GetData(data.data, data.info->value_handler->value_id, run)) return GeData();

	CUSTOMDATATYPEPLUGIN* pl = FindCustomDataTypePlugin(data.info->data_handler->data_id); 
	if (!pl) return GeData();

	GeData ge_data; 
	if (!CallCustomDataType(pl, ConvertGvToGeData)(data.data, 0, ge_data)) return GeData();

	return ge_data;
}

// Shows or hides a description from the node.
Bool ShowDescription(GeListNode *node, Description *descr, LONG MyDescID, Bool DoHide)
{
	AutoAlloc<AtomArray> ar; if(!ar) return FALSE;
	ar->Append(static_cast<C4DAtom*>(node));

	BaseContainer *bc = descr->GetParameterI(DescLevel(MyDescID), ar);
	if(bc) bc->SetBool(DESC_HIDE, !DoHide);
	else return FALSE;

	return TRUE;
}

// Sets a default spline into a SplineData (e.g. in SplineCurve GUI description)
Bool SetDefaultSplineCurve(BaseContainer *data, const LONG DescID)
{
	GeData gd01 (CUSTOMDATATYPE_SPLINE, DEFAULTVALUE);
	SplineData *p01 = (SplineData*) gd01.GetCustomDataType(CUSTOMDATATYPE_SPLINE);
	if (!p01) return FALSE;

	p01->MakeLinearSpline(-1);
	p01->InsertKnot(RCO 0.0, RCO 0.0, TRUE);
	p01->InsertKnot(RCO 1.0, RCO 1.0, TRUE);
	p01->SetRound(RCO 0.0);

	data->SetData(DescID, gd01);

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////
// Node class members
///////////////////////////////////////////////////////////////////////////

Bool gvOscillator::iCreateOperator(GvNode *bn)
{
	BaseContainer* bc = bn->GetOpContainerInstance();
	if (!bc) return FALSE;

	bc->SetLong(OSC_FUNCTION,			FUNC_SAWTOOTH);
	bc->SetLong(OSC_RANGE,				RANGE_01);
	bc->SetReal(OSC_PULSEWIDTH,		RCO 0.3);
	bc->SetReal(OSC_INPUTSCALE,		RCO 1.0);
	SetDefaultSplineCurve(bc, OSC_CUSTOMFUNC);

	return SUPER::iCreateOperator(bn);
}

// Get node text (when "port names" if OFF)
const String gvOscillator::GetText(GvNode *bn)
{
	switch (bn->GetOpContainerInstance()->GetLong(OSC_FUNCTION))
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
		case FUNC_CUSTOM:
			return GeLoadString(IDS_FUNC_CUSTOM);
	}

	return GeLoadString(IDS_OSCILLATORNODE);
}

Bool gvOscillator::InitCalculation(GvNode *bn, GvCalc *c, GvRun *r)
{
	return GvBuildInValuesTable(bn, ports, c, r, input_ids);   
}

Bool gvOscillator::Message(GeListNode *node, LONG type, void *data) //Durchschreiben
{
	DescriptionCommand *dc = (DescriptionCommand*) data;
	BaseContainer *bc = ((GvNode*)node)->GetOpContainerInstance();

	switch(type)
	{
		case MSG_DESCRIPTION_POSTSETPARAMETER:
			{
				CurveDrawData cdd;
				cdd.bInvert = bc->GetBool(OSC_INVERT);
				cdd.lCurveType = bc->GetLong(OSC_FUNCTION);
				cdd.range = (OSCILLATORRANGE)bc->GetLong(OSC_RANGE);
				cdd.rPulseWidth = bc->GetReal(OSC_PULSEWIDTH);

				GeData gd (CUSTOMDATATYPE_CURVEPREVIEW, DEFAULTVALUE);
				CurvePreviewData *cpd = (CurvePreviewData*) gd.GetCustomDataType(CUSTOMDATATYPE_CURVEPREVIEW);
				cpd->SetCurveDrawData(cdd);

				bc->SetData(OSC_PREVIEW, gd);
				break;
			}
	}

	return TRUE;
}

void gvOscillator::FreeCalculation(GvNode *bn, GvCalc *c)
{
	GvFreeValuesTable(bn, ports);
}

Bool gvOscillator::Calculate(GvNode *bn, GvPort *port, GvRun *run, GvCalc *calc)
{
	// Test if port contains valid pointer
	if(!port || !bn) return FALSE;

	// Get PortValue from Port 0, trigger calculation
	GvValue* PortVal_x = ports.in_values[0];
	if (!PortVal_x) return FALSE;   
	if (!PortVal_x->Calculate(bn, GV_PORT_INPUT_OR_GEDATA, run, calc, 0)) return FALSE;

	// Get PortValue from Port 1, trigger calculation
	PortVal_x = ports.in_values[1];
	if (!PortVal_x) return FALSE;   
	if (!PortVal_x->Calculate(bn, GV_PORT_INPUT_OR_GEDATA, run, calc, 0)) return FALSE;

	// Get PortValue from Port 2, trigger calculation
	PortVal_x = ports.in_values[2];
	if (!PortVal_x) return FALSE;   
	if (!PortVal_x->Calculate(bn, GV_PORT_INPUT_OR_GEDATA, run, calc, 0)) return FALSE;

	// Get PortValue from Port 3, trigger calculation
	PortVal_x = ports.in_values[3];
	if (!PortVal_x) return FALSE;   
	if (!PortVal_x->Calculate(bn, GV_PORT_INPUT_OR_GEDATA, run, calc, 0)) return FALSE;


	// Get node's BaseContainer
	BaseContainer* bc = bn->GetOpContainerInstance();
	if (!bc) return FALSE;

	// Input from port X
	GvPort* iptX = bn->GetInPortFirstMainID(INPORT_X);
	GeData iptdataX = GvGetPortGeData(bn, iptX, run);

	// Input from port INPUT SCALE
	Real rScale = RCO 0.0;
	GvPort* iptScale = bn->GetInPortFirstMainID(OSC_INPUTSCALE);
	GeData iptdataScale = GvGetPortGeData(bn, iptScale, run);
	if (iptScale)
		rScale = iptdataScale.GetReal();
	else
		rScale = bc->GetReal(OSC_INPUTSCALE);

	// Input from port PULSEWIDTH
	Real rPW = RCO 0.0;
	GvPort* iptPW = bn->GetInPortFirstMainID(OSC_PULSEWIDTH);
	GeData iptdataPW = GvGetPortGeData(bn, iptPW, run);
	if (iptPW)
		rPW = iptdataPW.GetReal();
	else
		rPW = bc->GetReal(OSC_PULSEWIDTH);

	// Input from port CUSTOMFUNC
	SplineData	*curve = NULL;
	GvPort* iptCV = bn->GetInPortFirstMainID(OSC_CUSTOMFUNC);
	GeData iptdataCV = GvGetPortGeData(bn, iptCV, run);
	if (iptCV)
		curve = (SplineData*)(iptdataCV.GetCustomDataType(CUSTOMDATATYPE_SPLINE));
	else
		curve = (SplineData*)(bc->GetCustomDataType(OSC_CUSTOMFUNC, CUSTOMDATATYPE_SPLINE));

	switch(port->GetMainID())
	{
		case OUTPORT_VALUE:
		{
			switch (bc->GetLong(OSC_FUNCTION))
			{
				case FUNC_SINE:
					return port->SetReal(osc.GetSin(iptdataX.GetReal() * rScale, (OSCILLATORRANGE)bc->GetLong(OSC_RANGE), bc->GetBool(OSC_INVERT)), run);

				case FUNC_COSINE:
					return port->SetReal(osc.GetCos(iptdataX.GetReal() * rScale, (OSCILLATORRANGE)bc->GetLong(OSC_RANGE), bc->GetBool(OSC_INVERT)), run);

				case FUNC_SAWTOOTH:
					return port->SetReal(osc.GetSawtooth(iptdataX.GetReal() * rScale, (OSCILLATORRANGE)bc->GetLong(OSC_RANGE), bc->GetBool(OSC_INVERT)), run);

				case FUNC_SQUARE:
					return port->SetReal(osc.GetSquare(iptdataX.GetReal() * rScale, (OSCILLATORRANGE)bc->GetLong(OSC_RANGE), bc->GetBool(OSC_INVERT)), run);

				case FUNC_TRIANGLE:
					return port->SetReal(osc.GetTriangle(iptdataX.GetReal() * rScale, (OSCILLATORRANGE)bc->GetLong(OSC_RANGE), bc->GetBool(OSC_INVERT)), run);

				case FUNC_PULSE:
					return port->SetReal(osc.GetPulse(iptdataX.GetReal() * rScale, rPW, FALSE, (OSCILLATORRANGE)bc->GetLong(OSC_RANGE), bc->GetBool(OSC_INVERT)), run);

				case FUNC_PULSERND:
					return port->SetReal(osc.GetPulse(iptdataX.GetReal() * rScale, rPW, TRUE, (OSCILLATORRANGE)bc->GetLong(OSC_RANGE), bc->GetBool(OSC_INVERT)), run);

				case FUNC_CUSTOM:
					{
						if (!curve) return FALSE;

						Real result = curve->GetPoint(osc.GetSawtooth(iptdataX.GetReal() * rScale, OSCILLATORRANGE_01)).y;

						if (bc->GetBool(OSC_INVERT))
							result = RCO 1.0 - result;

						if (bc->GetLong(OSC_RANGE) == RANGE_11)
							result = result * RCO 2.0 - RCO 1.0;

						return port->SetReal(result, run); 
					}
			}
			break;
		}
	}

	return FALSE;
}


#if API_VERSION < API_VERSION_R12
Bool gvOscillator::GetDDescription(GeListNode *node, Description *description, LONG &flags)
#else
Bool gvOscillator::GetDDescription(GeListNode *node, Description *description, DESCFLAGS_DESC &flags)
#endif
{
	if (!description->LoadDescription(ID_OSCILLATORNODE)) return FALSE;
	flags |= DESCFLAGS_DESC_LOADED;

	GvNode				*bn  = (GvNode*)node;
	BaseContainer	*data = bn->GetOpContainerInstance();

	ShowDescription(node, description, OSC_CUSTOMFUNC, data->GetLong(OSC_FUNCTION) == FUNC_CUSTOM);
	ShowDescription(node, description, OSC_PULSEWIDTH, data->GetLong(OSC_FUNCTION) == FUNC_PULSE || data->GetLong(OSC_FUNCTION) == FUNC_PULSERND);
	ShowDescription(node, description, OUTPORT_VALUE, FALSE);
	ShowDescription(node, description, INPORT_X, FALSE);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
// Register stuff
///////////////////////////////////////////////////////////////////////////

// Create empty dummy icon, since it's never used by C4D anyway
BaseBitmap* GetMyGroupIcon()
{
	static AutoAlloc<BaseBitmap> icon;
	if (!icon) return NULL;
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
	String name = GeLoadString(IDS_OSCILLATORNODE); if (!name.Content()) return TRUE;

	// Create SurfaceSPREAD node group (submenu in XPresso editor)
	static GV_OPGROUP_HANDLER mygroup;
	mygroup.group_id = ID_OSCILLATOR_NODEGROUP;
	mygroup.GetName = GetMyGroupName;
	mygroup.GetIcon = GetMyGroupIcon;
	if (!GvRegisterOpGroupType(&mygroup, sizeof(mygroup))) return FALSE;

	return GvRegisterOperatorPlugin(ID_OSCILLATORNODE, name, 0, gvOscillator::Alloc, "gvOscillator", 0,ID_GV_OPCLASS_TYPE_GENERAL, ID_OSCILLATOR_NODEGROUP, ID_GV_IGNORE_OWNER, AutoBitmap("gvOscillator.tif"));
}
