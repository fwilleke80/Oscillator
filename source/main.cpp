#include "c4d_plugin.h"
#include "c4d_resource.h"
#include "c4d_general.h"

#include "main.h"

#include "c4d_symbols.h"


Bool PluginStart()
{
	ApplicationOutput("Oscillator 1.2"_s);
	if (!RegisterGvOscillator())
		return false;
	if (!RegisterOscillatorTag())
		return false;

	return true;
}

Bool PluginMessage(Int32 id, void* data)
{
	switch (id)
	{
		case C4DPL_INIT_SYS:
			if (!g_resource.Init())
				return false;						// don't start plugin without resource
			return true;
		case C4DMSG_PRIORITY: 
			return true;
	}

	return false;
}

void PluginEnd()
{ }
