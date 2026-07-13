#include "viewer/viewer_spec.h"
#include <cmath>

namespace viewer {

	constexpr float kDegreesToRadians = 3.14159265358979323846f / 180.0f;

	TransformMatrix TransformMatrixBuilder::CreateRotationX(float angle_rad) noexcept {
		TransformMatrix matrix;
		const float cos_a = std::cos(angle_rad);
		const float sin_a = std::sin(angle_rad);

		matrix(1, 1) = cos_a;
		matrix(1, 2) = -sin_a;
		matrix(2, 1) = sin_a;
		matrix(2, 2) = cos_a;

		return matrix;
	}

	TransformMatrix TransformMatrixBuilder::CreateRotationY(float angle_rad) noexcept {
		TransformMatrix matrix;
		const float cos_a = std::cos(angle_rad);
		const float sin_a = std::sin(angle_rad);

		matrix(0, 0) = cos_a;
		matrix(0, 2) = sin_a;
		matrix(2, 0) = -sin_a;
		matrix(2, 2) = cos_a;

		return matrix;
	}

	TransformMatrix TransformMatrixBuilder::CreateRotationZ(float angle_rad) noexcept {
		TransformMatrix matrix;
		const float cos_a = std::cos(angle_rad);
		const float sin_a = std::sin(angle_rad);

		matrix(0, 0) = cos_a;
		matrix(0, 1) = -sin_a;
		matrix(1, 0) = sin_a;
		matrix(1, 1) = cos_a;

		return matrix;
	}

	TransformMatrix TransformMatrixBuilder::CreateRotationMatrix(float x_deg, float y_deg, float z_deg) noexcept {
		const float x_rad = x_deg * kDegreesToRadians;
		const float y_rad = y_deg * kDegreesToRadians;
		const float z_rad = z_deg * kDegreesToRadians;

		return CreateRotationX(x_rad) * CreateRotationY(y_rad) * CreateRotationZ(z_rad);
	}

	TransformMatrix TransformMatrixBuilder::CreateMoveMatrix(float x, float y, float z) noexcept {
		TransformMatrix matrix;

		matrix(0, 3) = x;
		matrix(1, 3) = y;
		matrix(2, 3) = z;

		return matrix;
	}

	TransformMatrix TransformMatrixBuilder::CreateScaleMatrix(float x, float y, float z) noexcept {
		TransformMatrix matrix;

		matrix(0, 0) = (x != 0.0f) ? x : 1.0f;
		matrix(1, 1) = (y != 0.0f) ? y : 1.0f;
		matrix(2, 2) = (z != 0.0f) ? z : 1.0f;

		return matrix;
	}

	TransformMatrix TransformMatrixBuilder::CreateOrthographicMatrix(float left, float right, float bottom, float top, float near, float far) noexcept {
		TransformMatrix matrix;

		matrix(0, 0) = 2.0f / (right - left);
		matrix(1, 1) = 2.0f / (top - bottom);
		matrix(2, 2) = -2.0f / (far - near);
		matrix(0, 3) = -(right + left) / (right - left);
		matrix(1, 3) = -(top + bottom) / (top - bottom);
		matrix(2, 3) = -(far + near) / (far - near);
		matrix(3, 3) = 1.0f;

		return matrix;
	}

	TransformMatrix TransformMatrixBuilder::CreatePerspectiveMatrix(float fov_degrees, float aspect_ratio, float near, float far) noexcept {

		TransformMatrix matrix;
		const float fov_rad = fov_degrees * kDegreesToRadians;
		const float tan_half_fov = std::tan(fov_rad / 2.0f);
		const float n = near;
		const float f = far;

		// Стандартная OpenGL перспективная матрица (column-major)
		matrix(0, 0) = 1.0f / (aspect_ratio * tan_half_fov);
		matrix(0, 1) = 0.0f;
		matrix(0, 2) = 0.0f;
		matrix(0, 3) = 0.0f;

		matrix(1, 0) = 0.0f;
		matrix(1, 1) = 1.0f / tan_half_fov;
		matrix(1, 2) = 0.0f;
		matrix(1, 3) = 0.0f;

		matrix(2, 0) = 0.0f;
		matrix(2, 1) = 0.0f;
		matrix(2, 2) = -(f + n) / (f - n);
		matrix(2, 3) = -(2.0f * f * n) / (f - n);

		matrix(3, 0) = 0.0f;
		matrix(3, 1) = 0.0f;
		matrix(3, 2) = -1.0f;
		matrix(3, 3) = 0.0f;

		return matrix;
	}

}
