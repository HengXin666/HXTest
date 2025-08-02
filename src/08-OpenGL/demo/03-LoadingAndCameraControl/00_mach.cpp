#include <HXTest.hpp>

// 4D 向量
struct Vec4 {
    float x, y, z, w;

    float operator[](std::size_t i) const {
        switch (i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            case 3: return w;
            [[unlikely]] default: throw std::runtime_error{"sb"};
        }
    }

    // 构造函数、+,-,*,/= 等你自己补齐
};

// 4x4 矩阵, 按列优先(兼容 OpenGL 风格)
struct Mat4 {
    inline constexpr static std::size_t Row = 4; // 行
    inline constexpr static std::size_t Col = 4; // 列
    float data[Col][Row]{}; // data[col][row]

    auto& operator[](std::size_t i) const {
        return data[i];
    }

    auto& operator[](std::size_t i)  {
        return data[i];
    }

    /**
     * @brief 返回单位矩阵, 即主对角线是 1, 其余是 0。
     * @return Mat4
     */
    static Mat4 identity() {
        static_assert(Row == Col, "");
        Mat4 res;
        for (std::size_t i = 0; i < Col; ++i)
            res[i][i] = 1;
        return res;
    }

    /**
     * @brief 构造平移矩阵
            | 1 0 0 tx |
            | 0 1 0 ty |
            | 0 0 1 tz |
            | 0 0 0 1  |
     * @param offset 是一个 (x, y, z, *), 其中 w 可忽略
     * @return Mat4
     */
    static Mat4 translate(const Vec4& offset) {
        auto res = identity();
        res[Col - 1][0] = offset.x;
        res[Col - 1][1] = offset.y;
        res[Col - 1][2] = offset.z;
        // res[Col - 1][3] = 1;
        return res;
    }

    /**
     * @brief 构造缩放矩阵
            | sx 0  0  0 |
            | 0  sy 0  0 |
            | 0  0  sz 0 |
            | 0  0  0  1 |
     * @param scale 缩放向量
     * @return Mat4
     */
    static Mat4 scale(const Vec4& scale) {
        Mat4 res;
        res[0][0] = scale.x;
        res[1][1] = scale.y;
        res[2][2] = scale.z;
        res[3][3] = 1;
        return res;
    }


    static Mat4 rotate(float radians, const Vec4& axis); // 绕轴旋转

    /**
     * @brief 矩阵 × 向量
     * @param v
     * @return Vec4
     */
    Vec4 multiplyVec4(const Vec4& v) const {
        // [i]行 * 列(向量)
        return {
            data[0][0] * v.x + data[1][0] * v.y + data[2][0] * v.z + data[3][0] * v.w,
            data[0][1] * v.x + data[1][1] * v.y + data[2][1] * v.z + data[3][1] * v.w,
            data[0][2] * v.x + data[1][2] * v.y + data[2][2] * v.z + data[3][2] * v.w,
            data[0][3] * v.x + data[1][3] * v.y + data[2][3] * v.z + data[3][3] * v.w,
        };
    }

    /**
     * @brief 矩阵 × 矩阵
     * @param other
     * @return Mat4
     */
    Mat4 multiplyMat4(const Mat4& other) const {
        Mat4 res;
        for (std::size_t i = 0; i < Col; ++i) {
            for (std::size_t j = 0; j < Row; ++j) {
                for (std::size_t k = 0; k < Col; ++k) {
                    res[i][j] += data[k][j] * other.data[i][k];
                }
            }
        }
        return res;
    }

    /**
     * @brief 转置, 交换 data[i][j] 和 data[j][i]
     * @return Mat4
     */
    Mat4 transpose() const {
        Mat4 res;
        for (std::size_t i = 0; i < Col; ++i) {
            for (std::size_t j = 0; j < Row; ++j) {
                res[i][j] = data[j][i];
            }
        }
        return res;
    }
    Mat4 inverse() const;                      // 逆矩阵(提示: 只做仿射矩阵逆)
};


int main() {
    auto T = Mat4::translate({1, 2, 3, 0});
    auto v = Vec4{4, 5, 6, 1};
    auto result = T.multiplyVec4(v);
    // 应该是 {5, 7, 9, 1}
    log::hxLog.info(result);
    return 0;
}