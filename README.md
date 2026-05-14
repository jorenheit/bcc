# Acus

Acus is a C++23 backend toolkit for generating Brainfuck programs through an API that resembles a simple programming language. It lets you construct a program in C++ using functions, scopes, local and global variables, arrays, structs, pointers, function pointers, arithmetic, comparisons, logical operations, labels, branches, and returns. Acus then lowers that structured program to Brainfuck.

The intended use case is to build higher-level frontends on top of Acus while still keeping the backend usable directly from C++ for experiments, tests, and low-level code generation.

## Minimal example: Hello, World!

The program below features a simple Hello World example and outlines the way an Acus program must be structured:

```cpp
#include <iostream>
#include <acus/acus.h>

using namespace acus::api;

int main() try {
  Assembler a;

  a.program("hello", "main").begin(); {

    a.function("main").begin(); {
      a.print(literal::string("Hello, World!\n"));
      a.returnFromFunction();
    } a.endFunction();

  } a.endProgram();

  std::cout << a.brainfuck("hello") << '\n';
}
catch (Error const &e) {
  std::cerr << e.what() << " (" << e.code() << ")\n";
}
```

This constructs a program named `hello`, defines `main` as its entry function that simply prints  `Hello, World!`. After defining the BF-program, the generated Brainfuck source is printed to `stdout`. More involved examples are included below.

## Building

Build instructions are intentionally left as a placeholder for now.

```text
TODO: add supported build and install instructions before publication.
```

Recommended topics to cover here before the first public release:

- required C++ standard,
- supported compilers,
- building the library,
- building examples,
- running tests,
- installing headers/library files,
- using Acus from another CMake or Make project.

## Public include files

To use the library, you need to include the main Acus header: `acus.h`. This header will include all necessary headers for you and expose the `acus::api` namespace that you can import for ease of use.

```cpp
#include <acus/acus.h>
using namespace acus::api;
```
The table below shows the classes and namespaces available after `acus.h` has been included (full path) and, optionally, when the `acus::api` namespace is imported (short path):

| Full path                | Short path         | type       | Description                                         |
|--------------------------|--------------------|------------|-----------------------------------------------------|
| `acus::Assembler`        | `Assembler`        | class      | The main assembler class used to generate BF.       |
| `acus::literal`          | `literal`          | namespace  | Namespace for literal-generation.                   |
| `acus::literal::Literal` | `Literal`          | class      | Literal object, representing a compile-time value.  |
| `acus::ts`               | `ts`               | namespace  | Typesystem namespace, used to request type-handles. |
| `acus::ts::TypeHandle`   | `TypeHandle`       | class      | Object representing a type.                         |
| `acus::error::Error`     | `error::Error`     | class      | Error object, thrown when an error occurs.          |
| `acus::error::ErrorCode` | `error::ErrorCode` | enum class | Error code embedded in the Error object.            |

### Acus Programming
## `Assembler`

`acus::Assembler` is the main public entry point. It owns the program under construction and exposes methods for declaring data, defining functions, creating expressions, emitting control flow, and generating Brainfuck.

A typical program has this shape:

```cpp
Assembler a;

a.program("program_name", "main").begin(); {
  // global variable declarations

  a.function("main").begin(); {
    // Function body
    a.returnFromFunction();
  } a.endFunction();
  
  // Other functions
  
} a.endProgram();

std::string bf = c.brainfuck("program_name");
```

The usage of C++ scope blocks is mainly for convenience (the C++ scope matches the BF scope and indentation is handled automatically by your editor) but not necessary for creating valid programs.

### Functions
#### Defining a Function

A function is defined using `Assembler::function`, which returns a `FunctionBuilder` object. This object is not meant to be stored in a variable; instead you call the `param` and `ret` methods on it until the entire function signature has been completed, at which point you call `begin` to start the body of the function:

```cpp
a.function("foo")
  .param("x", ts::u8())
  .ret(ts::u8())
  .begin();
```
This starts a function that takes a single `u8` parameter named `x` and returns an `u8`. A simpler function that takes no arguments and returns void might simply be defined with: 
```cpp
a.function("bar").begin();
```
The function-body is closed by calling `endFunction()`. Before closing a function, all flow-paths within that function should either end in a return (`Assembler::returnFromFunction(...)`) or an abort (`Assembler::abortProgram`). If there exists a flow-path that does not terminate in a return, an error will be thrown. If a function was defined with a return-type, a value of a compatible type must be passed to `returnFromFunction`. `abortProgram` is called without arguments and will terminate the entire program immediately without return-values being evaluated.

```cpp
a.function("universe")
  .ret(ts::u8())
  .begin();
{
  a.returnFromFunction(literal::u8(42));
} a.endFunction();

```

#### Calling a Function

Function calls use the same builder-pattern as function definitions, the difference being that you call `arg` to pass arguments (one by one), `into` to specify the target to store the return-value into and `done` to finish the builder and perform the call. 

```cpp
a.callFunction("foo")
  .arg("x")
  .into("y")
  .done();
```
In the snippet above, there has to exist a variable `x` with a type that is compatible with the first (and only) parameter of `foo`, and another variable `y` that is compatible with the return-type of `foo`. Note that, while many API calls return `Expression` objects, a function-call returns void and cannot therefore be embedded in other API calls that expect `Expression`s.

### Expressions

Most expression-operations accept flexible operands such as:

- variable names, e.g. `"x"`,
- literals, e.g. `literal::u8(42)`,
- expressions returned by other assembler calls,
- array elements,
- struct fields,
- dereferenced pointers.

For example:

```cpp
a.assign("x", literal::u8(10));             // x = 10
a.assign("y", c.add("x", literal::u8(5)));  // y = x + 5
a.print(c.arrayElement("arr", "idx"));      // print(arr[idx]) 
```

### Data types

Types are created through the `acus::ts` namespace. Each type-factory returns a `ts::TypeHandle` object (just a pointer internally) that you can pass to other API functions that require a type.

#### Integer types
There are four integer types available: two unsigned types and two signed types, both available in 8 or 16 bits:

```cpp
TypeHandle u8  = ts::u8();
TypeHandle s8  = ts::s8();
TypeHandle u16 = ts::u16();
TypeHandle s16 = ts::s16();
```

#### Arrays and strings
Array-types can be defined by passing a previously defined type and the size of the required array to `ts::array`. This will return a unique handle for each unique type/size combination. Strings are more flexible. They are defined by their maximum capacity but can be assigned to other strings of larger capacity; this does not hold for arrays.

```cpp
auto bytes4 = ts::array(u8, 4);
auto string  = ts::string(32);
```

Arrays and strings can be indexed using `Assembler::arrayElement`:

```cpp
a.declareLocal("arr", string);
a.declareLocal("idx", u8);
c.assign("str", literal::string("Hello"));

Literal u8_1 = literal::u8(1);
c.assign("idx", u8_1);
c.write(c.arrayElement("str", "idx")); // runtime index (slow) -> prints 'e'
c.write(c.arrayElement("str", u8_1));  // literal index (fast) -> prints 'e'
```

In the example above, `idx` is a runtime variable (even though we can see what its value will be), so finding the appropriate address in memory will be a lot slower compared to using a literal index.

#### Structs

Struct types are built with `ts::defineStruct(name)`. This function returns a builder object that expects you to call `field` for each field of the struct and finish the definition by calling `done`:

```cpp
auto point = ts::defineStruct("Point")
  .field("x", u8)
  .field("y", u8)
  .done();
*/
```

Struct fields can be accessed by either their name or by index:

```cpp
a.declareLocal("p", point);
auto x = a.structField("p", "x"); // p.x
auto y = a.structField("p", 1);   // p.y
```

#### Pointers
Acus supports pointers (and function pointers, see below):

```cpp
auto u8ptr = ts::pointer(u8); 
```

Pointers can point to locals, globals, array elements, struct fields, and other addressable objects and are initialized using the `Assembler::addressOf` function:

```cpp
a.declareLocal("p", u8ptr);
a.declareLocal("x", u8);

a.assign("p", a.addressOf("x"));
a.assign(a.dereferencePointer("p"), literal::u8('A'));
a.write("x"); // writes 'A' to the output
```

#### Function types and function pointers

To create a function-pointer type, one must first define the type of the function that it points to, with `ts::function()`. This is similar to a function definition, except that no parameter-names are passed to the `param` function.

```cpp
// fnType takes u8 and returns u8
auto fnType = ts::function()
  .ret(u8)
  .param(u8)
  .done();
```

Function pointer types can then be created from function types:

```cpp
auto fPtr = ts::function_pointer(fnType);
```

Function pointers are initialized through literals that refer to named functions and called similarly to normal functions, using `Assembler::callFunctionPointer` instead (the builder works exactly the same):

```cpp
a.declareLocal("fptr", ts::function_pointer(fnType));
a.assign("fptr", literal::function_pointer(fnType, "foo"));
a.callFunctionPointer("fptr").arg("x").into("y").done();
```

### Literals
Literals (represented by the `Literal` type) are compile-time objects that can be used to initialize runtime objects.


#### Integers and Strings
Literals are created through the `acus::literal` namespace. Each builtin type has a corresponding literal factory:

```cpp
literal::u8(65)
literal::s8(-3)
literal::u16(1024)
literal::s16(-300)
literal::string("Hello")
```

#### Arrays
Arrays use builders to compose multiple literals into an array-literal:

```cpp
auto arrType = ts::array(ts::u8(), 4);

auto arr = literal::array(arrType)
  .push(literal::u8('A'))
  .push(literal::u8('B'))
  .push(literal::u8('C'))
  .push(literal::u8('D'))
  .done();
```

#### Struct
One can also build literal struct instantiations at compile-time, once a struct type has been defined:

```cpp
auto point = ts::defineStruct("Point")
  .field("x", ts::u8())
  .field("y", ts::u8())
  .done();

auto p = literal::struct_t(point)
  .init("x", literal::u8('A'))
  .init("y", literal::u8('B'))
  .done();
```

### Global Variables

Global variables are declared inside a program, before the first function-definition. Every function that needs access to the globals, or a subset thereof, must first call `referGlobals` with a list of variables that need to be accessible within the scope of that function. In the example below, both `main` and `foo` refer to the global variable `g`. After `foo` is called, `main` sees the updated version of `g` and will print `F` instead of `A`.

```cpp
a.program("globals", "main").begin(); {

  a.declareGlobal("g", ts::u8());

  a.function("main").begin(); {
    a.referGlobals({"g"});
    a.assign("g", literal::u8('A'));
    a.callFunction("foo").done();
    a.write("g"); // writes F
    a.returnFromFunction();
  } a.endFunction();

  a.function("foo").begin(); {
    a.referGlobals({"g"});
    a.assign("g", literal::u8('F'));
    a.returnFromFunction();
  } a.endFunction();

} a.endProgram();
```

### Input/Output
#### Input
To read a single byte from the input, `Assembler::read` is called, where the argument refers to the destination variable. This destination may refer to any variable of size 1, but most commonly this is `u8`:
```cpp
a.declareLocal("x", u8);
a.read("x");
```

Note that, depending on terminal settings, your terminal will often buffer until you press Enter (which is a character by itself). Acus does not implement any input-parsing. Many terminals support Ctrl+D for end of input.

#### Output
Acus has two main output operations:

```cpp
a.write(value);
a.print(value);
```

`write` writes the raw byte or bytes represented by the value. Arrays and structs can are written word-wise, where 16-bit fields are written in big endian order.

`print` is defined for integers and strings. Integers are sent to the output in decimal notation; it supports the integer widths and signedness currently provided by the type system, for example `u8`, `s8`, `u16`, and `s16`. Strings are written until the terminating null is encountered (in contrast, when passing a string to `write`, all bytes are written, even if the string only takes up part of the total capacity).

```cpp
a.declareLocal("x", ts::s16());
a.assign("x", literal::s16(-300));
a.print("x"); // "-300"
```

### Control flow

Control-flow within a function is achieved by setting labels and jumping between those labels. This is all that's needed to create conditionals and loops. A label is set using `Assembler::label`, which can be jumped to unconditionally using `Assembler::jump` or conditionally using `Assembler::jumpIf`. The latter takes an expression that will be evaluated as a boolean expression (any nonzero will evaluate to true), the label to jump to when the expression evaluates to true, and the label to jump to when the expression evaluates to false. In the example below, a for-loop that would be written in C/C++ as `for (int i = 0; i < n; ++i) { ... }`, is implemented using labels and jumps:

```cpp
a.assign("i", literal::u8(0));
a.label("check");
a.jumpIf(c.lt("i", "n"), "body", "done");
a.label("body");
// ...
a.addAssign("i", literal::u8(1));
a.jump("check");
a.label("done");
```
### Scope

Even though functions already have their own local scope, it can be useful to subdivide that scope into smaller scopes. Variables declared within scopes will be freed at the end of their scope, so this is a way to avoid nameclashes and to manage stack-memory in a simple way. A scope is started using a builder for consistency's sake, but it does not provide any modifying functions. Simply:

```cpp
a.scope().begin(); {
// ...
} a.endScope();

```

### Integer Casts
Integer types may be implicitly cast when there is no size or signedness conflict. For example, a `u8` may be used wherever a `u16` is expected. For all other conversions, an explicit cast can be used with `Assembler::cast`.

```cpp
a.declareLocal("x", s8);
a.declareLocal("y", u8);

a.assign("y", literal::u8(200)); 
a.assign("x", "y");             // fails, different signedness
a.assign("x", a.cast("y", s8)); // OK
```

### Unary and Binary Operations
Many common unary and binary operations have been defined for all integer types. These have dedicated functions (see the full API table below) but can also be called using generic `unOp/unOpAssign` and `binOp/binOpAssign` functions, that take an enum-value as their first argument.

```cpp
a.negate("x");
a.unOp(UnOp::Neg, "x");

a.add("x", "y");
a.binOp(BinOp::Add, "x", "y"); 
```

#### Unary Operators
The table below lists all unary operations. Each of these has a `xxxAssign` version, where the operator is applied to the left-hand operand immediately.

| Function                | `UnOp::xxx` | Description                           |
|-------------------------|-------------|---------------------------------------|
| `lnot/lnotAssign`       | `Not`       | Take logical not.                     |
| `lbool/lboolAssign`     | `Bool`      | Convert to bool, either 0 or 1.       |
| `negate/negateAssign`   | `Neg`       | Negate the operand.                   |
| `abs/absAssign`         | `Abs`       | Take the absolute value.              |
| `signBit/signBitAssign` | `SignBit`   | Compute the sign-bit (1 if negative). |

#### Binary Operators
TODO: make table here

## Public API overview
A full overview of all API functions is shown in the table below:

### `acus::Assembler`

#### Construction and output

| Function | Returns | Description |
|---|---:|---|
| `Assembler` | `Assembler` | Creates an assembler and initializes the type system. |
| `primitives` | `std::string` | Returns a textual dump of the generated primitive instruction sequence for a named program. |
| `brainfuck` | `std::string` | Returns the generated Brainfuck source for a named program. |

#### Program, function, and scope structure

| Function | Returns | Description |
|---|---:|---|
| `program` | `ProgramBuilder` | Creates a program builder for a named program and entry function. Finalize with `begin`. |
| `function` | `FunctionBuilder` | Creates a function builder. Add parameters/return type if needed, then finalize with `begin`. |
| `scope` | `ScopeBuilder` | Creates a nested lexical scope builder. Finalize with `begin`. |
| `endProgram` | `void` | Finishes the current program, validates it, constructs generated support blocks, and stores generated output. |
| `endFunction` | `void` | Ends the current function. |
| `endScope` | `void` | Ends the current lexical scope and frees its locals. |

#### Declarations and global visibility

| Function | Returns | Description |
|---|---:|---|
| `declareLocal` | `Expression` | Declares a local variable in the current scope. |
| `declareGlobal` | `Expression` | Declares a global variable in the current program. |
| `referGlobals` | `void` | Makes selected globals visible inside the current function through local global-reference slots. |

#### Function calls and returns

| Function | Returns | Description |
|---|---:|---|
| `callFunction` | `FunctionCallBuilder` | Starts a direct function call. Add arguments with `arg`, optionally set a destination with `into`, then finalize with `done`. |
| `callFunctionPointer` | `FunctionCallBuilder` | Starts an indirect function-pointer call. Add arguments with `arg`, optionally set a destination with `into`, then finalize with `done`. |
| `returnFromFunction` | `void` | Returns from the current function, optionally with a return value. |
| `abortProgram` | `void` | Emits an abort path for the program. |

#### Expression conversion, assignment, and casting

| Function | Returns | Description |
|---|---:|---|
| `assign` | `Expression` | Assigns a value into an lvalue expression and returns the assigned lvalue. |
| `cast` | `Expression` | Converts an (integer) expression to another type and returns the converted result. |

#### Access expressions

| Function | Returns | Description |
|---|---:|---|
| `structField` | `Expression` | Produces an expression for a struct field, selected by field name or field index. |
| `arrayElement` | `Expression` | Produces an expression for an array/string element, selected by constant or runtime index. |
| `dereferencePointer` | `Expression` | Produces an expression for the value pointed to by a runtime pointer. |
| `addressOf` | `Expression` | Produces a pointer expression referring to an addressable object. |

#### Generic unary and binary operations

| Function | Returns | Description |
|---|---:|---|
| `unOp` | `Expression` | Applies a unary operation and returns a temporary result. |
| `unOpAssign` | `Expression` | Applies a unary operation in-place. |
| `binOp` | `Expression` | Applies a binary operation and returns a temporary result. |
| `binOpAssign` | `Expression` | Applies a binary operation in-place to the left-hand side. |

#### Unary operations

| Function | Returns | Description |
|---|---:|---|
| `lnot` | `Expression` | Computes logical NOT into a temporary result. |
| `lnotAssign` | `Expression` | Applies logical NOT in-place. |
| `lbool` | `Expression` | Converts a value to a boolean-like `0` or `1` result. |
| `lboolAssign` | `Expression` | Converts a value to boolean form in-place. |
| `negate` | `Expression` | Computes arithmetic negation into a temporary result. |
| `negateAssign` | `Expression` | Applies arithmetic negation in-place. |
| `abs` | `Expression` | Computes absolute value into a temporary result. |
| `absAssign` | `Expression` | Applies absolute value in-place. |
| `signBit` | `Expression` | Computes the sign bit of a signed integer into a temporary result. |
| `signBitAssign` | `Expression` | Replaces a signed integer with its sign bit. |

#### Arithmetic operations

| Function | Returns | Description |
|---|---:|---|
| `add` | `Expression` | Computes `lhs + rhs` into a temporary result. |
| `sub` | `Expression` | Computes `lhs - rhs` into a temporary result. |
| `mul` | `Expression` | Computes `lhs * rhs` into a temporary result. |
| `div` | `Expression` | Computes `lhs / rhs` into a temporary result. |
| `mod` | `Expression` | Computes `lhs % rhs` into a temporary result. |
| `addAssign` | `Expression` | Applies `lhs += rhs`. |
| `subAssign` | `Expression` | Applies `lhs -= rhs`. |
| `mulAssign` | `Expression` | Applies `lhs *= rhs`. |
| `divAssign` | `Expression` | Applies `lhs /= rhs`. |
| `modAssign` | `Expression` | Applies `lhs %= rhs`. |

#### Logical operations

| Function | Returns | Description |
|---|---:|---|
| `land` | `Expression` | Computes logical AND into a temporary result. |
| `lnand` | `Expression` | Computes logical NAND into a temporary result. |
| `lor` | `Expression` | Computes logical OR into a temporary result. |
| `lnor` | `Expression` | Computes logical NOR into a temporary result. |
| `lxor` | `Expression` | Computes logical XOR into a temporary result. |
| `lxnor` | `Expression` | Computes logical XNOR into a temporary result. |
| `landAssign` | `Expression` | Applies logical AND in-place. |
| `lnandAssign` | `Expression` | Applies logical NAND in-place. |
| `lorAssign` | `Expression` | Applies logical OR in-place. |
| `lnorAssign` | `Expression` | Applies logical NOR in-place. |
| `lxorAssign` | `Expression` | Applies logical XOR in-place. |
| `lxnorAssign` | `Expression` | Applies logical XNOR in-place. |

#### Comparison operations

| Function | Returns | Description |
|---|---:|---|
| `eq` | `Expression` | Computes `lhs == rhs` into a temporary boolean result. |
| `neq` | `Expression` | Computes `lhs != rhs` into a temporary boolean result. |
| `lt` | `Expression` | Computes `lhs < rhs` into a temporary boolean result. |
| `le` | `Expression` | Computes `lhs <= rhs` into a temporary boolean result. |
| `gt` | `Expression` | Computes `lhs > rhs` into a temporary boolean result. |
| `ge` | `Expression` | Computes `lhs >= rhs` into a temporary boolean result. |
| `eqAssign` | `Expression` | Applies equality comparison in-place. |
| `neqAssign` | `Expression` | Applies inequality comparison in-place. |
| `ltAssign` | `Expression` | Applies less-than comparison in-place. |
| `leAssign` | `Expression` | Applies less-or-equal comparison in-place. |
| `gtAssign` | `Expression` | Applies greater-than comparison in-place. |
| `geAssign` | `Expression` | Applies greater-or-equal comparison in-place. |

#### Output and control flow

| Function | Returns | Description |
|---|---:|---|
| `writeOut` | `void` | Writes the raw byte or bytes represented by a value. |
| `print` | `void` | Writes an integer in decimal notation. Supports unsigned and signed integer types. |
| `label` | `void` | Starts a labeled code block. If already inside a block, the previous block is ended and made to fall through to this label. |
| `jump` | `void` | Ends the current block and sets the next block to a label. |
| `jumpIf` | `void` | Ends the current block and selects one of two labels based on an integer condition. |
| `unreachable` | `void` | Marks the current block as intentionally unreachable. |

### Builder APIs

#### `Assembler::ProgramBuilder`

| Function | Returns | Description |
|---|---:|---|
| `begin` | `void` | Starts program construction for the program described by `program`. |

#### `Assembler::FunctionBuilder`

| Function | Returns | Description |
|---|---:|---|
| `ret` | `FunctionBuilder` | Sets the function return type. Defaults to `void` when omitted. |
| `param` | `FunctionBuilder` | Adds a named parameter to the function. |
| `begin` | `void` | Defines the function and starts its entry block. |

#### `Assembler::ScopeBuilder`

| Function | Returns | Description |
|---|---:|---|
| `begin` | `void` | Starts a nested lexical scope. |

#### `Assembler::FunctionCallBuilder`

| Function | Returns | Description |
|---|---:|---|
| `into` | `FunctionCallBuilder` | Sets the destination for a non-void function return value. |
| `arg` | `FunctionCallBuilder` | Adds an argument to the call. |
| `done` | `void` | Finalizes and emits the call. |

### Type system API

All type factories live in `acus::ts`.

| Function | Returns | Description |
|---|---:|---|
| `init` | `void` | Initializes the shared type system. `Assembler` calls this automatically. |
| `void_t` | `VoidType const *` | Returns the `void` type. |
| `u8` | `IntegerType const *` | Returns the unsigned 8-bit integer type. |
| `s8` | `IntegerType const *` | Returns the signed 8-bit integer type. |
| `u16` | `IntegerType const *` | Returns the unsigned 16-bit integer type. |
| `s16` | `IntegerType const *` | Returns the signed 16-bit integer type. |
| `array` | `ArrayType const *` | Returns a fixed-length array type. |
| `string` | `StringType const *` | Returns a string type with space for a terminating zero byte. |
| `raw` | `RawType const *` | Returns an internal raw storage type of a given slot size. |
| `struct_t` | `StructType const *` | Looks up an already-defined struct type by name. |
| `pointer` | `PointerType const *` | Returns a runtime pointer type for a pointee type. |
| `function_pointer` | `FunctionPointerType const *` | Returns a function pointer type for a function type. |
| `void_function` | `FunctionType const *` | Returns the `void()` function type. |
| `function` | `FunctionTypeBuilder` | Starts building a function type. |
| `defineStruct` | `StructTypeBuilder` | Starts defining a named struct type. |

#### `ts::FunctionTypeBuilder`

| Function | Returns | Description |
|---|---:|---|
| `ret` | `FunctionTypeBuilder` | Sets the return type. Defaults to `void` when omitted. |
| `param` | `FunctionTypeBuilder` | Adds one parameter type. |
| `done` | `FunctionType const *` | Finalizes and returns the function type. |

#### `ts::StructTypeBuilder`

| Function | Returns | Description |
|---|---:|---|
| `field` | `StructTypeBuilder` | Adds a named field to the struct type. |
| `done` | `StructType const *` | Finalizes and returns the struct type. |

### Literal factory API

All literal factories live in `acus::literal`.

| Function | Returns | Description |
|---|---:|---|
| `u8` | `Literal` | Creates an unsigned 8-bit integer literal. |
| `s8` | `Literal` | Creates a signed 8-bit integer literal. |
| `u16` | `Literal` | Creates an unsigned 16-bit integer literal. |
| `s16` | `Literal` | Creates a signed 16-bit integer literal. |
| `string` | `Literal` | Creates a string literal. |
| `function_pointer` | `Literal` | Creates a function pointer literal referring to a named function. |
| `struct_t` | `StructLiteralBuilder` | Starts building a struct literal. |
| `array` | `ArrayLiteralBuilder` | Starts building an array literal. |
| `cast` | concrete literal type | Casts a generic literal wrapper to a concrete literal implementation type. |

#### `literal::StructLiteralBuilder`

| Function | Returns | Description |
|---|---:|---|
| `init` | `StructLiteralBuilder` | Initializes a named field in the literal. |
| `done` | `Literal` | Finalizes and returns the struct literal. |

#### `literal::ArrayLiteralBuilder`

| Function | Returns | Description |
|---|---:|---|
| `push` | `ArrayLiteralBuilder` | Appends an element literal. |
| `done` | `Literal` | Finalizes and returns the array literal. |

### Operator enums

Generic unary and binary operators use these enums.

| Enum | Values | Description |
|---|---|---|
| `UnOp` | `Not`, `Bool`, `Neg`, `Abs`, `SignBit` | Unary operations used by `unOp` and `unOpAssign`. |
| `BinOp` | `Add`, `Sub`, `Mul`, `Div`, `Mod`, `And`, `Or`, `Nand`, `Nor`, `Xor`, `Xnor`, `Eq`, `Neq`, `Lt`, `Le`, `Gt`, `Ge` | Binary operations used by `binOp` and `binOpAssign`. |

String helpers are also available:

| Function | Returns | Description |
|---|---:|---|
| `unOpStr` | `std::string` | Returns a textual name for a unary operator. |
| `binOpStr` | `std::string` | Returns a textual name for a binary operator. |

### Useful public type definitions and helpers

| Name | Kind | Description |
|---|---:|---|
| `types::TypeHandle` | alias | Alias for `types::Type const *`. |
| `types::null` | constant | Null type handle. |
| `types::TypeTag` | enum | Type tags: `VOID`, `RAW`, `U8`, `S8`, `U16`, `S16`, `ARRAY`, `STRING`, `STRUCT`, `POINTER`, `FUNCTION`, `FUNCTION_POINTER`. |
| `types::Signedness` | enum | Integer signedness: `SIGNED` or `UNSIGNED`. |
| `types::cast` | function template | Casts a `TypeHandle` to a concrete type class. |
| `types::isU8` | `bool` | Checks whether a type is `u8`. |
| `types::isU16` | `bool` | Checks whether a type is `u16`. |
| `types::isS8` | `bool` | Checks whether a type is `s8`. |
| `types::isS16` | `bool` | Checks whether a type is `s16`. |
| `types::isInteger` | `bool` | Checks whether a type is any integer type. |
| `types::isSignedInteger` | `bool` | Checks whether a type is signed integer. |
| `types::isUnsignedInteger` | `bool` | Checks whether a type is unsigned integer. |
| `types::isArray` | `bool` | Checks whether a type is an array. |
| `types::isString` | `bool` | Checks whether a type is a string. |
| `types::isArrayLike` | `bool` | Checks whether a type is an array or string. |
| `types::isStruct` | `bool` | Checks whether a type is a struct. |
| `types::isPointer` | `bool` | Checks whether a type is a pointer. |
| `types::isFunction` | `bool` | Checks whether a type is a function type. |
| `types::isFunctionPointer` | `bool` | Checks whether a type is a function pointer. |
| `types::isRaw` | `bool` | Checks whether a type is raw storage. |


## How Acus works internally

This section is a practical orientation for contributors and advanced users. The older architecture documents in `doc/` are still useful background, but the names below follow the current code. In particular, the current `MacroCell` fields are `Value0`, `Value1`, `FrameMarker`, `Flag`, `SeekMarker`, `Scratch0`, `Scratch1`, `Payload0`, and `Payload1`.

### From Brainfuck cells to Acus storage

Brainfuck gives Acus only a linear tape of byte-sized cells and a movable pointer. Acus builds a structured memory model on top of that tape.

The layers are:

```text
Brainfuck cell -> MacroCell field -> MacroCell -> Slot -> Frame -> Program
```

A raw Brainfuck cell is the physical unit. Acus groups several physical cells into one logical `MacroCell`.

```cpp
struct MacroCell {
  enum Field {
    Value0,
    Value1,
    FrameMarker,
    Flag,
    SeekMarker,
    Scratch0,
    Scratch1,
    Payload0,
    Payload1,
    FieldCount
  };
};
```

So logical cell `n`, field `f`, maps to the concrete Brainfuck tape index:

```text
n * MacroCell::FieldCount + f
```

`Value0` and `Value1` hold actual user data. `Value1` is used for wider values such as 16-bit integers and pointer/function-pointer metadata. The other fields are compiler-owned working fields used by navigation, copying, dynamic lookup, and algorithms.

### Cells and the data pointer

The current logical tape position is represented by `Cell` and tracked by `DataPointer`.

A `Cell` contains:

- a logical-cell offset,
- a `MacroCell::Field`.

The assembler uses this symbolic position while emitting primitive operations. When code is finally generated, pointer movement is converted into Brainfuck `>` and `<` movement over the physical tape.

### Slots

A `Slot` is Acus's description of a typed storage location. It has:

- a name,
- a type,
- a kind,
- a logical offset,
- a scope pointer.

Slot kinds currently include:

```text
Local, Global, GlobalReference, ArrayElement, StructField,
Dummy, Available, Temp, Invalid
```

Most public API operations eventually normalize user input into an `Expression`. An expression may refer to a slot, a literal, or a proxy for something that may need materialization. For example:

- `"x"` usually becomes a direct local/global-reference slot expression,
- `literal::u8(42)` becomes a literal expression,
- `c.arrayElement("arr", "idx")` may become a proxy because the real element is selected at runtime,
- `c.dereferencePointer("p")` becomes a pointer-dereference proxy.

A slot's size is measured in logical `MacroCell`s, not raw Brainfuck cells. For example:

- `u8`, `s8`, `u16`, and `s16` each occupy one logical slot,
- arrays occupy `length * element_size` slots,
- structs occupy the sum of their field sizes,
- runtime pointers occupy two logical slots: frame depth and offset,
- function pointers occupy one logical slot but use both value fields.

### Frames

A function call runs in a frame. The current frame layout is defined by `FrameLayout`:

```text
TargetBlock
RunState
ReturnValueStart ...
locals ...
```

`TargetBlock` stores the numeric id of the block that should execute next. It uses `Value0` and `Value1`, so block ids are stored as a 16-bit value.

`RunState` controls whether the dispatch loop should continue executing this frame. When the entry function returns, the run state is cleared and the main Brainfuck loop terminates.

`ReturnValueStart` is the start of the return-value area. The local area begins after the return-value area. Each function has a `FrameLayout` whose return area size is derived from the function return type.

The frame's total logical size is:

```text
ReturnValueStart + returnValueSize + localAreaSize
```

This is important because frame sizes differ per function. Some generated primitive offsets are therefore deferred until code generation using `primitive::Context`.

### Program layout

A program contains:

- bootstrap code,
- generated function blocks,
- generated meta-blocks,
- optional generated builtin print functions,
- hatstrap code that closes the main loop,
- global storage,
- a global block order.

At the beginning of a generated program, Acus sets up a global data frame and the first stack frame. The bootstrap logic:

1. marks the global frame with `SeekMarker`,
2. initializes the first frame's `FrameMarker`,
3. writes the entry block id into `TargetBlock`,
4. sets `RunState` to `1`,
5. opens the main Brainfuck loop.

The hatstrap moves back to `RunState` and closes the main loop.

### Primitive instruction model

The assembler does not directly append Brainfuck text for every public API call. Instead, it appends primitive IR nodes to `primitive::Sequence` objects. Each primitive node can produce:

- a textual debug representation via `text(ctx)`,
- Brainfuck code via `generate(ctx)`.

Important primitive nodes include:

| Primitive | Purpose |
|---|---|
| `MovePointerRelative` | Moves the Brainfuck pointer by a relative amount. The amount may be deferred. |
| `ZeroCell` | Clears the current cell. |
| `ZeroCellPlus` | Clears the current cell by incrementing. |
| `ChangeBy` | Adds a constant or deferred value to the current cell. |
| `MoveData` | Destructively moves a cell value into another cell. |
| `CopyData` | Copies a cell value using a scratch cell. |
| `LoopOpen` / `LoopClose` | Emits Brainfuck loop boundaries. |
| `Out` | Emits output. |
| Boolean/logical primitives | Implement destructive boolean, NOT, AND, OR, XOR-style operations. |

After the full program is assembled, primitive sequences are simplified by merging adjacent compatible nodes. The generated Brainfuck is also simplified by cancelling opposite pointer moves and opposite increments/decrements where possible.

### Code blocks and dispatch

Acus implements control flow through generated blocks rather than raw Brainfuck jumps. Every function contains `Function::Block` objects. Each block has:

- a local name,
- a globally assigned block index,
- a primitive sequence,
- reachability/return metadata,
- outgoing child edges used for validation.

The public label/jump API manipulates these blocks.

At the start of each block, generated code checks two things:

1. whether the current frame's `TargetBlock` equals this block's global block index,
2. whether the current frame's `RunState` is still set.

If both conditions are true, the block body is entered by opening a Brainfuck loop guarded by a flag in the `RunState` macrocell. At the end of the block, the loop closes and the pointer returns to the frame origin.

`label(name)` starts a new block. If it is called while another block is open, the previous block falls through to the new label by setting `TargetBlock` to that label.

`jump(label)` writes the target block id into `TargetBlock`, ends the current block, and requires the next API operation to define a label. This prevents accidental unreachable code from silently continuing in the wrong block.

`jumpIf(condition, trueLabel, falseLabel)` evaluates the condition and writes one of two target block ids. If the condition is a literal, the branch can be folded at assembly time. If it is runtime data, Acus emits a destructive conditional sequence.

When `endProgram()` runs, Acus checks that every reachable path in every function eventually returns, and it reports unreachable code unless a block has been explicitly marked with `unreachable()`.

### Function calls and meta-blocks

A function call is not just a raw jump. Acus has to:

1. synchronize relevant globals,
2. create a new frame,
3. copy arguments into the callee frame,
4. set the callee's entry block as target,
5. transfer control to the callee,
6. return to generated continuation code after the callee returns,
7. fetch the return value if one was requested,
8. restore or pop frames as needed.

The continuation code is represented by generated meta-blocks. A call site records a `MetaBlock` containing the caller, callee, return type, optional return slot, and next block name. Later, `constructMetaBlocks()` generates actual blocks that fetch return data, synchronize globals back into the caller frame, choose the next block, or pop the frame when the run state indicates unwinding.

This is why function calls are relatively expensive compared with straight-line arithmetic. They involve frame setup, argument movement, dispatch state, and generated continuation machinery.

### Function pointers

A function pointer stores the block id of a function entry point. A function-pointer literal resolves to a deferred block index, split across `Value0` and `Value1`.

A direct function call sets the next target block from a known function name. A function-pointer call sets `TargetBlock` from the runtime function pointer value. The same `FunctionCallBuilder` style is used for both:

```cpp
c.callFunction("foo").arg("x").done();
c.callFunctionPointer("fptr").arg("x").done();
```

Function pointer calls are more dynamic because the callee block is selected from runtime data rather than a fixed name.

### Dynamic frame navigation

Brainfuck has no stack pointer abstraction, so Acus uses markers and searches. The current code uses:

- `FrameMarker` to identify frame starts and support moving to previous frames,
- `SeekMarker` as a temporary marker left at a location that must be found again,
- `Payload0` and `Payload1` to carry data while searching across the tape.

`pushFrame()` moves to a new frame after the current frame, copies/increments frame-marker information, sets the new frame's run state, and returns to the new origin.

`popFrame()` clears the entry function's run state if the entry function returns. Otherwise it clears the current frame marker and searches left to the previous frame.

`seek(markerField, direction, payload, checkCurrent)` walks left or right by full macrocell strides until it finds a marker. If a payload is supplied, it moves that payload along while it walks. This mechanism is central to globals, pointers, dynamic array indexing, dereferencing, and return data movement.

### Arrays and dynamic indexing

Static array indexing can often be represented as a direct slot offset.

Dynamic array indexing is more expensive. Acus computes a scaled index by multiplying the runtime index by the element size. Then it plants a `SeekMarker`, moves by a runtime offset, and transfers the selected element through payload fields.

Reading a dynamic element copies the selected element into a temporary slot. Writing a dynamic element copies source data into payload fields, seeks to the runtime-selected position, and moves the payload into the target element.

This is powerful but costly. A frontend targeting Acus should prefer static indexing where possible and avoid repeated dynamic indexing in tight loops unless the semantics require it.

### Pointers and dereferencing

A runtime pointer occupies two logical cells:

```text
RuntimePointer::FrameDepth
RuntimePointer::Offset
```

For a local pointer, `FrameDepth` is zero. For a pointer to global data, Acus stores frame-marker information so that the pointer can navigate back to the global frame. The offset field stores the logical offset of the pointee.

Dereferencing a pointer is expensive because the pointee may be in another frame. `moveToPointee()` copies the pointer fields into payload cells, repeatedly moves to previous frames while decrementing the frame-depth payload, then moves to the runtime offset inside the target frame.

Reading through a pointer copies the pointee value into payload fields, seeks back to the original marked slot, and writes the payload into a materialized temporary.

Writing through a pointer does the opposite: it marks the source, navigates to the pointee, moves source data through payload fields, writes it into the pointee, and then returns.

If pointers can alias globals, Acus performs additional global synchronization so that local global-reference copies and canonical global data stay consistent.

### Globals

Globals live in a separate global data frame. Inside functions, `referGlobals({"g"})` creates local `GlobalReference` slots such as `__g_g`. These slots act like local mirrors of global values.

When a function starts or continues after a call, Acus can fetch global values into these local references. Before calls or pointer dereferences that may alias globals, it can put local values back into the global frame.

Fetching or putting a global involves:

1. marking the current position with `SeekMarker`,
2. seeking left to the global frame,
3. copying values into payload fields or moving payload into the global slot,
4. seeking back to the original frame,
5. clearing the marker.

This design keeps normal local operations simple, but global access is expensive. Treat globals as shared state that must cross frame boundaries, not as cheap local variables.

### Temporary values and materialization

Many expression-producing operations return temporaries. For example:

```cpp
auto tmp = c.add("x", literal::u8(1));
```

A temporary result is usually stored in a compiler-managed temp slot. Some expressions, such as dynamic array elements and pointer dereferences, are represented as proxies that know how to read or write themselves. When an operation needs an actual slot, the proxy materializes into a temporary.

This distinction matters for performance and legality:

- direct locals and static fields are cheap,
- dynamic array elements may require seek/payload transfer,
- dereferenced pointers may require frame navigation,
- temporaries cannot be addressed with `addressOf`.

### Signed integers

Signedness is part of the integer type. `ts::u8()` and `ts::u16()` are unsigned; `ts::s8()` and `ts::s16()` are signed.

Signed integers use the same value fields as unsigned integers, but signed-aware operations dispatch to algorithms that interpret the encoded value as signed. Unary operations such as `negate`, `abs`, and `signBit`, signed comparisons, and signed division/modulo all depend on this type information.

### Printing

`writeOut` emits raw bytes. It is simple when the source is already a byte/string-like value.

`print` emits decimal digits for integer values. The assembler generates builtin print functions on demand:

```text
__print_u8
__print_u16
__print_s8
__print_s16
```

These builtins are only constructed when used. Signed printing delegates to signed-aware logic; unsigned printing uses unsigned decimal conversion.

### Validation and diagnostics

Acus performs API-level validation during construction and deferred validation at program end.

Examples of checked conditions include:

- using API calls in the wrong phase,
- missing `done()` or `begin()` on builders,
- duplicate names,
- duplicate function parameters,
- unknown labels or functions,
- wrong entry function type,
- wrong argument count or type,
- incompatible operands,
- invalid pointer operations,
- execution paths without a return,
- unreachable code.

The public signatures contain `API_FUNC` internally so that diagnostics can point back to the call site, while user code calls the functions normally.

## Testing

Test instructions are intentionally left as a placeholder for now.

```text
TODO: document how to run the runtime and compile-time test suites.
```

The current tests cover areas such as:

- integer literals,
- arithmetic,
- signed arithmetic,
- division and modulo,
- comparisons,
- logical operators,
- decimal printing,
- arrays and dynamic array elements,
- structs and nested structs,
- globals,
- function calls and return values,
- function pointers,
- pointers and pointer arithmetic,
- compile-time API errors.

## Examples

The repository contains examples for:

- hello world,
- writing all byte values directly,
- writing all byte values through a loop,
- iterative Fibonacci,
- recursive Fibonacci.

These examples are useful as executable documentation and should be kept in sync with this README.

## Known limitations

- The public API is still stabilizing.
- Generated Brainfuck prioritizes correctness over compactness.
- Runtime performance depends heavily on tape movement.
- Pointer dereferencing, dynamic indexing, function calls, and global synchronization are powerful but expensive.
- Dynamic library packaging and ABI policy are not documented yet.
- Build and install instructions still need to be finalized.

## License

TODO: add the project license before publication.

## Name

Acus is Latin for needle: a small, precise tool for working on a very long tape.
