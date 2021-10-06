/// \headerfile SBProxy.hpp "SBProxy.hpp"
#include "SBProxy.hpp"

/// \headerfile SEAdenitaImporterJson.hpp "SEAdenitaImporterJson.hpp"
#include "SEAdenitaImporterJson.hpp"


// Class descriptor

// SAMSON Element generator pro tip: complete this descriptor to expose this class to SAMSON and other SAMSON Elements

SB_CLASS_BEGIN(SEAdenitaImporterJson);

	SB_CLASS_TYPE(SBCClass::Importer);
	SB_CLASS_DESCRIPTION("Adenita importer for Cadnano and legacy Adenita formats (.json)");

	SB_FACTORY_BEGIN;

		SB_CONSTRUCTOR_0(SEAdenitaImporterJson);

	SB_FACTORY_END;

	SB_INTERFACE_BEGIN;

	SB_INTERFACE_END;

SB_CLASS_END(SEAdenitaImporterJson);

