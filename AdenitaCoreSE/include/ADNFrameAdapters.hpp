#pragma once

#include "ADNFrameUtils.hpp"
#include "ADNMixins.hpp"

namespace ADNFrameAdapters {

[[nodiscard]] ADNFrameUtils::Frame frameFromOrientable(const Orientable& orientable);
void setFrame(Orientable& orientable, const ADNFrameUtils::Frame& frame);

[[nodiscard]] ADNFrameUtils::Frame sanitizedFrame(const Orientable& orientable,
	const ADNFrameUtils::Frame& fallback = ADNFrameUtils::identityFrame());
void sanitizeFrame(Orientable& orientable,
	const ADNFrameUtils::Frame& fallback = ADNFrameUtils::identityFrame());
void rotateFrame(Orientable& orientable, const ADNFrameUtils::Mat3& rotation);

} // namespace ADNFrameAdapters
