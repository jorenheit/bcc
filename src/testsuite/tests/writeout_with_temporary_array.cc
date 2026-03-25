// Tests if we can pass an arraystored in a temporary slot to writeOut
// Expect: "ABCD"

Compiler c;
auto &ts = c.typeSystem();

c.setEntryPoint("main");

c.begin(); {
  c.beginFunction("main"); {
    c.beginBlock("entry"); {
      c.writeOut(values::array(ts, ts.i8(), 'A', 'B', 'C', 'D'));
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
