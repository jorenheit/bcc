
# Brainfuck Compiler – Tape Execution Model

This document explains the **runtime behavior of the generated Brainfuck program**
and how the compiler maps high‑level constructs to the tape.

It complements the architecture document and focuses specifically on:

• tape layout  
• stack frames  
• pointer movement  
• function calls  
• meta‑blocks  
• global variables  

---

# 1. Tape Overview

The Brainfuck tape is interpreted as a sequence of **logical cells**.

Each logical cell is composed of several Brainfuck cells forming a **MacroCell**.

```
MacroCell
 ├─ FrameMarker
 ├─ Runtime0
 ├─ Runtime1
 └─ Value
```

Defined in `MacroCell::Field`.

| Field | Purpose |
|------|--------|
FrameMarker | marks start of stack frames |
Runtime0 | runtime scratch channel |
Runtime1 | runtime scratch channel |
Value | actual stored value |

Each logical cell therefore occupies:

```
MacroCell::FieldCount = 4 BF cells
```

---

# 2. Global Memory Region

At the start of the tape lives the **global variable region**.

```
[ Global variables ]
```

Each global is represented by a `Slot` in `Program::globals`.

The total size of the region is:

```
Program::globalVariableFrameSize()
```

Globals are not accessed directly by functions.  
Instead functions use **local shadow copies**.

---

# 3. Stack Frames

After the global region, stack frames are placed sequentially.

Example layout:

```
| globals | frame0 | frame1 | frame2 | ...
```

Each frame corresponds to one active function call.

---

# 4. Frame Layout

Frames follow the structure defined in `FrameLayout`.

```
Offset   Field
--------------------------
0        ReturnBlock
1        TargetBlock
2        RunState
3        ReturnValue
4+       Local variables
```

Constants are defined in:

```
FrameLayout::Offsets
```

### Field meanings

**ReturnBlock**

Index of the block to execute when returning to the caller.

**TargetBlock**

Dispatcher target determining which block executes next.

**RunState**

Program execution flag.

```
0 → halt
1 → continue
```

**ReturnValue**

Location where functions place return values.

**Locals**

All declared locals and referenced globals.

---

# 5. Block Dispatcher

The generated program uses a dispatcher loop.

Conceptually:

```
while (RunState != 0)
    execute block(TargetBlock)
```

Each block begins with code equivalent to:

```
if (TargetBlock == myIndex)
    run block body
```

The compiler generates this using loops and comparisons in Brainfuck.

---

# 6. Pointer Invariants

At **block boundaries**, the following invariant is maintained:

```
pointer → frame base
field   → Value
```

This simplifies reasoning about pointer movements.

Whenever the compiler emits instructions, it assumes this invariant holds.

---

# 7. Pointer Movement

Pointer movement happens at two levels.

## Logical movement

Moves between logical cells.

Implemented with:

```
MovePointerRelative
```

Actual BF distance:

```
amount * MacroCell::FieldCount
```

## Field switching

Moves between fields inside a MacroCell.

Implemented with:

```
MovePointerRelativeRaw
```

Example:

```
Value → FrameMarker
```

requires moving left 3 cells.

---

# 8. Function Call Walkthrough

Example:

```
main → foo
```

### Step 1: prepare continuation

Caller sets:

```
TargetBlock = metaBlock
```

The meta block handles return processing.

### Step 2: push frame

Compiler emits:

```
PushFrame(callee)
```

Runtime behavior:

```
pointer += callerFrameSize
```

New frame becomes active.

### Step 3: jump to callee entry

```
TargetBlock = foo.entry
```

Execution continues in the callee.

---

# 9. Return Walkthrough

When a function returns, it executes:

```
PopFrame()
```

Runtime algorithm:

```
while FrameMarker != 1
    move left
```

This finds the previous frame start.

Once located, the caller frame becomes active again.

The dispatcher then executes the **meta block** stored earlier.

---

# 10. Meta Block Execution

Meta blocks are generated automatically for each call site.

Responsibilities:

1. move forward into callee frame
2. read return value
3. read run state
4. copy values back to caller frame
5. restore pointer
6. jump to continuation block

This mechanism avoids needing to carry values across dynamic pointer scans.

---

# 11. Global Variable Synchronization

Globals are accessed through **local shadow copies**.

## On function entry

```
CopyGlobalToLocal
```

All referenced globals are loaded into local slots.

## Before a function call

```
CopyLocalToGlobal
```

Ensures global state is visible to other frames.

## After a call

Meta block reloads globals:

```
CopyGlobalToLocal
```

## On return

Globals are written back again.

```
CopyLocalToGlobal
```

---

# 12. Temporary Variables

Temporary storage is allocated dynamically within a block.

Temps are stored in frame slots like locals but managed separately.

Allocation:

```
allocateTemp()
```

Release:

```
freeTemp()
```

Temps may reuse slots previously freed.

---

# 13. Example Tape Snapshot

Example call stack:

```
| globals | main frame | foo frame |
```

Inside `foo` execution:

```
pointer → foo frame base
```

After return:

```
pointer → main frame base
```

The meta block then resumes execution in `main`.

---

# 14. Execution Timeline Example

```
main.entry
    ↓
call foo
    ↓
push frame
    ↓
foo.entry
    ↓
foo.return
    ↓
pop frame
    ↓
meta block
    ↓
main.afterCall
```

---

# 15. Important Runtime Guarantees

The compiler maintains several invariants:

1. pointer always returns to frame base between blocks
2. frames are strictly stack structured
3. blocks never change frame ownership
4. globals are synchronized before and after calls

These guarantees allow simple and predictable code generation.

---

# 16. Debugging Tips

If runtime behavior seems incorrect, check:

• frame offsets in IR dump  
• block index assignments  
• pointer field switching  
• global synchronization sequences  

Incorrect pointer field alignment is a common source of bugs.

---

End of document.
