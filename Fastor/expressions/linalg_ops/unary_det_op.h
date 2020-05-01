#ifndef UNARY_DET_OP_H
#define UNARY_DET_OP_H

#include "Fastor/meta/meta.h"
#include "Fastor/simd_vector/SIMDVector.h"
#include "Fastor/backend/determinant.h"
#include "Fastor/tensor/AbstractTensor.h"
#include "Fastor/tensor/TensorTraits.h"
#include "Fastor/expressions/expression_traits.h"
#include "Fastor/expressions/linalg_ops/linalg_computation_types.h"
#include "Fastor/expressions/linalg_ops/linalg_traits.h"
#include "Fastor/expressions/linalg_ops/unary_qr_op.h"

#include <cmath>

namespace Fastor {

// For tensors
template<DetCompType DetType = DetCompType::Simple, typename T, size_t M,
    enable_if_t_<is_less_equal_v_<M,4UL> && DetType==DetCompType::Simple,bool> = false>
FASTOR_INLINE T determinant(const Tensor<T,M,M> &a) {
    return _det<T,M,M>(static_cast<const T *>(a.data()));
}
template<DetCompType DetType = DetCompType::Simple, typename T, size_t M,
    enable_if_t_<is_greater_v_<M,4UL>    && DetType == DetCompType::Simple,bool> = false>
FASTOR_INLINE T determinant(const Tensor<T,M,M> &a) {
    static_assert(DetType==DetCompType::RREF, "DETERMINANT COMPUTATION USING THIS METHOD IS NOT IMPLEMENETED YET");
    return 0;
}
template<DetCompType DetType = DetCompType::Simple, typename T, size_t M,
    enable_if_t_<DetType == DetCompType::QR,bool> = false>
FASTOR_INLINE T determinant(const Tensor<T,M,M> &a) {
    Tensor<T,M,M> Q, R;
    std::tie(Q,R) = qr(a);
    return product(diag(R));
}
template<DetCompType DetType = DetCompType::Simple, typename T, size_t M,
    enable_if_t_<DetType != DetCompType::Simple && DetType != DetCompType::QR,bool> = false>
FASTOR_INLINE T determinant(const Tensor<T,M,M> &a) {
    static_assert(DetType==DetCompType::RREF, "DETERMINANT COMPUTATION USING THIS METHOD IS NOT IMPLEMENETED YET");
    return 0;
}

// For high order tensors
template<DetCompType DetType = DetCompType::Simple,
    typename T, size_t ... Rest, enable_if_t_<sizeof...(Rest)>=3 && DetType == DetCompType::Simple,bool> = false>
FASTOR_INLINE
typename LastMatrixExtracter<Tensor<T,Rest...>, typename std_ext::make_index_sequence<sizeof...(Rest)-2>::type>::type
determinant(const Tensor<T,Rest...> &a) {

    using OutTensor = typename LastMatrixExtracter<Tensor<T,Rest...>,
        typename std_ext::make_index_sequence<sizeof...(Rest)-2>::type>::type;
    constexpr size_t remaining_product = LastMatrixExtracter<Tensor<T,Rest...>,
        typename std_ext::make_index_sequence<sizeof...(Rest)-2>::type>::remaining_product;

    constexpr size_t I = get_value<sizeof...(Rest)-1,Rest...>::value;
    constexpr size_t J = get_value<sizeof...(Rest),Rest...>::value;
    static_assert(I==J,"THE LAST TWO DIMENSIONS OF TENSOR MUST BE THE SAME");

    OutTensor out;
    T *a_data = a.data();
    T *out_data = out.data();

    for (size_t i=0; i<remaining_product; ++i) {
        out_data[i] = _det<T,J,J>(static_cast<const T *>(a_data+i*J*J));
    }

    return out;
}

// For expressions - dispatches to determinant for tensors
template<DetCompType DetType = DetCompType::Simple, typename Derived, size_t DIMS>
FASTOR_INLINE typename Derived::scalar_type determinant(const AbstractTensor<Derived,DIMS> &_src) {
    const Derived &src = _src.self();
    using result_type = typename Derived::result_type;
    const result_type out = evaluate(src);
    return determinant<DetType>(out);
}


template<DetCompType DetType = DetCompType::Simple, typename Derived, size_t DIMS>
FASTOR_INLINE typename Derived::scalar_type det(const AbstractTensor<Derived,DIMS> &_src) {
    const Derived &src = _src.self();
    using result_type = typename Derived::result_type;
    const result_type out = evaluate(src);
    return determinant<DetType>(out);
}

template<DetCompType DetType = DetCompType::Simple, typename Derived, size_t DIMS>
FASTOR_INLINE typename Derived::scalar_type absdet(const AbstractTensor<Derived,DIMS> &_src) {
    const Derived &src = _src.self();
    using result_type = typename Derived::result_type;
    const result_type out = evaluate(src);
    return std::abs(determinant<DetType>(out));
}

template<DetCompType DetType = DetCompType::Simple, typename Derived, size_t DIMS>
FASTOR_INLINE typename Derived::scalar_type logdet(const AbstractTensor<Derived,DIMS> &_src) {
    const Derived &src = _src.self();
    using result_type = typename Derived::result_type;
    const result_type out = evaluate(src);
    return std::log(std::abs(determinant<DetType>(out)));
}


} // end of namespace Fastor

#endif // UNARY_DET_OP_H