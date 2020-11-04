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

		SB_ATTRIBUTE_READ_WRITE_RESET_RANGE(float, SEAdenitaVisualModel, Scale, "Scale", "Properties");
		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, DiscreteScale, "Scale (discrete)", "Properties");

		SB_ATTRIBUTE_READ_WRITE_RESET_RANGE(float, SEAdenitaVisualModel, Dimension, "Dimension", "Properties");
		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, DiscreteDimension, "Dimension (discrete)", "Properties");

		SB_ATTRIBUTE_READ_WRITE_RESET_RANGE(double, SEAdenitaVisualModel, Visibility, "Visibility", "Properties");

		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, Highlight, "Highlight", "Highlight options");
		SB_ATTRIBUTE_READ_WRITE_RESET(bool, SEAdenitaVisualModel, NotWithinRange, "Not within this range", "Highlight options");
		SB_ATTRIBUTE_READ_WRITE_RESET(bool, SEAdenitaVisualModel, NotScaffold, "Not Scaffold", "Highlight options");
		SB_ATTRIBUTE_READ_WRITE(unsigned int, SEAdenitaVisualModel, HighlightMinLength, "Min length (nts)", "Highlight options");
		SB_ATTRIBUTE_READ_WRITE(unsigned int, SEAdenitaVisualModel, HighlightMaxLength, "Max length (nts)", "Highlight options");

		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, ColorType, "Color type", "Colorization");
		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, PropertyColorScheme, "Property color scheme", "Colorization");

		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, SingleStrandColors, "Single strand colors", "Colorization");
		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, NucleotideColors, "Nucleotide colors", "Colorization");
		SB_ATTRIBUTE_READ_WRITE_LIST(SEAdenitaVisualModel, DoubleStrandColors, "Double strand colors", "Colorization");

		SB_ATTRIBUTE_READ_WRITE(bool, SEAdenitaVisualModel, ShowBasePairingFlag, "Show base pairing", "Properties");

	SB_INTERFACE_END;

SB_CLASS_END(SEAdenitaVisualModel);

