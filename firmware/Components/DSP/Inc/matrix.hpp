#pragma once

namespace PUTM
{
    /**
     *  @brief  A simple matrix class for 2D arrays.
     *  @tparam ROWS Number of rows in the matrix.
     *  @tparam COLS Number of columns in the matrix.
     *  @tparam TYPE Type of the elements in the matrix.
     *  @note   This class is designed to be used with fixed-size matrices.
     *          It does not support dynamic resizing or complex operations.
     */
    template<size_t ROWS, size_t COLS, class TYPE = float>
    requires (ROWS > 0 && COLS > 0) && std::is_arithmetic_v<TYPE> && (ROWS * COLS > 1)
    class Matrix
    {
    private:
        mutable TYPE data[ROWS * COLS] { 0 };
    public:
        using type = TYPE;
    public:
        /**
         *  @brief  Default constructor for the Matrix class.
         *  @note   Initializes all elements to zero.
         */
        Matrix() = default;

        /**
         *  @brief  Copy constructor for the Matrix class.
         *  @param  other The matrix to copy from.
         */
        constexpr Matrix(const Matrix& other)
        {
            for (size_t i = 0; i < ROWS * COLS; ++i)
            {
                data[i] = other.data[i];
            }
        }
    public:
        /**
         *  @brief  Constructor for the Matrix class.
         *  @param  arr A 2D array to initialize the matrix.
         *  @note   This constructor allows initializing the matrix with a
         *          predefined 2D array.
         */
        constexpr Matrix(TYPE const (&arr)[ROWS][COLS])
        {
            for (size_t i = 0; i < ROWS; ++i)
            {
                for (size_t j = 0; j < COLS; ++j)
                {
                    data[i * COLS + j] = arr[i][j];
                }
            }
        }
        
    public:
        /**
         *  @brief  Access an element in the matrix.
         *  @param  row The row index of the element.
         *  @param  col The column index of the element.
         *  @return A reference to the element at the specified position.
         *  @note   This function is not constexpr because it returns a reference
         */
        constexpr const TYPE& at(const size_t row, const size_t col) const
        {
            return data[row * COLS + col];
        }

        /**
         *  @brief  Access an element in the matrix.
         *  @param  row The row index of the element.
         *  @param  col The column index of the element.
         *  @return A reference to the element at the specified position.
         *  @note   This function is not constexpr because it returns a reference
         */
        constexpr TYPE& at(const size_t row, const size_t col)
        {
            return data[row * COLS + col];
        }

        /**
         *  @brief  Access an element in the matrix using a single index.
         *  @param  index The index of the element.
         *  @return A reference to the element at the specified position.
         *  @note   This function is not constexpr because it returns a reference
         */
        constexpr const TYPE& at(size_t index) const
        {
            return data[index];
        }

        /**
         *  @brief  Access an element in the matrix using a single index.
         *  @param  index The index of the element.
         *  @return A reference to the element at the specified position.
         *  @note   This function is not constexpr because it returns a reference
         */
        constexpr TYPE& at(size_t index)
        {
            return data[index];
        }

        /**
         *  @brief  Transpose the matrix.
         *  @return A new matrix that is the transpose of the original matrix.
         *  @note   This function creates a new matrix with the rows and columns swapped.
         */
        constexpr Matrix<COLS, ROWS, TYPE> T() const
        {
            Matrix<COLS, ROWS, TYPE> result;
            for (size_t i = 0; i < ROWS; ++i)
            {
                for (size_t j = 0; j < COLS; ++j)
                {
                    result.at(j, i) = data[i * COLS + j];
                }
            }
            return result;
        }

        /**
         *  @brief  Access an element in the matrix using a single index.
         *  @param  index The index of the element.
         *  @return A reference to the element at the specified position.
         *  @note   This function is not constexpr because it returns a reference
         */
        Matrix& operator=(const Matrix& other)
        {
            for (size_t i = 0; i < ROWS; ++i)
            {
                for (size_t j = 0; j < COLS; ++j)
                {
                    data[i * COLS + j] = other.data[i * COLS + j];
                }
            }

            return *this;
        }
    public:
        /**
         *  @brief  Get the number of rows in the matrix.
         *  @return The number of rows in the matrix.
         */
        constexpr size_t rows() const
        {
            return ROWS;
        }
    public:
        /**
         *  @brief  Get the number of columns in the matrix.
         *  @return The number of columns in the matrix.
         */
        constexpr size_t cols() const
        {
            return COLS;
        }

        /**
         *  @brief  Get the number of elements in the matrix.
         *  @return The number of elements in the matrix as a pair of number
         *          of rows and colmuns.
         */
        constexpr std::pair<size_t, size_t> size() const
        {
            return { ROWS, COLS };
        }
    };

    /**
     *  @brief  Operator + to add a scalar to a matrix.
     *  @tparam MATRIX The matrix type.
     *  @param  mat The matrix to be added.
     *  @param  scalar The scalar value to be added.
     *  @return A new matrix with the result of the addition.
     *  @note   This operator allows adding a scalar to each element of the matrix.
     */
    template<size_t ROWS, size_t COLS, typename TYPE>
    constexpr Matrix<ROWS, COLS, TYPE> operator + (const Matrix<ROWS, COLS, TYPE> &mat, TYPE &scalar)
    {
        Matrix<ROWS, COLS, TYPE> result;
        for (size_t i = 0; i < mat.rows(); ++i)
        {
            for (size_t j = 0; j < mat.cols(); ++j)
            {
                result.at(i, j) = mat.at(i, j) + scalar;
            }
        }
        return result;
    }

    /**
     *  @brief  Operator + to add two matrices.
     *  @tparam MATRIX_LEFT The left matrix type.
     *  @tparam MATRIX_RIGHT The right matrix type.
     *  @param  mat_left The left matrix to be added.
     *  @param  mat_right The right matrix to be added.
     *  @return A new matrix with the result of the addition.
     *  @note   This operator allows adding two matrices of the same size.
     */
    template<size_t ROWS, size_t COLS, typename TYPE>
    constexpr Matrix<ROWS, COLS, TYPE> operator + (const Matrix<ROWS, COLS, TYPE> &mat_left, const Matrix<ROWS, COLS, TYPE> &mat_right)
    {
        Matrix<ROWS, COLS, TYPE> result;
        for (size_t i = 0; i < mat_left.rows(); ++i)
        {
            for (size_t j = 0; j < mat_left.cols(); ++j)
            {
                result.at(i, j) = mat_left.at(i, j) + mat_right.at(i, j);
            }
        }
        return result;
    }

    /**
     *  @brief  Operator - to substruct a scalar to a matrix.
     *  @tparam MATRIX The matrix type.
     *  @param  mat The matrix to be substructed.
     *  @param  scalar The scalar value to be substructed.
     *  @return A new matrix with the result of the substruction.
     *  @note   This operator allows substracting a scalar from each element of the matrix.
     */
    template<size_t ROWS, size_t COLS, typename TYPE>
    constexpr Matrix<ROWS, COLS, TYPE> operator - (const Matrix<ROWS, COLS, TYPE> &mat, TYPE &scalar)
    {
        Matrix<ROWS, COLS, TYPE> result;
        for (size_t i = 0; i < mat.rows(); ++i)
        {
            for (size_t j = 0; j < mat.cols(); ++j)
            {
                result.at(i, j) = mat.at(i, j) - scalar;
            }
        }
        return result;
    }

    /**
     *  @brief  Operator - to substruct two matrices.
     *  @tparam MATRIX_LEFT The left matrix type.
     *  @tparam MATRIX_RIGHT The right matrix type.
     *  @param  mat_left The left matrix to be substructed.
     *  @param  mat_right The right matrix to be substructed.
     *  @return A new matrix with the result of the substruction.
     *  @note   This operator allows substracting two matrices of the same size.
     */
    template<size_t ROWS, size_t COLS, typename TYPE>
    constexpr Matrix<ROWS, COLS, TYPE> operator - (const Matrix<ROWS, COLS, TYPE> &mat_left, const Matrix<ROWS, COLS, TYPE> &mat_right)
    {
        Matrix<ROWS, COLS, TYPE> result;
        for (size_t i = 0; i < mat_left.rows(); ++i)
        {
            for (size_t j = 0; j < mat_left.cols(); ++j)
            {
                result.at(i, j) = mat_left.at(i, j) - mat_right.at(i, j);
            }
        }
        return result;
    }

    /**
     *  @brief  Operator * to multiply a matrix by a scalar.
     *  @tparam MATRIX The matrix type.
     *  @param  mat The matrix to be multiplied.
     *  @param  scalar The scalar value to multiply with.
     *  @return A new matrix with the result of the multiplication.
     *  @note   This operator allows multiplying each element of the matrix by a scalar.
     */
    template<size_t ROWS, size_t COLS, typename TYPE>
    constexpr Matrix<ROWS, COLS, TYPE> operator * (const Matrix<ROWS, COLS, TYPE> mat, TYPE scalar)
    {
        Matrix<ROWS, COLS, TYPE> result;
        for (size_t i = 0; i < mat.rows(); ++i)
        {
            for (size_t j = 0; j < mat.cols(); ++j)
            {
                result.at(i, j) = mat.at(i, j) * scalar;
            }
        }
        return result;
    }

    /**
     *  @brief  Operator * to multiply two matrices.
     *  @tparam MATRIX_LEFT  The left matrix type.
     *  @tparam MATRIX_RIGHT The right matrix type.
     *  @param  mat_left The left matrix to be multiplied.
     *  @param  mat_right The right matrix to be multiplied.
     *  @return A new matrix with the result of the multiplication.
     *  @note   This operator allows multiplying two matrices if the number of columns
     *          in the left matrix is equal to the number of rows in the right matrix.
     */
    template
    <
        size_t RL, size_t CL,
        size_t RR, size_t CR,
        typename TYPE
    >
    requires (CL == RR && (RL * CR > 1)) 
    constexpr Matrix<RL, CR, TYPE> operator * (const Matrix<RL, CL, TYPE> &mat_left, const Matrix<RR, CR, TYPE> &mat_right)
    {
        Matrix<RL, CR, TYPE> result;
        for (size_t i = 0; i < mat_left.rows(); ++i)
        {
            for (size_t j = 0; j < mat_right.cols(); ++j)
            {
                result.at(i, j) = 0;
                for (size_t k = 0; k < mat_left.cols(); ++k)
                {
                    result.at(i, j) += mat_left.at(i, k) * mat_right.at(k, j);
                }
            }
        }
        return result;
    }

    template
    <
        size_t RL,
        size_t CR,
        typename TYPE
    >
    requires (RL == CR)
    // TYPE operator * (const Matrix<1, RL, TYPE> &mat_left, const Matrix<CR, 1, TYPE> &mat_right)
    constexpr TYPE dot(const Matrix<1, RL, TYPE> &mat_left, const Matrix<CR, 1, TYPE> &mat_right)
    {
        TYPE result { };
        for (size_t i = 0; i < mat_left.rows(); ++i)
        {
            for (size_t j = 0; j < mat_right.cols(); ++j)
            {
                result += mat_left.at(i, j) * mat_right.at(i, j);
            }
        }
        return result;
    }

    /**
     *  @brief  Operator / to divide a matrix by a scalar.
     *  @tparam MATRIX The matrix type.
     *  @param  mat The matrix to be divided.
     *  @param  scalar The scalar value to divide with.
     *  @return A new matrix with the result of the division.
     *  @note   This operator allows dividing each element of the matrix by a scalar.
     */
    template<size_t ROWS, size_t COLS, typename TYPE>
    constexpr Matrix<ROWS, COLS, TYPE> operator / (const Matrix<ROWS, COLS, TYPE> &mat, TYPE scalar)
    {
        Matrix<ROWS, COLS, TYPE> result;
        for (size_t i = 0; i < mat.rows(); ++i)
        {
            for (size_t j = 0; j < mat.cols(); ++j)
            {
                result[i, j] = mat[i, j] / scalar;
            }
        }
        return result;
    }    

    template<size_t ROWS, size_t COLS, typename TYPE>
    constexpr Matrix<COLS, ROWS, TYPE> transpose(const Matrix<ROWS, COLS, TYPE> &mat)
    {
        Matrix<COLS, ROWS, TYPE> result;
        for (size_t i = 0; i < mat.rows(); ++i)
        {
            for (size_t j = 0; j < mat.cols(); ++j)
            {
                result.at(j, i) = mat.at(i, j);
            }
        }
        return result;
    }

    // template
    // <
    //     size_t CL,
    //     size_t RR,
    //     typename TYPE
    // >
    // requires (CL == RR)
    // float dot(const Matrix<1, CL, TYPE> &mat_left, const Matrix<RR, 1, TYPE> &mat_right)
    // {
    //     float result = 0.f;
    //     for (size_t i = 0; i < mat_left.rows(); ++i)
    //     {
    //         for (size_t j = 0; j < mat_right.cols(); ++j)
    //         {
    //             result += mat_left.at(i, j) * mat_right.at(i, j);
    //         }
    //     }
    //     return result;
    // }

    /**
     *  @brief  Create an identity matrix of size N x N.
     *  @tparam N The size of the identity matrix.
     *  @tparam TYPE The type of the elements in the matrix.
     *  @return A new identity matrix of size N x N.
     *  @note   This function is consteval, allowing it to be evaluated at compile time.
     */
    template<size_t N, class TYPE = float>
    constexpr Matrix<N, N, TYPE> eye()
    {
        Matrix<N, N, TYPE> result;
        for (size_t i = 0; i < result.rows(); ++i)
        {
            for (size_t j = 0; j < result.cols(); ++j)
            {
                result.at(i, j) = (i == j) ? 1 : 0;
            }
        }
        return result;
    }
}