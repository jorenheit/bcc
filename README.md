# Acus

**Acus** is a C++ backend toolkit for generating Brainfuck programs from a structured builder API. It provides a small, explicit program-construction layer with functions, blocks, scopes, globals, locals, structs, arrays, pointers, arithmetic, comparisons, logical operations, and control flow. The intended use case is to build higher-level frontends on top of Acus, while still keeping the backend usable directly from C++ for experiments, tests, and low-level code generation.

The central API type is currently planned as:

```cpp
acus::ProgramBuilder builder;
```

The current source still uses the temporary name `Compiler`; this README uses the intended public name `ProgramBuilder`.


## Small example

The following sketch builds a tiny program whose `main` function calls a separate `hello` function. The `hello` function writes `Hello, world!` and then returns to `main`.

```cpp
acus::ProgramBuilder b;

b.begin();

b.setEntryPoint("main");
  
  b.beginFunction("main"); {
   
    b.beginBlock("entry"); {
       b.callFunction("hello", "after_hello")();
    } b.endBlock();
   
    b.beginBlock("after_hello"); {
       b.returnFromFunction();
    } b.endBlock();
  
  } bf.endFunction();
  
  b.beginFunction("hello"); {
  
    b.beginBlock("entry"); {
      b.writeOut(values::string("Hello, World!"));
      b.returnFromFunction();
    b.endBlock();
    
  } bf.endFunction();

bf.end();
```

Most public functions carry an internal source-location/context argument used for diagnostics. That argument is omitted from the signatures below.

---

## Public API overview

### Construction and output

| Function | Returns | Description |
|---|---:|---|
| `ProgramBuilder()` | `ProgramBuilder` | Creates a new builder and initializes the type system. |
| `dumpPrimitives()` | `std::string` | Returns a textual dump of the generated primitive instruction sequence. |
| `dumpBrainfuck()` | `std::string` | Emits the final Brainfuck program. |

### Program, function, block, and scope structure

| Function | Returns | Description |
|---|---:|---|
| `setEntryPoint(functionName)` | `void` | Selects the function used as the program entry point. |
| `begin()` | `void` | Starts program construction. |
| `end()` | `void` | Finishes program construction and performs final validation/post-processing. |
| `beginScope()` | `void` | Opens a nested lexical scope. |
| `endScope()` | `void` | Closes the current lexical scope. |
| `beginBlock(name)` | `void` | Starts a named basic block in the current function. |
| `endBlock()` | `void` | Ends the current block. |
| `beginFunction(name)` | `void` | Starts a function with an inferred/default function type. |
| `beginFunction(name, funcType)` | `void` | Starts a function with an explicit function type. |
| `beginFunction(name, funcType, params)` | `void` | Starts a function with an explicit function type and parameter names. |
| `endFunction()` | `void` | Ends the current function. |

### Declarations and name visibility

| Function | Returns | Description |
|---|---:|---|
| `declareLocal(name, type)` | `Slot` | Declares a local variable in the current scope. |
| `declareGlobal(name, type)` | `Slot` | Declares a global variable. |
| `referGlobals(names)` | `void` | Makes selected globals available inside the current function. |
| `defineStruct(name)(fields...)` | `types::TypeHandle` | Defines a struct type using the returned `StructDefinition` builder. Example: `defineStruct("Point")("x", i8, "y", i8)`. |

### Function calls and returns

| Function | Returns | Description |
|---|---:|---|
| `callFunction(functionName, nextBlockName)(args...)` | `void` | Calls a function and continues at `nextBlockName`. The returned `FunctionCall` object must be invoked. |
| `callFunction(functionName, nextBlockName, returnSlot)(args...)` | `void` | Calls a function, stores its return value in `returnSlot`, and continues at `nextBlockName`. |
| `returnFromFunction()` | `void` | Returns from the current function without a value. |
| `returnFromFunction(ret)` | `void` | Returns from the current function with a value. |
| `abortProgram()` | `void` | Emits an abort/halt path for the program. |

### Control flow

| Function | Returns | Description |
|---|---:|---|
| `setNextBlock(block)` | `void` | Sets the next block within the current function. |
| `setNextBlock(function, block)` | `void` | Sets the next block using an explicit function/block pair. |
| `branchIf(condition, trueLabel, falseLabel)` | `void` | Branches to one of two labels based on a runtime condition. |

### Assignment, output, and access expressions

| Function | Returns | Description |
|---|---:|---|
| `assign(lhs, rhs)` | `ExpressionResult` | Assigns `rhs` into `lhs`. |
| `writeOut(val)` | `void` | Emits code to write a value to output. |
| `structField(obj, fieldName)` | `ExpressionResult` | Produces an expression referring to a named struct field. |
| `structField(obj, fieldIndex)` | `ExpressionResult` | Produces an expression referring to a struct field by index. |
| `arrayElement(arr, index)` | `ExpressionResult` | Produces an expression referring to an array element. Supports both constant and runtime indices. |
| `dereferencePointer(ptr)` | `ExpressionResult` | Produces an expression for the pointee of a runtime pointer. |
| `addressOf(obj)` | `ExpressionResult` | Produces a pointer expression referring to `obj`. |

### Arithmetic operations

| Function | Returns | Description |
|---|---:|---|
| `add(lhs, rhs)` | `ExpressionResult` | Computes `lhs + rhs` into a temporary result. |
| `sub(lhs, rhs)` | `ExpressionResult` | Computes `lhs - rhs` into a temporary result. |
| `mul(lhs, rhs)` | `ExpressionResult` | Computes `lhs * rhs` into a temporary result. |
| `div(lhs, rhs)` | `ExpressionResult` | Computes `lhs / rhs` into a temporary result. |
| `mod(lhs, rhs)` | `ExpressionResult` | Computes `lhs % rhs` into a temporary result. |
| `addAssign(lhs, rhs)` | `ExpressionResult` | Applies `lhs += rhs`. |
| `subAssign(lhs, rhs)` | `ExpressionResult` | Applies `lhs -= rhs`. |
| `mulAssign(lhs, rhs)` | `ExpressionResult` | Applies `lhs *= rhs`. |
| `divAssign(lhs, rhs)` | `ExpressionResult` | Applies `lhs /= rhs`. |
| `modAssign(lhs, rhs)` | `ExpressionResult` | Applies `lhs %= rhs`. |

### Logical operations

| Function | Returns | Description |
|---|---:|---|
| `lnot(rhs)` | `ExpressionResult` | Computes logical NOT into a temporary result. Planned/under implementation. |
| `lnotAssign(rhs)` | `ExpressionResult` | Applies logical NOT in-place. Planned/under implementation. |
| `land(lhs, rhs)` | `ExpressionResult` | Computes logical AND into a temporary result. |
| `lnand(lhs, rhs)` | `ExpressionResult` | Computes logical NAND into a temporary result. |
| `lor(lhs, rhs)` | `ExpressionResult` | Computes logical OR into a temporary result. |
| `lnor(lhs, rhs)` | `ExpressionResult` | Computes logical NOR into a temporary result. |
| `lxor(lhs, rhs)` | `ExpressionResult` | Computes logical XOR into a temporary result. |
| `lxnor(lhs, rhs)` | `ExpressionResult` | Computes logical XNOR into a temporary result. |
| `landAssign(lhs, rhs)` | `ExpressionResult` | Applies logical AND in-place. |
| `lnandAssign(lhs, rhs)` | `ExpressionResult` | Applies logical NAND in-place. |
| `lorAssign(lhs, rhs)` | `ExpressionResult` | Applies logical OR in-place. |
| `lnorAssign(lhs, rhs)` | `ExpressionResult` | Applies logical NOR in-place. |
| `lxorAssign(lhs, rhs)` | `ExpressionResult` | Applies logical XOR in-place. |
| `lxnorAssign(lhs, rhs)` | `ExpressionResult` | Applies logical XNOR in-place. |

### Comparison operations

| Function | Returns | Description |
|---|---:|---|
| `eq(lhs, rhs)` | `ExpressionResult` | Computes `lhs == rhs` into a temporary boolean result. |
| `neq(lhs, rhs)` | `ExpressionResult` | Computes `lhs != rhs` into a temporary boolean result. |
| `lt(lhs, rhs)` | `ExpressionResult` | Computes `lhs < rhs` into a temporary boolean result. |
| `le(lhs, rhs)` | `ExpressionResult` | Computes `lhs <= rhs` into a temporary boolean result. |
| `gt(lhs, rhs)` | `ExpressionResult` | Computes `lhs > rhs` into a temporary boolean result. |
| `ge(lhs, rhs)` | `ExpressionResult` | Computes `lhs >= rhs` into a temporary boolean result. |
| `eqAssign(lhs, rhs)` | `ExpressionResult` | Applies equality comparison in-place. |
| `neqAssign(lhs, rhs)` | `ExpressionResult` | Applies inequality comparison in-place. |
| `ltAssign(lhs, rhs)` | `ExpressionResult` | Applies less-than comparison in-place. |
| `leAssign(lhs, rhs)` | `ExpressionResult` | Applies less-or-equal comparison in-place. |
| `gtAssign(lhs, rhs)` | `ExpressionResult` | Applies greater-than comparison in-place. |
| `geAssign(lhs, rhs)` | `ExpressionResult` | Applies greater-or-equal comparison in-place. |

---

## Builder-style calls

Some operations return short-lived builder objects. These objects are marked `[[nodiscard]]` and must be called immediately:

```cpp
builder.callFunction("foo", "after_foo")(arg1, arg2);

builder.defineStruct("Point")(
  "x", i8,
  "y", i8
);
```

This keeps the public API variadic without losing accurate source-location diagnostics.

---

## Current status

Acus is still under active development. The backend already supports a substantial set of atomic operations for a future frontend, but names and structure may still change before the public API is stabilized.
