/// \headerfile SBProxy.hpp "SBProxy.hpp"
#include "SBProxy.hpp"

/// \headerfile SEAdenitaImporterPly.hpp "SEAdenitaImporterPly.hpp"
#include "SEAdenitaImporterPly.hpp"


// Class descriptor

// SAMSON Element generator pro tip: complete this descriptor to expose this class to SAMSON and other SAMSON Elements

SB_CLASS_BEGIN(SEAdenitaImporterPly);

	SB_CLASS_TYPE(SBCClass::Importer);
	SB_CLASS_DESCRIPTION("Adenita importer for Cadnano mesh format (.ply)");

	SB_FACTORY_BEGIN;

		SB_CONSTRUCTOR_0(SEAdenitaImporterPly);

	SB_FACTORY_END;

	SB_INTERFACE_BEGIN;

	SB_INTERFACE_END;

SB_CLASS_END(SEAdenitaImporterPly);

