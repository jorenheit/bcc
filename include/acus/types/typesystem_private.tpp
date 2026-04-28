
namespace acus::ts::impl {

  class StructType {
  public:
    types::StructType const *operator()(auto const&... args) && {
      static_assert(sizeof ... (args) % 2 == 0);
      std::vector<types::NameTypePair> fields;

      auto addField = [&]<typename ... Rest>(auto&& self, std::string const &name, types::TypeHandle type, Rest&& ... rest) -> void {
	fields.push_back(types::NameTypePair{name, type});
	if constexpr (sizeof ... (Rest) == 0) return;
	else self(self, std::forward<Rest>(rest)...);
      };

      addField(addField, std::forward<decltype(args)>(args)...);
      return std::move(*this)(fields);
    }
      
    types::StructType const *operator()(std::vector<types::NameTypePair> const &fields) &&;
    StructType(std::string const &structName, api::impl::Context const &ctx);
    ~StructType() noexcept(false);
    
  private:
    std::string _structName;
    api::impl::Context API_CTX_NAME;
    bool _called = false;
  
    StructType(StructType const&) = delete;
    StructType(StructType&&) = delete;
    StructType& operator=(StructType const&) = delete;
    StructType& operator=(StructType&&) = delete;
  };


  class FunctionType {
  public:
    types::FunctionType const *operator()(auto const&... args) && {
      std::vector<types::TypeHandle> paramTypes = {args ...};
      return std::move(*this)(paramTypes);
    }
      
    types::FunctionType const *operator()(std::vector<types::TypeHandle> const &paramTypes) &&;
    FunctionType(types::TypeHandle ret, api::impl::Context const &ctx);
    ~FunctionType() noexcept(false);
    
  private:
    types::TypeHandle _ret;
    api::impl::Context API_CTX_NAME;
    bool _called = false;
  
    FunctionType(FunctionType const&) = delete;
    FunctionType(FunctionType&&) = delete;
    FunctionType& operator=(FunctionType const&) = delete;
    FunctionType& operator=(FunctionType&&) = delete;
  };  

} // namespace impl
  
