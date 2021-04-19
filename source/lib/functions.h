#ifndef FUNCTIONS_H__
#define FUNCTIONS_H__

///
/// \brief Shows or hides a description element from the node.
///
/// \param[in] node Pointer to the GeListNode that owns the description
/// \param[in] description Pointer to the Desctiption instance
/// \param[in] descId ID of the element to hide
/// \param[in] hide Set to true to hide the element, set to false to show it.
///
inline Bool HideDescriptionElement(GeListNode* node, Description* description, Int32 descId, Bool hide)
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


#endif // FUNCTIONS_H__
