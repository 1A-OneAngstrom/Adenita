#include "ADNFrameAdapters.hpp"

#include <cstddef>

namespace ADNFrameAdapters {

namespace {

[[nodiscard]] double component(const ublas::vector<double>& vector, std::size_t index) noexcept {

	return index < vector.size() ? vector[index] : 0.0;

}

[[nodiscard]] ublas::vector<double> toUblasVector(const ADNFrameUtils::Vec3& vector) {

	ublas::vector<double> result(3, 0.0);
	result[0] = vector.x;
	result[1] = vector.y;
	result[2] = vector.z;
	return result;

}

} // namespace

ADNFrameUtils::Frame frameFromOrientable(const Orientable& orientable) {

	const auto& e1 = orientable.GetE1();
	const auto& e2 = orientable.GetE2();
	const auto& e3 = orientable.GetE3();

	return ADNFrameUtils::Frame{
		ADNFrameUtils::Vec3{ component(e1, 0), component(e1, 1), component(e1, 2) },
		ADNFrameUtils::Vec3{ component(e2, 0), component(e2, 1), component(e2, 2) },
		ADNFrameUtils::Vec3{ component(e3, 0), component(e3, 1), component(e3, 2) }
	};

}

void setFrame(Orientable& orientable, const ADNFrameUtils::Frame& frame) {

	orientable.SetE1(toUblasVector(frame.e1));
	orientable.SetE2(toUblasVector(frame.e2));
	orientable.SetE3(toUblasVector(frame.e3));

}

ADNFrameUtils::Frame sanitizedFrame(const Orientable& orientable,
	const ADNFrameUtils::Frame& fallback) {

	return ADNFrameUtils::orthonormalized(frameFromOrientable(orientable), fallback);

}

void sanitizeFrame(Orientable& orientable, const ADNFrameUtils::Frame& fallback) {

	setFrame(orientable, sanitizedFrame(orientable, fallback));

}

void rotateFrame(Orientable& orientable, const ADNFrameUtils::Mat3& rotation) {

	setFrame(orientable, ADNFrameUtils::orthonormalized(
		ADNFrameUtils::rotated(rotation, frameFromOrientable(orientable))));

}

} // namespace ADNFrameAdapters
