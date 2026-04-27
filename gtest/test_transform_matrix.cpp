#include <gtest/gtest.h>
#include <viewer/viewerSpec.h>
#include <cmath>

constexpr float kEpsilon = 1e-5f;

// =====================================
// ========== TransformMatrix ==========
// =====================================

TEST(TransformMatrixTest, DefaultConstructorCreatesIdentity) {
    s21::TransformMatrix matrix;

    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            if (i == j) {
                EXPECT_NEAR(matrix(i, j), 1.0f, kEpsilon);
            } else {
                EXPECT_NEAR(matrix(i, j), 0.0f, kEpsilon);
            }
        }
    }
}

TEST(TransformMatrixTest, CopyConstructor) {
    s21::TransformMatrix original;
    original(0, 0) = 5.0f;
    original(1, 2) = 3.0f;

    s21::TransformMatrix copy(original);

    EXPECT_NEAR(copy(0, 0), 5.0f, kEpsilon);
    EXPECT_NEAR(copy(1, 2), 3.0f, kEpsilon);
}

TEST(TransformMatrixTest, AssignmentOperator) {
    s21::TransformMatrix matrix1;
    matrix1(2, 3) = 7.0f;

    s21::TransformMatrix matrix2;
    matrix2 = matrix1;

    EXPECT_NEAR(matrix2(2, 3), 7.0f, kEpsilon);
}

TEST(TransformMatrixTest, MatrixMultiplication) {
    s21::TransformMatrix m1;
    s21::TransformMatrix m2;

    m1(0, 0) = 2.0f;
    m2(0, 0) = 3.0f;

    s21::TransformMatrix result = m1 * m2;

    EXPECT_NEAR(result(0, 0), 6.0f, kEpsilon);
}

TEST(TransformMatrixTest, TransformPointWithIdentity) {
    s21::TransformMatrix identity;
    s21::Point3D point(1.0f, 2.0f, 3.0f);

    s21::Point3D result = identity.TransformPoint(point);

    EXPECT_NEAR(result.x, 1.0f, kEpsilon);
    EXPECT_NEAR(result.y, 2.0f, kEpsilon);
    EXPECT_NEAR(result.z, 3.0f, kEpsilon);
}

// =====================================
// ======= TransformMatrixBuilder ======
// =====================================

TEST(TransformMatrixBuilderTest, CreateMoveMatrix) {
    s21::TransformMatrix move = s21::TransformMatrixBuilder::CreateMoveMatrix(5.0f, 10.0f, 15.0f);

    EXPECT_NEAR(move(0, 3), 5.0f, kEpsilon);
    EXPECT_NEAR(move(1, 3), 10.0f, kEpsilon);
    EXPECT_NEAR(move(2, 3), 15.0f, kEpsilon);
    EXPECT_NEAR(move(3, 3), 1.0f, kEpsilon);
}

TEST(TransformMatrixBuilderTest, MovePoint) {
    s21::TransformMatrix move = s21::TransformMatrixBuilder::CreateMoveMatrix(1.0f, 2.0f, 3.0f);
    s21::Point3D point(0.0f, 0.0f, 0.0f);

    s21::Point3D result = move.TransformPoint(point);

    EXPECT_NEAR(result.x, 1.0f, kEpsilon);
    EXPECT_NEAR(result.y, 2.0f, kEpsilon);
    EXPECT_NEAR(result.z, 3.0f, kEpsilon);
}

TEST(TransformMatrixBuilderTest, CreateScaleMatrix) {
    s21::TransformMatrix scale = s21::TransformMatrixBuilder::CreateScaleMatrix(2.0f, 3.0f, 4.0f);

    EXPECT_NEAR(scale(0, 0), 2.0f, kEpsilon);
    EXPECT_NEAR(scale(1, 1), 3.0f, kEpsilon);
    EXPECT_NEAR(scale(2, 2), 4.0f, kEpsilon);
}

TEST(TransformMatrixBuilderTest, ScalePoint) {
    s21::TransformMatrix scale = s21::TransformMatrixBuilder::CreateScaleMatrix(2.0f, 2.0f, 2.0f);
    s21::Point3D point(1.0f, 1.0f, 1.0f);

    s21::Point3D result = scale.TransformPoint(point);

    EXPECT_NEAR(result.x, 2.0f, kEpsilon);
    EXPECT_NEAR(result.y, 2.0f, kEpsilon);
    EXPECT_NEAR(result.z, 2.0f, kEpsilon);
}

TEST(TransformMatrixBuilderTest, CreateRotationMatrixZ) {
    // Поворот на 90 градусов вокруг Z
    s21::TransformMatrix rot = s21::TransformMatrixBuilder::CreateRotationMatrix(0.0f, 0.0f, 90.0f);

    // cos(90°) ≈ 0, sin(90°) ≈ 1
    EXPECT_NEAR(rot(0, 0), 0.0f, kEpsilon);
    EXPECT_NEAR(rot(0, 1), -1.0f, kEpsilon);
    EXPECT_NEAR(rot(1, 0), 1.0f, kEpsilon);
    EXPECT_NEAR(rot(1, 1), 0.0f, kEpsilon);
}

TEST(TransformMatrixBuilderTest, RotatePoint90Degrees) {
    // Поворот точки (1, 0, 0) на 90° вокруг Z должен дать (0, 1, 0)
    s21::TransformMatrix rot = s21::TransformMatrixBuilder::CreateRotationMatrix(0.0f, 0.0f, 90.0f);
    s21::Point3D point(1.0f, 0.0f, 0.0f);

    s21::Point3D result = rot.TransformPoint(point);

    EXPECT_NEAR(result.x, 0.0f, kEpsilon);
    EXPECT_NEAR(result.y, 1.0f, kEpsilon);
    EXPECT_NEAR(result.z, 0.0f, kEpsilon);
}

TEST(TransformMatrixBuilderTest, CombinedTransformations) {
    // Сначала масштабирование, потом перемещение
    s21::TransformMatrix scale = s21::TransformMatrixBuilder::CreateScaleMatrix(2.0f, 2.0f, 2.0f);
    s21::TransformMatrix move = s21::TransformMatrixBuilder::CreateMoveMatrix(1.0f, 1.0f, 1.0f);

    // Комбинируем: сначала scale, потом move
    s21::TransformMatrix combined = move * scale;

    s21::Point3D point(1.0f, 1.0f, 1.0f);
    s21::Point3D result = combined.TransformPoint(point);

    // Ожидаем: (1*2+1, 1*2+1, 1*2+1) = (3, 3, 3)
    EXPECT_NEAR(result.x, 3.0f, kEpsilon);
    EXPECT_NEAR(result.y, 3.0f, kEpsilon);
    EXPECT_NEAR(result.z, 3.0f, kEpsilon);
}

TEST(TransformMatrixBuilderTest, ZeroScaleProtection) {
    s21::TransformMatrix scale = s21::TransformMatrixBuilder::CreateScaleMatrix(0.0f, 0.0f, 0.0f);

    // Должно вернуться к 1.0 для защиты
    EXPECT_NEAR(scale(0, 0), 1.0f, kEpsilon);
    EXPECT_NEAR(scale(1, 1), 1.0f, kEpsilon);
    EXPECT_NEAR(scale(2, 2), 1.0f, kEpsilon);
}

TEST(TransformMatrix, DeterminantTest) {
    s21::TransformMatrix default_matrix;
    s21::TransformMatrix transform_matrix2;
    s21::TransformMatrix transform_matrix3;

    std::vector<float> matrix2({
        0, 6, -2, -1,
        4, -2, 7, 0,
        5, -1, 2, -3,
        2, 3, 5, 8
    });
    std::vector<float> matrix3({
        1, 2, 3, 4,
        1, 2, 3, 4,
        0, 0, 0, 0,
        5, 6, 7, 8
    });

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            transform_matrix2(i, j) =  matrix2[i * 4 + j];
            transform_matrix3(i, j) =  matrix3[i * 4 + j];
        }
    }

    float det_default_matrix = default_matrix.det();
    float det_transform_matrix2 = transform_matrix2.det();
    float det_transform_matrix3 = transform_matrix3.det();

    EXPECT_FLOAT_EQ(det_default_matrix, 1.0f);
    EXPECT_NEAR(det_transform_matrix2, 1113.0f, kEpsilon);
    EXPECT_FLOAT_EQ(det_transform_matrix3, 0.0f);
}

TEST(TransformMatrix, TransposeTest) {
    s21::TransformMatrix default_matrix;
    s21::TransformMatrix transform_matrix2;
    s21::TransformMatrix transform_matrix3;

    std::vector<float> matrix2({
        0, 6, -2, -1,
        4, -2, 7, 0,
        5, -1, 2, -3,
        2, 3, 5, 8
    });
    std::vector<float> matrix3({
        1, 2, 3, 4,
        1, 2, 3, 4,
        0, 0, 0, 0,
        5, 6, 7, 8
    });

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            transform_matrix2(i, j) =  matrix2[i * 4 + j];
            transform_matrix3(i, j) =  matrix3[i * 4 + j];
        }
    }

    auto result = default_matrix.transpose();
    auto result2 = transform_matrix2.transpose();
    auto result3 = transform_matrix3.transpose();

    EXPECT_NEAR(result.det(), default_matrix.det(), kEpsilon);
    EXPECT_NEAR(result2.det(), transform_matrix2.det(), kEpsilon);
    EXPECT_NEAR(result3.det(), transform_matrix3.det(), kEpsilon);
}

TEST(TransformMatrix, InverseTest) {
    s21::TransformMatrix default_matrix;
    s21::TransformMatrix transform_matrix2;
    s21::TransformMatrix transform_matrix3;

    std::vector<float> matrix2({
        0, 6, -2, -1,
        4, -2, 7, 0,
        5, -1, 2, -3,
        2, 3, 5, 8
    });
    std::vector<float> matrix3({
        1, 2, 3.4, 4,
        1, 2, 3, 4,
        0.44, -0.4, 1.8, 0.1,
        5, 6, 7, 8
    });

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            transform_matrix2(i, j) =  matrix2[i * 4 + j];
            transform_matrix3(i, j) =  matrix3[i * 4 + j];
        }
    }

    auto result = default_matrix.inverse() * default_matrix;
    auto result2 = transform_matrix2.inverse() * transform_matrix2;
    auto result3 = transform_matrix3.inverse() * transform_matrix3;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(result(i, j), default_matrix(i, j), kEpsilon);
            EXPECT_NEAR(result2(i, j), default_matrix(i, j), kEpsilon);
            EXPECT_NEAR(result3(i, j), default_matrix(i, j), kEpsilon);
        }
    }
}