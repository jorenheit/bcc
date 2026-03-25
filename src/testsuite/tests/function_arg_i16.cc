// Tests passing i16 arguments (constant and local) and returning an i16 value from the callee.
// Expect: AB
Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("x", ts.i16());
    c.declareLocal("y", ts.i16());

    c.beginBlock("entry"); {
      c.assign("x", values::i16(ts, CAT('C', 'D')));
      c.callFunctionReturn("foo", "after_foo", "y",
			   values::i16(ts, CAT('A', 'B')),
			   values::ref("x"));
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.writeOut("y");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.i16(), "arg1", ts.i16(), "arg2", ts.i16()); {
    c.beginBlock("entry"); {
      c.returnFromFunction("arg1");
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();

