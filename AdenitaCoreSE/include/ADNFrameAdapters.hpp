#pragma once

#include "ADNFrameUtils.hpp"
#include "ADNMixins.hpp"

/*! \file ADNFrameAdapters.hpp */

namespace ADNFrameAdapters {

/// \brief Read an \c Orientable basis into a value-type frame copy.
SB_EXPORT [[nodiscard]] ADNFrameUtils::Frame frameFromOrientable(const Orientable& orientable);
/// \brief Write a frame basis back to an \c Orientable object.
SB_EXPORT void setFrame(Orientable& orientable, const ADNFrameUtils::Frame& frame);

/// \brief Return a sanitized frame copy for the given \c Orientable object.
///
/// The returned frame is orthonormal and right-handed. The fallback frame is
/// used when the current basis cannot be repaired robustly.
SB_EXPORT [[nodiscard]] ADNFrameUtils::Frame sanitizedFrame(const Orientable& orientable,
	const ADNFrameUtils::Frame& fallback = ADNFrameUtils::identityFrame());
/// \brief Sanitize the frame stored on an \c Orientable object in place.
SB_EXPORT void sanitizeFrame(Orientable& orientable,
	const ADNFrameUtils::Frame& fallback = ADNFrameUtils::identityFrame());
/// \brief Rotate the frame stored on an \c Orientable object.
SB_EXPORT void rotateFrame(Orientable& orientable, const ADNFrameUtils::Mat3& rotation);

} // namespace ADNFrameAdapters
