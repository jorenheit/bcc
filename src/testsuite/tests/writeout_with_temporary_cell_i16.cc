// Tests if we can pass a i16 stored in a temporary cell to writeOut
// Expect: "AB"

Compiler c;
auto &ts = c.typeSystem();

c.setEntryPoint("main");

c.begin(); {
  c.beginFunction("main"); {
    c.beginBlock("entry"); {
      c.writeOut(values::i16(ts, CAT('A', 'B')));
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
