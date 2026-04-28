
template <typename T> requires std::derived_from<T, types::Type>
auto cast(Literal const &v) {
  if constexpr (std::is_same_v<T, types::IntegerType>) {
    return impl::cast<impl::Integer>(v);
  }
  else if constexpr (std::is_same_v<T, types::ArrayLike>) {
    return impl::cast<impl::ArrayLike>(v);
  }    
  else if constexpr (std::is_same_v<T, types::ArrayType>) {
    return impl::cast<impl::array>(v);
  }
  else if constexpr (std::is_same_v<T, types::StringType>) {
    return impl::cast<impl::string>(v);
  }
  else if constexpr (std::is_same_v<T, types::StructType>) {
    return impl::cast<impl::structT>(v);
  }
  else if constexpr (std::is_same_v<T, types::FunctionPointerType>) {
    return impl::cast<impl::FunctionPointer>(v);
  }
  else {
    static_assert(false, "invalid cast type");
  }
}
