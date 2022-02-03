/// \headerfile SBProxy.hpp "SBProxy.hpp"
#include "SBProxy.hpp"

/// \headerfile SEAdenitaCoreSEApp.hpp "SEAdenitaCoreSEApp.hpp"
#include "SEAdenitaCoreSEApp.hpp"


// Class descriptor

// SAMSON Element generator pro tip: complete this descriptor to expose this class to SAMSON and other SAMSON Elements

SB_CLASS_BEGIN(SEAdenitaCoreSEApp);

	SB_CLASS_TYPE(SBCClass::App);
	SB_CLASS_DESCRIPTION("Adenita: DNA Nanostructures Modeling and Visualization Toolkit");
	SB_CLASS_PUBLIC_NAME("Adenita");
	SB_CLASS_GUI_UUID("386506A7-DD8B-69DD-4599-F136C1B91610");
	SB_CLASS_GUI_SHORTCUT("");
	SB_CLASS_ICON_FILE_NAME(SB_ELEMENT_PATH + "/Resource/icons/SEAdenitaCoreSEAppIcon.png");
	SB_CLASS_TOOL_TIP("<b>Adenita</b><br><br>Design nanostructures using DNA strands.");
	SB_CLASS_VERSION_NUMBER("1.0.0");

	SB_FACTORY_BEGIN;

		SB_CONSTRUCTOR_0(SEAdenitaCoreSEApp);

	SB_FACTORY_END;

	SB_INTERFACE_BEGIN;

		SB_FUNCTION_3(void, SEAdenitaCoreSEApp, addPartToDocument, ADNPointer<ADNPart>, bool, SBFolder*);

	SB_INTERFACE_END;

SB_CLASS_END(SEAdenitaCoreSEApp);

