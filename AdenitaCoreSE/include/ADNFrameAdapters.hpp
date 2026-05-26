#pragma once

#include "ADNFrameUtils.hpp"
#include "ADNMixins.hpp"

namespace ADNFrameAdapters {

SB_EXPORT [[nodiscard]] ADNFrameUtils::Frame frameFromOrientable(const Orientable& orientable);
SB_EXPORT void setFrame(Orientable& orientable, const ADNFrameUtils::Frame& frame);

SB_EXPORT [[nodiscard]] ADNFrameUtils::Frame sanitizedFrame(const Orientable& orientable,
	const ADNFrameUtils::Frame& fallback = ADNFrameUtils::identityFrame());
SB_EXPORT void sanitizeFrame(Orientable& orientable,
	const ADNFrameUtils::Frame& fallback = ADNFrameUtils::identityFrame());
SB_EXPORT void rotateFrame(Orientable& orientable, const ADNFrameUtils::Mat3& rotation);

} // namespace ADNFrameAdapters
