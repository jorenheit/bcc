// Tests if we can pass a value stored in a temporary cell to writeOut
// Expect: "A"

Compiler c;
auto &ts = c.typeSystem();

c.setEntryPoint("main");

c.begin(); {
  c.beginFunction("main"); {
    c.beginBlock("entry"); {
      c.writeOut(values::value(ts.i8(), 'A'));
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
