#include "SEUIHelper.hpp"

#include "SAMSON.hpp"

namespace SEUIHelper {

ScopedProgressBar::ScopedProgressBar(
	const QString& name,
	int minimum,
	int maximum,
	const SBQuantity::second& minimumDuration,
	bool isCancellable,
	const QString& cancelButtonText) {

#if 1
	// disable showing the progress bar for now due to a flickering issue
	active_ = false;
#else
	active_ = true;
	SAMSON::showProgressBar(name, minimum, maximum, minimumDuration, isCancellable, cancelButtonText);
	SAMSON::setProgressBarValue(minimum);
#endif

}

ScopedProgressBar::~ScopedProgressBar() {

	hide();

}

void ScopedProgressBar::setValue(int value) const {

	if (!active_) return;

	SAMSON::setProgressBarValue(value);

}

void ScopedProgressBar::hide() {

	if (!active_) return;

	SAMSON::hideProgressBar();
	active_ = false;

}

//bool ScopedProgressBar::isStopped() const {
//
//	return active_ && SAMSON::isProgressBarStopped();
//
//}

} // namespace SEUIHelper
