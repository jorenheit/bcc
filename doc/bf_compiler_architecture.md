# BF Compiler Architecture (Updated)

## Overview

This compiler translates a higher‑level IR into Brainfuck programs that
run on a structured runtime model built on the BF tape. The runtime
simulates:

-   stack frames
-   block-based control flow
-   function calls and returns
-   global/local synchronization

Execution is controlled by a **block dispatcher** which repeatedly
checks which block should execute next.

------------------------------------------------------------------------

# Runtime Execution Model

The runtime operates on a structured tape model consisting of **stack
frames** and **macrocells**.

Each frame contains several logical cells, each of which contains a
macrocell with multiple fields.

At block boundaries the following invariant always holds:

-   The pointer is at the **frame origin**
-   The active field is **Value**

------------------------------------------------------------------------

# MacroCell Layout

Each logical cell contains several fields:

  Field         |Purpose
  ------------- |----------------------------------
  FrameMarker   |Marks the start of a stack frame
  Runtime0      |Scratch register
  Runtime1      |Scratch register
  EnterFlag     |Block entry condition
  Value         |Actual cell value

### Helper Field Invariant

The helper fields:

-   `Runtime0`
-   `Runtime1`
-   `EnterFlag`

**must always be reset to `0` after they are used.**

This ensures helper cells behave like temporary registers and prevents
hidden state from affecting later execution.

------------------------------------------------------------------------

# Frame Model

Each function call creates a new stack frame.

A frame contains:

-   local variables
-   shadow copies of globals
-   control state fields

Important fields include:

  Field         |Purpose
  ------------- |-----------------------------------
  TargetBlock   |Which block should execute next
  RunState      |Whether execution should continue
  ReturnValue   |Function return storage
  FrameMarker   |Used to locate frame boundaries

### Frame Boundary Invariant

Frame boundaries are detected by scanning left until the `FrameMarker`
field is encountered.

------------------------------------------------------------------------

# Block Dispatcher

Execution proceeds as a repeated dispatch loop.

Each block begins with a **block-open routine** that checks two
conditions:

1.  Does `TargetBlock` match the block's index?
2.  Is `RunState` still enabled?

If both conditions hold:

    EnterFlag = 1

Otherwise:

    EnterFlag = 0

The block is entered using:

    (EnterFlag) [ (Reset EnterFlag), ... ]

This loop executes the block body exactly once.

------------------------------------------------------------------------

# Block Lifecycle

Each block consists of:

1.  **blockOpen**
2.  block body
3.  **blockClose**

### blockOpen

Performs:

-   block index comparison
-   run-state check
-   initialization of `EnterFlag`

### blockClose

Performs:

-   closing the block loop
-   restoring pointer position
-   restoring active field

------------------------------------------------------------------------

# Function Calls

A function call is compiled as:

1.  Synchronize local → global state
2.  Set `TargetBlock` to a **meta return block**
3.  Push a new stack frame
4.  Mark the frame boundary
5.  Set `TargetBlock` to the callee entry block

------------------------------------------------------------------------

# Meta Return Blocks

Each call site generates a **meta block** which runs after the callee
returns.

Meta block responsibilities:

1.  Fetch return data from the callee frame
2.  Synchronize global → local state
3.  Set `TargetBlock` to the continuation block

Meta blocks are generated **after normal compilation** to avoid nested
`beginBlock` / `endBlock` calls during code generation.

------------------------------------------------------------------------

# Context Snapshot

Code generation uses a lightweight immutable context containing:

  Field            Purpose
  ---------------- --------------------------------------------
  fieldCount       Number of macrocell fields
  blockIDtoIndex   Maps `function.block` → global block index
  stackFrameSize   Maps function → frame size

This context allows deferred resolution of values such as:

-   block indices
-   stack frame sizes

during generation time.

------------------------------------------------------------------------

# Compiler Invariants

The compiler assumes the following invariants:

1.  Pointer is at **frame origin** at block boundaries
2.  Active field is **Value** at block boundaries
3.  First block of a function is its **entry block**
4.  Helper fields (`Runtime0`, `Runtime1`, `EnterFlag`) are **zero after
    use**
5.  Frame boundaries are marked with **FrameMarker**
6.  Block identifiers (strings) follow the pattern: `<function>.<block>` with the special identifier `<function>` representing the entry block.

------------------------------------------------------------------------

# Notes

This document reflects the current architecture of the BF compiler
runtime and should be updated whenever invariants or execution structure
change.
