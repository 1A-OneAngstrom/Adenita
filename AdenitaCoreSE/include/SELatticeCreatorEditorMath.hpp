#pragma once

namespace SELatticeCreatorEditorMath {

/// Calculates the zero-to-full row taper for triangular lattice previews.
/// Keeping this pure lets standalone tests cover the intended interpolation
/// without constructing SAMSON editor or rendering state.
[[nodiscard]] inline int calculateTriangleLatticeLength(int rowIndex, int rowCount, int maxBasePairs) {

	if (maxBasePairs <= 0 || rowCount <= 0) return 0;
	if (rowCount == 1) return maxBasePairs;
	if (rowIndex <= 0) return 0;
	if (rowIndex >= rowCount - 1) return maxBasePairs;

	const int denominator = rowCount - 1;
	const long long numerator =
		static_cast<long long>(rowIndex) * static_cast<long long>(maxBasePairs) +
		static_cast<long long>(denominator / 2);

	return static_cast<int>(numerator / denominator);

}

} // namespace SELatticeCreatorEditorMath
