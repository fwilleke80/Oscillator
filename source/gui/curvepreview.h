///////////////////////////////////////////////////
// Oscillator Curve Preview Header
///////////////////////////////////////////////////
// (c) 2010 Jack's Secret Stash
// All rights reserved.
///////////////////////////////////////////////////

#ifndef _CURVEPREVIEW_H_
#define _CURVEPREVIEW_H_

#include "oscillator_ids.h"
#include "wsOscillator.h"

// Resource symbol for use in .res files
#define RESOURCE_SYMBOL       "CURVEPREVIEW"

// Result types (none at all)
static LONG restypetable[] = { CUSTOMDATATYPE_CURVEPREVIEW };

// Internal gadget ID for CurvePreviewArea
#define IDC_CURVEAREA		1000

// Minimum size
#define CURVEPREVIEW_MIN_WIDTH		200
#define CURVEPREVIEW_MIN_HEIGHT		75

// Grid lines
#define CURVEPREVIEW_GRID_X				6
#define CURVEPREVIEW_GRID_Y				4

// Curve display
#define CURVEPREVIEW_CURVE_SUBDIV	20

struct CurveDrawData 
{
	LONG						lCurveType;
	OSCILLATORRANGE	range;
	Bool						bInvert;
	Real						rPulseWidth;
};


///////////////////////////////////////////////////////////////////////////
// Preview Area class declaration (we'll draw the curve into this)
///////////////////////////////////////////////////////////////////////////

class iCurvePreviewGui;
class CurvePreviewArea : public GeUserArea
{
	//friend class iCurvePreviewGui;
private:
	wsOscillator osc;
	void DrawGrid(void);
	void DrawCurve(LONG curveindex);
	Real GetCurveValue(const Real &x);

protected:
	LONG m_lWidth, m_lHeight;
	CurveDrawData	m_DrawData;

public:
	virtual void DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg);
	virtual void Sized(LONG w,LONG h);
	virtual Bool GetMinSize(LONG &w,LONG &h);
	void SetCurveDrawData(const CurveDrawData &data)
	{
		m_DrawData = data;
	}

	CurvePreviewArea(void)
	{
		// Initialize DrawData
		m_DrawData.lCurveType = 1;
		m_DrawData.rPulseWidth = RCO 0.25;
		m_DrawData.range = OSCILLATORRANGE_01;
		m_DrawData.bInvert = FALSE;
	}

	~CurvePreviewArea(void)
	{
	}
};


///////////////////////////////////////////////////////////////////////////
// CustomGui class declaration
///////////////////////////////////////////////////////////////////////////

class iCurvePreviewGui : public iBaseCustomGui
{
	INSTANCEOF(iCurvePreviewGui, iBaseCustomGui)

public:
	iCurvePreviewGui(const BaseContainer &settings,CUSTOMGUIPLUGIN *plugin);

	virtual Bool SetData(const TriState<GeData> &tristate);
	virtual TriState<GeData> GetData();

	virtual Bool CreateLayout();
	virtual Bool InitValues();

	void SetCurveDrawData(const CurveDrawData &data);

	Bool m_bIsTristate;

protected:
	CurvePreviewArea m_area;
	CurveDrawData m_DrawData;
};


///////////////////////////////////////////////////////////////////////////
// CustomGui plugin class declaration
///////////////////////////////////////////////////////////////////////////

class CurvePreviewCustomGuiData : public CustomGuiData 
{
public:
	virtual LONG GetId() { return CUSTOMGUI_CURVEPREVIEW; }

	virtual CDialog* Alloc(const BaseContainer &settings)
	{
		iCurvePreviewGui *dlg = gNew iCurvePreviewGui(settings, GetPlugin());
		if (!dlg) return NULL;

		CDialog *cdlg = dlg->Get();
		if (!cdlg) return NULL;

		return cdlg;
	}

	virtual void Free(CDialog *dlg,void *userdata)
	{
		if (!dlg || !userdata) return;
		iCurvePreviewGui *sub = (iCurvePreviewGui*)userdata;
		gDelete(sub);
	}		

	virtual const CHAR *GetResourceSym()
	{
		return RESOURCE_SYMBOL;
	}

	virtual CustomProperty* GetProperties();
	//{
	//	return g_CurvePreviewProps;
	//}

	virtual LONG GetResourceDataType(LONG *&table)
	{
		table = restypetable;
		return sizeof(restypetable)/sizeof(LONG);
	}
};


///////////////////////////////////////////////////////////////////////////
// CustomDataTypeClass stuff
///////////////////////////////////////////////////////////////////////////

class CurvePreviewData : public iCustomDataType<CurvePreviewData>
{
private:
	CurveDrawData m_data;

public:
  CurvePreviewData() { }
	~CurvePreviewData() { }

	CurveDrawData GetCurveDrawData()
	{
		return m_data;
	}

	void SetCurveDrawData(const CurveDrawData &data)
	{
		m_data = data;
	}
};

class CurvePreviewDataTypeClass : public CustomDataTypeClass
{
	public:

		virtual LONG GetId()
		{
			return CUSTOMDATATYPE_CURVEPREVIEW;
		}

		virtual CustomDataType*	AllocData()
		{
			CurvePreviewData *data = gNew CurvePreviewData;
			if (!data) return NULL;
			return data;
		}
		
		virtual void FreeData(CustomDataType *data)
		{
			CurvePreviewData* d = (CurvePreviewData*)data;
			gDelete(d);
		}

		virtual Bool CopyData(const CustomDataType *src,CustomDataType *dest,AliasTrans *aliastrans)
		{
			CurvePreviewData* s = (CurvePreviewData*)src;
			CurvePreviewData* d = (CurvePreviewData*)dest;
			if (!s || !d) return FALSE;
			return TRUE;
		}
		
		virtual LONG Compare(const CustomDataType *d1,const CustomDataType *d2)
		{
			return 1;
		}

		virtual Bool WriteData(const CustomDataType *d,HyperFile *hf)
		{
			return TRUE;
		}
		
		virtual Bool ReadData (CustomDataType *d,HyperFile *hf,LONG level)
		{
			return TRUE;
		}

		virtual const CHAR *GetResourceSym()
		{
			return RESOURCE_SYMBOL;
		}

		virtual CustomProperty *GetProperties();

		virtual void GetDefaultProperties(BaseContainer &data) 
		{
			//data.FlushAll(); 
			data.SetLong(DESC_CUSTOMGUI, CUSTOMGUI_CURVEPREVIEW);
		}

		virtual Bool _GetDescription(const CustomDataType *data, Description &res, LONG &flags, const BaseContainer &parentdescription, DescID *singledescid)
		{
			return CustomDataTypeClass::_GetDescription(data, res, flags, parentdescription, singledescid);
		};

		virtual Bool GetParameter(const CustomDataType *data, const DescID &id, GeData &t_data, LONG &flags)
		{
			return CustomDataTypeClass::GetParameter(data, id, t_data, flags);
		};

		virtual Bool SetDParameter(CustomDataType *data, const DescID &id, const GeData &t_data, LONG &flags)
		{
			return CustomDataTypeClass::SetDParameter(data, id, t_data, flags);
		};
};

#endif
