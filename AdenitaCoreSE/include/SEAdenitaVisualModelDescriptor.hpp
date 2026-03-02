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

		SB_CONSTRUCTOR_0();
		SB_CONSTRUCTOR_1(const SBNodeIndexer&);

	SB_FACTORY_END;

	SB_INTERFACE_BEGIN;
	
		SB_ATTRIBUTE_READ_WRITE(const std::string&, Name, "Name", "Identity");

		SB_ATTRIBUTE_READ_ONLY(bool, Selected, "Selected", "Node");
		SB_ATTRIBUTE_READ_ONLY(bool, Visible, "Visible", "Node");
		SB_ATTRIBUTE_READ_WRITE(bool, SelectionFlag, "Selection flag", "Node");
		SB_ATTRIBUTE_READ_WRITE(bool, VisibilityFlag, "Visibility flag", "Node");
		SB_ATTRIBUTE_READ_ONLY(SBNode*, Parent, "Parent", "Node");
		SB_ATTRIBUTE_READ_ONLY(SBNode*, ThisNode, "Itself", "Node");
		SB_ATTRIBUTE_READ_ONLY(SBNode*, NextNode, "Next", "Node");
		SB_ATTRIBUTE_READ_ONLY(SBNode*, PreviousNode, "Previous", "Node");

		SB_ATTRIBUTE_READ_WRITE_RESET_RANGE(float, Scale, "Scale", "Properties");
		SB_ATTRIBUTE_READ_WRITE_LIST(DiscreteScale, "Scale (discrete)", "Properties");

		SB_ATTRIBUTE_READ_WRITE_RESET_RANGE(float, Dimension, "Dimension", "Properties");
		SB_ATTRIBUTE_READ_WRITE_LIST(DiscreteDimension, "Dimension (discrete)", "Properties");

		SB_ATTRIBUTE_READ_WRITE_RESET_RANGE(double, Visibility, "Visibility", "Properties");

		SB_ATTRIBUTE_READ_WRITE_LIST(Highlight, "Highlight", "Highlight options");

		SB_ATTRIBUTE_BEGIN(SBAttribute::Access::ReadWrite, SBAttribute::Type::Scalar, bool, NotScaffold, "Not scaffold", "Highlight options");
			SB_ATTRIBUTE_GET(bool, getNotScaffold);
			SB_ATTRIBUTE_SET(bool, setNotScaffold);
			SB_ATTRIBUTE_DEFAULT(bool, getDefaultNotScaffold);
			SB_ATTRIBUTE_ENABLED_FLAG(getEnabledFlagForHighlightAttributes);
		SB_ATTRIBUTE_END;
		
		SB_ATTRIBUTE_BEGIN(SBAttribute::Access::ReadWrite, SBAttribute::Type::Scalar, bool, NotWithinRange, "Not within the range", "Highlight options");
			SB_ATTRIBUTE_GET(bool, getNotWithinRange);
			SB_ATTRIBUTE_SET(bool, setNotWithinRange);
			SB_ATTRIBUTE_DEFAULT(bool, getDefaultNotWithinRange);
			SB_ATTRIBUTE_ENABLED_FLAG(getEnabledFlagForHighlightAttributes);
		SB_ATTRIBUTE_END;

		SB_ATTRIBUTE_BEGIN(SBAttribute::Access::ReadWrite, SBAttribute::Type::Scalar, unsigned int, HighlightMinLength, "Min length (nts)", "Highlight options");
			SB_ATTRIBUTE_GET(unsigned int, getHighlightMinLength);
			SB_ATTRIBUTE_SET(unsigned int, setHighlightMinLength);
			SB_ATTRIBUTE_ENABLED_FLAG(getEnabledFlagForHighlightAttributes);
		SB_ATTRIBUTE_END;
		
		SB_ATTRIBUTE_BEGIN(SBAttribute::Access::ReadWrite, SBAttribute::Type::Scalar, unsigned int, HighlightMaxLength, "Max length (nts)", "Highlight options");
			SB_ATTRIBUTE_GET(unsigned int, getHighlightMaxLength);
			SB_ATTRIBUTE_SET(unsigned int, setHighlightMaxLength);
			SB_ATTRIBUTE_ENABLED_FLAG(getEnabledFlagForHighlightAttributes);
		SB_ATTRIBUTE_END;

		SB_ATTRIBUTE_READ_WRITE_LIST(ColorType, "Color type", "Colorization");
		SB_ATTRIBUTE_READ_WRITE_LIST(PropertyColorScheme, "Property color scheme", "Colorization");

		SB_ATTRIBUTE_READ_WRITE_LIST(SingleStrandColors, "Single strand colors", "Colorization");
		SB_ATTRIBUTE_READ_WRITE_LIST(NucleotideColors, "Nucleotide colors", "Colorization");
		SB_ATTRIBUTE_READ_WRITE_LIST(DoubleStrandColors, "Double strand colors", "Colorization");

		SB_ATTRIBUTE_READ_WRITE(bool, ShowBasePairingFlag, "Show base pairing", "Properties");

		SB_ATTRIBUTE_PUSH_BUTTON("Update", "Update", "Display", update);
		SB_ATTRIBUTE_READ_WRITE_RESET_RANGE_SLIDER(unsigned int, Transparency, "Transparency", "Display");
		//SB_ATTRIBUTE_READ_ONLY(SBDDataGraphNodeMaterial*, SEAdenitaVisualModel, Material, "Material", "Display");

	SB_INTERFACE_END;

SB_CLASS_END(SEAdenitaVisualModel);

