///////////////////////////////////////////////////
// Oscillator Curve Preview
///////////////////////////////////////////////////
// (c) 2010 Jack's Secret Stash
// All rights reserved.
///////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_symbols.h"
#include "curvepreview.h"

// CustomGUI CustomProperties
CustomProperty g_CurvePreviewProps[] = 
{
	{ CUSTOMTYPE_END, 0, NULL }
};

///////////////////////////////////////////////////////////////////////////
// Preview Area member functions
///////////////////////////////////////////////////////////////////////////

// PreviewArea has been resized
void CurvePreviewArea::Sized(LONG w,LONG h)
{
	m_lWidth = w;
	m_lHeight = h;
}

// Return minimum size
Bool CurvePreviewArea::GetMinSize(LONG &w, LONG &h)
{
	w = CURVEPREVIEW_MIN_WIDTH;
	h = CURVEPREVIEW_MIN_HEIGHT;
	return TRUE;
}

// Redraw
void CurvePreviewArea::DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg)
{
	OffScreenOn();
	DrawSetPen(COLOR_BG);
	DrawRectangle(0, 0, m_lWidth, m_lHeight);
	DrawGrid();
	DrawCurve(0);
}

// Draw the background grid
void CurvePreviewArea::DrawGrid(void)
{
	LONG i;

	// Set grid color
	DrawSetPen(COLOR_EDGEDK);

	// Vertical Lines
	for (i = 1; i < CURVEPREVIEW_GRID_Y; i++)
		DrawLine(i * m_lWidth / CURVEPREVIEW_GRID_Y, 0, i * m_lWidth / CURVEPREVIEW_GRID_Y, m_lHeight);

	// Horizontal Lines
	for (i = 1; i < CURVEPREVIEW_GRID_X; i++)
		DrawLine(0, i * m_lHeight / CURVEPREVIEW_GRID_X, m_lWidth, i * m_lHeight / CURVEPREVIEW_GRID_X);

	// Frame
	DrawLine(0, 0, m_lWidth, 0);
	DrawLine(0, 0, 0, m_lHeight);
	DrawLine(m_lWidth - 1, 0, m_lWidth - 1, m_lHeight - 1);
	DrawLine(0, m_lHeight - 1, m_lWidth - 1, m_lHeight - 1);

	// Set axis color
	DrawSetPen(COLOR_EDGEBL);

	// Draw axes
	DrawLine(0, m_lHeight / 2, m_lWidth, m_lHeight / 2);
	DrawLine(0, 0, 0, m_lHeight);

}

// Draw the curve
void CurvePreviewArea::DrawCurve(LONG curveindex)
{
	LONG i;
	Real x, y;
	Real px, py;

	// Set curve color
	DrawSetPen(Vector(RCO 1.0, RCO 0.0, RCO 0.0));

	// Initialize previous coordinates
	px = RCO 0.0;
	py = GetCurveValue(RCO 0.0);

	// Draw curve
	for (i = 0; i < m_lWidth; i++)
	{
		// Calculate new coordinates
		x = Real(i + 1) / Real(m_lWidth);
		y = GetCurveValue(x);

		// Draw line
		DrawLine(	(LONG)(m_lWidth * px),
							(LONG)(m_lHeight * py),
							(LONG)(m_lWidth * x),
							(LONG)(m_lHeight * y));

		// Previous coordinates = current coordinates
		px = x;
		py = y;
	}
}

// Get a value from the oscillator. All parameters are taken from m_drawdata
Real CurvePreviewArea::GetCurveValue(const Real &x)
{
	switch (m_DrawData.lCurveType)
	{
		case 1:
			return osc.GetSin(x, m_DrawData.range, m_DrawData.bInvert);
		case 2:
			return osc.GetCos(x, m_DrawData.range, m_DrawData.bInvert);
		case 3:
			return osc.GetSawtooth(x, m_DrawData.range, m_DrawData.bInvert);
		case 4:
			return osc.GetSquare(x, m_DrawData.range, m_DrawData.bInvert);
		case 5:
			return osc.GetTriangle(x, m_DrawData.range, m_DrawData.bInvert);
		case 6:
			return osc.GetPulse(x, m_DrawData.rPulseWidth, FALSE, m_DrawData.range, m_DrawData.bInvert);
		case 7:
			return osc.GetPulse(x, m_DrawData.rPulseWidth, TRUE, m_DrawData.range, m_DrawData.bInvert);

		default:
			return RCO 0.0;
	}
}


///////////////////////////////////////////////////////////////////////////
// Preview Custom Gui member functions
///////////////////////////////////////////////////////////////////////////

// Constructor
iCurvePreviewGui::iCurvePreviewGui(const BaseContainer &settings,CUSTOMGUIPLUGIN *plugin) : iBaseCustomGui(settings,plugin)
{
	// Initialize DrawData
	m_DrawData.lCurveType = 1;
	m_DrawData.rPulseWidth = RCO 0.25;
	m_DrawData.range = OSCILLATORRANGE_01;
	m_DrawData.bInvert = FALSE;

	// Initialize Tristate
	m_bIsTristate = FALSE;
}

// Build CustomGUI layout
Bool iCurvePreviewGui::CreateLayout(void)
{
	GroupBegin(100, BFH_SCALEFIT, 0, 1, String(""), 0);
	AddUserArea(IDC_CURVEAREA, 0);
	GroupEnd();
	AttachUserArea(m_area, IDC_CURVEAREA);

	return SUPER::CreateLayout();
}

// Redraw everything
Bool iCurvePreviewGui::InitValues()
{
	m_area.SetCurveDrawData(m_DrawData);
	m_area.Redraw();
	return TRUE;
}

// Set data into CustomGUI
Bool iCurvePreviewGui::SetData(const TriState<GeData> &tristate)
{
	CurvePreviewData *cpd = (CurvePreviewData*) tristate.GetValue().GetCustomDataType(CUSTOMDATATYPE_CURVEPREVIEW);
	m_DrawData = cpd->GetCurveDrawData();

	m_bIsTristate = tristate.GetTri();
	InitValues();
	return TRUE;
}

// Get data from CustomGUI
TriState<GeData> iCurvePreviewGui::GetData()
{
	TriState<GeData> tri;
	//GeData gd (CUSTOMDATATYPE_CURVEPREVIEW, DEFAULTVALUE);
	//CurvePreviewData *cpd = (CurvePreviewData*) gd.GetCustomDataType(CUSTOMDATATYPE_CURVEPREVIEW);
	//cpd->

	return tri;
}

// Set CurveDrawData
void iCurvePreviewGui::SetCurveDrawData(const CurveDrawData &data)
{
	//m_DrawData = data;
	//m_area.SetCurveDrawData(data);
}


///////////////////////////////////////////////////////////////////////////
// Return CustomProperties
///////////////////////////////////////////////////////////////////////////

CustomProperty* CurvePreviewCustomGuiData::GetProperties()
{
	return g_CurvePreviewProps;
}

CustomProperty* CurvePreviewDataTypeClass::GetProperties()
{
	return g_CurvePreviewProps;
}


///////////////////////////////////////////////////////////////////////////
// Register function
///////////////////////////////////////////////////////////////////////////

Bool RegisterCurvePreviewGui()
{
	// Register dummy CustomDataType
  if (!RegisterCustomDataTypePlugin(GeLoadString(IDS_DATATYPE_CURVEPREVIEW), PLUGINFLAG_HIDE, gNew CurvePreviewDataTypeClass, 0)) return FALSE;

	// Register CustomGUI
	return RegisterCustomGuiPlugin(GeLoadString(IDS_CURVEPREVIEW), 0, gNew CurvePreviewCustomGuiData);
}
