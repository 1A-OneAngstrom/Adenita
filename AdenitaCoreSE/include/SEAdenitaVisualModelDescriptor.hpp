/// \headerfile SBProxy.hpp "SBProxy.hpp"
#include "SBProxy.hpp"

/// \headerfile SEAdenitaVisualModel.hpp "SEAdenitaVisualModel.hpp"
#include "SEAdenitaVisualModel.hpp"


// Class descriptor

// SAMSON Element generator pro tip: complete this descriptor to expose this class to SAMSON and other SAMSON Elements

SB_CLASS_BEGIN(SEAdenitaVisualModel);

	SB_CLASS_TYPE(SBCClass::VisualModel);
	SB_CLASS_DESCRIPTION("Adenita Visual Model");

	SB_FACTORY_BEGIN;

		SB_CONSTRUCTOR_0(SEAdenitaVisualModel);
		SB_CONSTRUCTOR_1(SEAdenitaVisualModel, const SBNodeIndexer&);

	SB_FACTORY_END;

	SB_INTERFACE_BEGIN;
	
		SB_ATTRIBUTE_READ_WRITE(const std::string&, SEAdenitaVisualModel, Name, "Name", "Identity");

		SB_ATTRIBUTE_READ_ONLY(bool, SEAdenitaVisualModel, Selected, "Selected", "Node");
		SB_ATTRIBUTE_READ_ONLY(bool, SEAdenitaVisualModel, Visible, "Visible", "Node");
		SB_ATTRIBUTE_READ_WRITE(bool, SEAdenitaVisualModel, SelectionFlag, "Selection flag", "Node");
		SB_ATTRIBUTE_READ_WRITE(bool, SEAdenitaVisualModel, VisibilityFlag, "Visibility flag", "Node");
		SB_ATTRIBUTE_READ_ONLY(SBNode*, SEAdenitaVisualModel, Parent, "Parent", "Node");
		SB_ATTRIBUTE_READ_ONLY(SBNode*, SEAdenitaVisualModel, ThisNode, "Itself", "Node");
		SB_ATTRIBUTE_READ_ONLY(SBNode*, SEAdenitaVisualModel, NextNode, "Next", "Node");
		SB_ATTRIBUTE_READ_ONLY(SBNode*, SEAdenitaVisualModel, PreviousNode, "Previous", "Node");

		SB_ATTRIBUTE_READ_WRITE_RESET_RANGE(float, SEAdenitaVisualModel, Scale, "Scale", "Properties");
		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, DiscreteScale, "Scale (discrete)", "Properties");

		SB_ATTRIBUTE_READ_WRITE_RESET_RANGE(float, SEAdenitaVisualModel, Dimension, "Dimension", "Properties");
		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, DiscreteDimension, "Dimension (discrete)", "Properties");

		SB_ATTRIBUTE_READ_WRITE_RESET_RANGE(double, SEAdenitaVisualModel, Visibility, "Visibility", "Properties");

		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, Highlight, "Highlight", "Highlight options");

		SB_ATTRIBUTE_BEGIN(SBAttribute::Type::ReadWrite, bool, SEAdenitaVisualModel, NotScaffold, "Not scaffold", "Highlight options");
			SB_ATTRIBUTE_GET(bool, SEAdenitaVisualModel, getNotScaffold);
			SB_ATTRIBUTE_SET(bool, SEAdenitaVisualModel, setNotScaffold);
			SB_ATTRIBUTE_DEFAULT(bool, SEAdenitaVisualModel, getDefaultNotScaffold);
			SB_ATTRIBUTE_ENABLED_FLAG(SEAdenitaVisualModel, getEnabledFlagForHighlightAttributes);
		SB_ATTRIBUTE_END;
		
		SB_ATTRIBUTE_BEGIN(SBAttribute::Type::ReadWrite, bool, SEAdenitaVisualModel, NotWithinRange, "Not within the range", "Highlight options");
			SB_ATTRIBUTE_GET(bool, SEAdenitaVisualModel, getNotWithinRange);
			SB_ATTRIBUTE_SET(bool, SEAdenitaVisualModel, setNotWithinRange);
			SB_ATTRIBUTE_DEFAULT(bool, SEAdenitaVisualModel, getDefaultNotWithinRange);
			SB_ATTRIBUTE_ENABLED_FLAG(SEAdenitaVisualModel, getEnabledFlagForHighlightAttributes);
		SB_ATTRIBUTE_END;

		SB_ATTRIBUTE_BEGIN(SBAttribute::Type::ReadWrite, unsigned int, SEAdenitaVisualModel, HighlightMinLength, "Min length (nts)", "Highlight options");
			SB_ATTRIBUTE_GET(unsigned int, SEAdenitaVisualModel, getHighlightMinLength);
			SB_ATTRIBUTE_SET(unsigned int, SEAdenitaVisualModel, setHighlightMinLength);
			SB_ATTRIBUTE_ENABLED_FLAG(SEAdenitaVisualModel, getEnabledFlagForHighlightAttributes);
		SB_ATTRIBUTE_END;
		
		SB_ATTRIBUTE_BEGIN(SBAttribute::Type::ReadWrite, unsigned int, SEAdenitaVisualModel, HighlightMaxLength, "Max length (nts)", "Highlight options");
			SB_ATTRIBUTE_GET(unsigned int, SEAdenitaVisualModel, getHighlightMaxLength);
			SB_ATTRIBUTE_SET(unsigned int, SEAdenitaVisualModel, setHighlightMaxLength);
			SB_ATTRIBUTE_ENABLED_FLAG(SEAdenitaVisualModel, getEnabledFlagForHighlightAttributes);
		SB_ATTRIBUTE_END;

		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, ColorType, "Color type", "Colorization");
		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, PropertyColorScheme, "Property color scheme", "Colorization");

		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, SingleStrandColors, "Single strand colors", "Colorization");
		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, NucleotideColors, "Nucleotide colors", "Colorization");
		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, DoubleStrandColors, "Double strand colors", "Colorization");

		SB_ATTRIBUTE_READ_WRITE(bool, SEAdenitaVisualModel, ShowBasePairingFlag, "Show base pairing", "Properties");

		SB_ATTRIBUTE_PUSH_BUTTON(SEAdenitaVisualModel, "Update", "Update", "Display", update);
		SB_ATTRIBUTE_READ_WRITE_RESET_RANGE_SLIDER(unsigned int, SEAdenitaVisualModel, Transparency, "Transparency", "Display");
		//SB_ATTRIBUTE_READ_ONLY(SBDDataGraphNodeMaterial*, SEAdenitaVisualModel, Material, "Material", "Display");

	SB_INTERFACE_END;

SB_CLASS_END(SEAdenitaVisualModel);

