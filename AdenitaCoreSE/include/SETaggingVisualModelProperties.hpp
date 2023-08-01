#pragma once


#include "SBGDataGraphNodeProperties.hpp" 
#include "ui_SETaggingVisualModelProperties.h"
#include "SBPointer.hpp"
#include "SETaggingVisualModel.hpp"


/// This class implements the property window of the visual model

// SAMSON Element generator pro tip: add GUI functionality in this class. The non-GUI functionality should go in the SETaggingVisualModel class

// SAMSON Element generator pro tip: if you do not want to have a property window associated to you visual model, do not expose it in the descriptor. 
// Precisely, remove the line SB_ELEMENT_CLASS(SETaggingVisualModelProperties); from the file SETaggingDescriptor.cpp

class SB_EXPORT SETaggingVisualModelProperties : public SBGDataGraphNodeProperties {

	SB_CLASS
	Q_OBJECT

public:

	/// \name Constructors and destructors
	//@{

	SETaggingVisualModelProperties();																									///< Constructs a widget to show the properties of the visual model
	virtual ~SETaggingVisualModelProperties();																							///< Destructs the widget

	//@}

	/// \name Identity
	//@{

	virtual SBCContainerUUID									getUUID() const override;												///< Returns the widget UUID
	virtual QString												getName() const override;												///< Returns the widget name (used as a title for the embedding window)
	virtual QPixmap												getLogo() const override;												///< Returns the widget logo
	virtual int													getFormat() const override;												///< Returns the widget format
	virtual QString												getCitation() const override;											///< Returns the citation information

	//@}

	///\name Settings
	//@{

	virtual void												loadSettings(SBGSettings* settings) override;							///< Load GUI settings
	virtual void												saveSettings(SBGSettings* settings) override;							///< Save GUI settings

	//@}

	///\name Setup
	//@{

	virtual bool												setup() override;														///< Initializes the properties widget
	virtual bool												setup(SBNode* node) override;											///< Initializes the properties widget

	//@}

	class Observer : public SBCReferenceTarget {

		Observer(SETaggingVisualModelProperties* properties);
		virtual ~Observer();

		friend class SETaggingVisualModelProperties;

		void														onBaseEvent(SBBaseEvent* baseEvent);								///< Base event management
		void														onVisualEvent(SBVisualEvent* visualEvent);							///< Visual event management

		SETaggingVisualModelProperties* properties;

	};

private:

	friend class SETaggingVisualModel;
	Ui::SETaggingVisualModelPropertiesClass						ui;
	SBPointer<SETaggingVisualModel>								visualModel;

	SBPointer<Observer>											observer;

};

SB_REGISTER_TYPE(SETaggingVisualModelProperties, "SETaggingVisualModelProperties", "7E07A97C-279E-5D33-9F37-A2D47520C149");
SB_DECLARE_BASE_TYPE(SETaggingVisualModelProperties, SBGDataGraphNodeProperties);
