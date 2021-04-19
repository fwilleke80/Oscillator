#include "c4d.h"
#include "c4d_symbols.h"
#include "toscillator.h"
#include "main.h"

#include "customgui_priority.h"


static const Int32 ID_OSCILLATORTAG = 1057129;


class OscillatorTag : public TagData
{
	INSTANCEOF(OscillatorTag, TagData);

public:
	virtual Bool Init(GeListNode* node);

	virtual EXECUTIONRESULT Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags);
	virtual Bool GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags);

	static NodeData* Alloc()
	{
		return NewObj(OscillatorTag) iferr_ignore();

	}
};


Bool OscillatorTag::Init(GeListNode* node)
{
	BaseTag* tagPtr	= static_cast<BaseTag*>(node);
	BaseContainer& dataRef = tagPtr->GetDataInstanceRef();



	return SUPER::Init(node);
}


Bool OscillatorTag::GetDDescription(GeListNode* node, Description* description, DESCFLAGS_DESC& flags)
{
	if (!description->LoadDescription(ID_OSCILLATORTAG))
		return false;

//	const DescID* singleid = description->GetSingleDescID();
//
//	DescID cid = DescLevel(6001, DTYPE_GROUP, 0);
//	if (!singleid || cid.IsPartOf(*singleid, nullptr))	// important to check for speedup c4d!
//	{
//		BaseContainer maingroup = GetCustomDataTypeDefault(DTYPE_GROUP);
//		maingroup.SetString(DESC_NAME, "Main Group"_s);
//		if (!description->SetParameter(cid, maingroup, DescLevel(0)))
//			return true;
//	}
//
//	cid = DescLevel(6002, DTYPE_GROUP, 0);
//	if (!singleid || cid.IsPartOf(*singleid, nullptr))	// important to check for speedup c4d!
//	{
//		BaseContainer subgroup = GetCustomDataTypeDefault(DTYPE_GROUP);
//		subgroup.SetString(DESC_NAME, "Sub Group"_s);
//		if (!description->SetParameter(cid, subgroup, DescLevel(6001)))
//			return true;
//	}
//
//	cid = DescLevel(6003, DTYPE_BOOL, 0);
//	if (!singleid || cid.IsPartOf(*singleid, nullptr))	// important to check for speedup c4d!
//	{
//		BaseContainer locked = GetCustomDataTypeDefault(DTYPE_BOOL);
//		locked.SetString(DESC_NAME, "SPECIAL Locked"_s);
//		locked.SetBool(DESC_DEFAULT, true);
//		if (!description->SetParameter(cid, locked, DescLevel(6002)))
//			return true;
//	}
//
//	cid = DescLevel(6004, DTYPE_BOOL, 0);
//	if (!singleid || cid.IsPartOf(*singleid, nullptr))	// important to check for speedup c4d!
//	{
//		BaseContainer locked = GetCustomDataTypeDefault(DTYPE_BOOL);
//		locked = GetCustomDataTypeDefault(DTYPE_LONG);
//		locked.SetString(DESC_NAME, "SPECIAL Long"_s);
//		locked.SetBool(DESC_DEFAULT, true);
//		if (!description->SetParameter(cid, locked, DescLevel(6002)))
//			return true;
//	}

	flags |= DESCFLAGS_DESC::LOADED;

	return SUPER::GetDDescription(node, description, flags);
}


EXECUTIONRESULT OscillatorTag::Execute(BaseTag* tag, BaseDocument* doc, BaseObject* op, BaseThread* bt, Int32 priority, EXECUTIONFLAGS flags)
{
	const BaseContainer& dataRef = tag->GetDataInstanceRef();

	return EXECUTIONRESULT::OK;
}


Bool RegisterLookAtCamera()
{
	return RegisterTagPlugin(ID_OSCILLATORTAG, GeLoadString(IDS_OSCILLATORTAG), TAG_EXPRESSION | TAG_VISIBLE, OscillatorTag::Alloc, "toscillator"_s, AutoBitmap("oscillator.tif"_s), 0);
}
