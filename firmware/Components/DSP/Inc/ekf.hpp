#pragma once

#include "main.h"

#include "matrix.hpp"

// namespace PUTM
// {
//     template<size_t N, class TYPE = float>
//     class EKF
//     {
//     private:
//         Matrix<N, N, TYPE> A { };
//         Matrix<N, N, TYPE> B { };
//         Matrix<N, N, TYPE> C { };
//         Matrix<N, N, TYPE> Q { };
//         Matrix<N, N, TYPE> R { };
//     public:
//         EKF(const Matrix<N, N, TYPE> &A, const Matrix<N, N, TYPE> &B, const Matrix<N, N, TYPE> &C,
//             const Matrix<N, N, TYPE> &Q, const Matrix<N, N, TYPE> &R)
//             : A(A), B(B), C(C), Q(Q), R(R) { }
//         void predict()
//         {
//             x = x; // TODO: Implement the prediction step
//             P = P + Q;
//         }
//     };
// }