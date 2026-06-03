#pragma once

#include "SBQuantity.hpp"

#include <QString>

namespace SEUIHelper {

/// Keeps SAMSON progress-bar lifetime scoped to the operation that displays it.
///
/// The editors use this for final structure creation, where early returns should
/// not leave stale UI feedback visible. Cancellation defaults to disabled because
/// the current generation code does not expose a safe abort contract for partially
/// generated or inserted ADNPart instances.
class ScopedProgressBar final {

public:

	explicit ScopedProgressBar(
		const QString& name,
		int minimum = 0,
		int maximum = 100,
		const SBQuantity::second& minimumDuration = SBQuantity::second(0.0),
		bool isCancellable = false,
		const QString& cancelButtonText = QStringLiteral("Cancel"));

	~ScopedProgressBar();

	ScopedProgressBar(const ScopedProgressBar&) = delete;
	ScopedProgressBar& operator=(const ScopedProgressBar&) = delete;

	void setValue(int value) const;
	void hide();

	[[nodiscard]] bool isStopped() const;

private:

	bool active_{ false };

};

} // namespace SEUIHelper
