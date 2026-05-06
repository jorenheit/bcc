
namespace acus::error {

  enum class ErrorCode {
    GenericApiRequirement,
    UnexpectedApiCall,
    EmptyProgram,
    DuplicateFunctionParameters,
    DuplicateGlobalReferences,
    FunctionDoesNotExist,
    LabelDoesNotExist,
    TakingAddressOfTemporary,
    ReadOnlyExpression,
    IncompatibleOperands,
    NotPrintable,
    EntryFunctionNotDefined,
    WrongEntryFunctionType,
    
    ExpectedOutsideProgramBlock,
    ExpectedInsideProgramBlock,
    ExpectedOutsideFunctionBlock,
    ExpectedInsideFunctionBlock,
    ExpectedNoScope,

    GlobalDeclarationNotAllowed,
    GlobalNameUnavailable,
    ExpectedGlobal,

    NameNotInScope,
    NameAlreadyInScope,
    NameAlreadyInCurrentScope,

    ExpectedInteger,
    ExpectedSignedInteger,
    ExpectedUnsignedInteger,
    ExpectedArray,
    ExpectedString,
    ExpectedArrayOrString,
    ExpectedStruct,
    ExpectedPointer,
    ExpectedFunctionType,
    ExpectedFunctionPointer,

    FieldIndexOutOfBounds,
    FieldCountMismatch,
    MissingField,
    MultipleInitializationsOfSameField,    
    FieldNotFound,
    IndexOutOfBounds,
    NegativeIndex,
    TooManyElementsInArrayInitialization,
    TooFewElementsInArrayInitialization,

    AssignmentTypeMismatch,
    UnexpectedType,
    FunctionParameterCountMismatch,
    InvalidFunctionPointerCall,    

    BuilderNotFinalized,
    ReturnTypeSpecifiedMultipleTimes    
  };
  
} // namespace acus::error
