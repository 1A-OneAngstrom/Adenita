#pragma once 

#include "SBMVisualModel.hpp"

#include "SBBaseEvent.hpp"
#include "SBDocumentEvent.hpp"
#include "SBStructuralEvent.hpp"


/// This class implements a visual model

class SB_EXPORT SETaggingVisualModel : public SBMVisualModel {

	SB_CLASS

public:

	/// \name Constructors and destructors
	//@{

	SETaggingVisualModel();																												///< Builds a visual model					
	SETaggingVisualModel(const SBNodeIndexer& nodeIndexer);																				///< Builds a visual model 
	virtual ~SETaggingVisualModel();																									///< Destructs the visual model

	//@}

	/// \name Serialization
	//@{

	virtual bool												isSerializable() const override;										///< Returns true when the class is serializable

	virtual void												serialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) const override;		///< Serializes the node
	virtual void												unserialize(SBCSerializer* serializer, const SBNodeIndexer& nodeIndexer, const SBVersionNumber& sdkVersionNumber = SB_SDK_VERSION_NUMBER, const SBVersionNumber& classVersionNumber = SBVersionNumber(1, 0, 0)) override;			///< Unserializes the node

	//@}

	/// \name Topology
	//@{

	virtual	void												onErase() override;														///< Erases the visual model

	//@}

	/// \name Rendering
	//@{

	virtual void												display() override;														///< Displays the visual model
	virtual void												displayForShadow() override;											///< Displays the visual model for shadow purposes
	virtual void												displayForSelection() override;											///< Displays the visual model for selection purposes

	virtual void												expandBounds(SBIAPosition3& bounds) const override;						///< Expands the bounds to make sure the visual model fits inside them

	virtual void												collectAmbientOcclusion(const SBPosition3& boxOrigin, const SBPosition3& boxSize, unsigned int nCellsX, unsigned int nCellsY, unsigned int nCellsZ, float* ambientOcclusionData) override;		///< To collect ambient occlusion data

	//@}

	/// \name Events
	//@{

	virtual void												onBaseEvent(SBBaseEvent* baseEvent);									///< Handles base events
	virtual void												onDocumentEvent(SBDocumentEvent* documentEvent);						///< Handles document events
	virtual void												onStructuralEvent(SBStructuralEvent* documentEvent);					///< Handles structural events

	//@}

	SBNodeIndexer												goldAtoms_;

};


SB_REGISTER_TARGET_TYPE(SETaggingVisualModel, "SETaggingVisualModel", "5E0EB74B-CBEA-175C-43AA-EEC452539522");
SB_DECLARE_BASE_TYPE(SETaggingVisualModel, SBMVisualModel);
