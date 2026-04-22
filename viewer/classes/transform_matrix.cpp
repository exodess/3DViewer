#include <viewer/viewerSpec.h>
#include <cmath>

namespace s21 {

// =====================================
// ========== TransformMatrix ==========
// =====================================

TransformMatrix::TransformMatrix() noexcept {
    // Инициализация единичной матрицей (Identity Matrix)
    // | 1  0  0  0 |
    // | 0  1  0  0 |
    // | 0  0  1  0 |
    // | 0  0  0  1 |
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            mtrx_[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

TransformMatrix::TransformMatrix(const TransformMatrix& other) noexcept {
    // Копирование матрицы
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            mtrx_[i][j] = other.mtrx_[i][j];
        }
    }
}

TransformMatrix& TransformMatrix::operator=(const TransformMatrix& other) noexcept {
    // Оператор присваивания
    if (this != &other) {
        for (size_t i = 0; i < 4; ++i) {
            for (size_t j = 0; j < 4; ++j) {
                mtrx_[i][j] = other.mtrx_[i][j];
            }
        }
    }
    return *this;
}

TransformMatrix TransformMatrix::operator*(const TransformMatrix& other) const noexcept {
    // Умножение матриц (матричное произведение)
    // C[i][j] = Σ A[i][k] * B[k][j]
    TransformMatrix result;
    
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            result.mtrx_[i][j] = 0.0f;
            for (size_t k = 0; k < 4; ++k) {
                result.mtrx_[i][j] += mtrx_[i][k] * other.mtrx_[k][j];
            }
        }
    }
    
    return result;
}

Point3D TransformMatrix::TransformPoint(const Point3D& point) const noexcept {
    // Трансформация точки с использованием однородных координат
    // [x']   [m00 m01 m02 m03]   [x]
    // [y']   [m10 m11 m12 m13]   [y]
    // [z'] = [m20 m21 m22 m23] * [z]
    // [w']   [m30 m31 m32 m33]   [1]
    
    const float w = 1.0f;  // Однородная координата
    
    float result_x = mtrx_[0][0] * point.x + 
                     mtrx_[0][1] * point.y + 
                     mtrx_[0][2] * point.z + 
                     mtrx_[0][3] * w;
    
    float result_y = mtrx_[1][0] * point.x + 
                     mtrx_[1][1] * point.y + 
                     mtrx_[1][2] * point.z + 
                     mtrx_[1][3] * w;
    
    float result_z = mtrx_[2][0] * point.x + 
                     mtrx_[2][1] * point.y + 
                     mtrx_[2][2] * point.z + 
                     mtrx_[2][3] * w;
    
    // Перспективное деление (для перспективной проекции)
    float result_w = mtrx_[3][0] * point.x + 
                     mtrx_[3][1] * point.y + 
                     mtrx_[3][2] * point.z + 
                     mtrx_[3][3] * w;
    
    if (std::abs(result_w) > 1e-6f) {
        result_x /= result_w;
        result_y /= result_w;
        result_z /= result_w;
    }
    
    return Point3D(result_x, result_y, result_z);
}

float TransformMatrix::GetElement(size_t row, size_t col) const noexcept {
    // Получение элемента матрицы
    if (row < 4 && col < 4) {
        return mtrx_[row][col];
    }
    return 0.0f;
}

void TransformMatrix::SetElement(size_t row, size_t col, float value) noexcept {
    // Установка элемента матрицы
    if (row < 4 && col < 4) {
        mtrx_[row][col] = value;
    }
}

TransformMatrix TransformMatrix::Identity() noexcept {
    // Возврат единичной матрицы
    return TransformMatrix();
}

float TransformMatrix::calc_minor(int i, int j) const noexcept {
    float sub_det = 0.0; // определитель нижней подматрицы
    float temp[3][3];

    // копируем подматрицу в другой массив
    int ki = 0;
    for (int a = 0; a < 3; ++a) {
        int kj = 0;
        if (i != ki) {
            for (int b = 0; b < 3; ++b) {
                if (kj != i)
                    temp[a][b] = mtrx_[ki][kj];
                kj++;
            }
        };

        ki ++;
    }

    for (int a = 0; i < 3; ++i) {
        float temp_temp[2][2];

        // копируем матрицу и находим определитель
        for (int aa = 0; aa < 2; ++aa) {
            int kk = 0;
            for (int bb = 0; bb < 2; ++bb) {
                if (kk != j)
                    temp_temp[aa][bb] = temp[aa + 1][kk];
                kk++;
            }
        }
        float simple_det = temp_temp[0][0] * temp_temp[1][1] - temp_temp[0][1] * temp_temp[1][0];

        sub_det += (j % 2 == 0) ? simple_det * temp[0][j] : -simple_det * temp[0][j];
    }

    return ((i + j) % 2 == 0) ? sub_det : -sub_det;
}


float TransformMatrix::det() const noexcept {
    float det = 0.0;

    for (int j = 0; j < 4; ++j) {
        det += mtrx_[0][j] * calc_minor(0, j);
    }

    return det;
}

TransformMatrix TransformMatrix::calcComplements() const noexcept {
    TransformMatrix result;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.mtrx_[i][j] = calc_minor(i, j);
        }
    }

    return result;
}


TransformMatrix TransformMatrix::inverse() const noexcept {
    TransformMatrix result;

    float d = det();
    if (d != 0.0) {
        result = calcComplements().transpose();

        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                result.mtrx_[i][j] /= d;

    }
    return result;
}

TransformMatrix TransformMatrix::transpose() const noexcept {
    TransformMatrix result;

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result.mtrx_[i][j] = mtrx_[j][i];

    return result;
}

} // namespace s21
