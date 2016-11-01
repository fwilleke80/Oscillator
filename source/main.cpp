/////////////////////////////////////////////////////////////
// Oscillator Main.cpp
/////////////////////////////////////////////////////////////
// (c) 2010 Jack's Secret Stash
// All rights reserved.
/////////////////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_symbols.h"

// forward declarations
Bool RegisterGvOscillator(void);
Bool RegisterCurvePreviewGui(void);

C4D_CrashHandler old_handler;


void SDKCrashHandler(CHAR *crashinfo)
{
	// don't forget to call the original handler!!!
	if (old_handler) (*old_handler)(crashinfo);
}

Bool RegisterAllPlugins(void)
{
	old_handler = C4DOS.CrashHandler;				// backup the original handler (must be called!)
	C4DOS.CrashHandler = SDKCrashHandler;		// insert the own handler

	GePrint("****************************************");
	if (!RegisterGvOscillator()) goto ErrorHappened;
	GePrint("Oscillator 1.0");
	GePrint("2010 by Jack's Secret Stash (www.c4d-jack.de)");
	GePrint("Freeware, all rights reserved");
	GePrint("****************************************");

	return TRUE;
ErrorHappened:
	GePrint("Oscillator plugin could not be initialized!");
	GePrint("****************************************");
	return FALSE;
}

Bool PluginStart(void)
{
	return RegisterAllPlugins();
}

Bool PluginMessage(LONG id, void *data)
{
	switch (id)	{
		case C4DPL_INIT_SYS:
			if (!resource.Init()) return FALSE;						// don't start plugin without resource
			if (!RegisterCurvePreviewGui()) return FALSE;
			return TRUE;
		case C4DMSG_PRIORITY: 
			return TRUE;
	}

	return FALSE;
}

void PluginEnd(void)
{
}
