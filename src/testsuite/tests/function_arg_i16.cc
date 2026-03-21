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
      c.assign("x", values::value(ts.i16(), CAT('C', 'D')));
      c.callFunction("foo", "after_foo", {
	  values::value(ts.i16(), CAT('A', 'B')),
	  values::var("x")
	}, "y");
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

