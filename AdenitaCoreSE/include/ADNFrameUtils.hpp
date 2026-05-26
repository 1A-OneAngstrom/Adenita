#pragma once

#include <cmath>

namespace ADNFrameUtils {

struct Vec3 {
	double x{};
	double y{};
	double z{};
};

struct Mat3 {
	double m[3][3]{};
};

struct Frame {
	Vec3 e1{};
	Vec3 e2{};
	Vec3 e3{};
};

[[nodiscard]] Vec3 operator+(const Vec3& a, const Vec3& b) noexcept;
[[nodiscard]] Vec3 operator-(const Vec3& a, const Vec3& b) noexcept;
[[nodiscard]] Vec3 operator-(const Vec3& v) noexcept;
[[nodiscard]] Vec3 operator*(const Vec3& v, double scalar) noexcept;
[[nodiscard]] Vec3 operator*(double scalar, const Vec3& v) noexcept;
[[nodiscard]] Vec3 operator/(const Vec3& v, double scalar) noexcept;

[[nodiscard]] Frame identityFrame() noexcept;

[[nodiscard]] double dot(const Vec3& a, const Vec3& b) noexcept;
[[nodiscard]] Vec3 cross(const Vec3& a, const Vec3& b) noexcept;
[[nodiscard]] double squaredNorm(const Vec3& v) noexcept;
[[nodiscard]] double norm(const Vec3& v) noexcept;
[[nodiscard]] Vec3 normalized(const Vec3& v, double eps = 1.0e-12) noexcept;
[[nodiscard]] bool isNearlyZero(const Vec3& v, double eps = 1.0e-12) noexcept;
[[nodiscard]] Vec3 rotated(const Mat3& r, const Vec3& v) noexcept;

[[nodiscard]] double determinant(const Frame& f) noexcept;

[[nodiscard]] bool isFinite(const Vec3& v) noexcept;
[[nodiscard]] bool isNearlyUnit(const Vec3& v, double eps = 1.0e-6) noexcept;
[[nodiscard]] bool isOrthonormalRightHanded(const Frame& f, double eps = 1.0e-6) noexcept;

[[nodiscard]] Frame rotated(const Mat3& r, const Frame& f) noexcept;
[[nodiscard]] Frame orthonormalized(const Frame& f,
	const Frame& fallback = Frame{
		Vec3{ 1.0, 0.0, 0.0 },
		Vec3{ 0.0, 1.0, 0.0 },
		Vec3{ 0.0, 0.0, 1.0 } },
	double eps = 1.0e-10) noexcept;

[[nodiscard]] Frame frameFromE2AndTangent(const Vec3& e2,
	const Vec3& tangent,
	const Frame& fallback = Frame{
		Vec3{ 1.0, 0.0, 0.0 },
		Vec3{ 0.0, 1.0, 0.0 },
		Vec3{ 0.0, 0.0, 1.0 } },
	double eps = 1.0e-10) noexcept;

[[nodiscard]] Mat3 rotationZ(double radians) noexcept;

} // namespace ADNFrameUtils
