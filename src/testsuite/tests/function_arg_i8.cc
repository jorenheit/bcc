// Tests passing an i8 constant and an i8 local as function arguments, and returning an i8 value into a caller local.
// Expected: AB

Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("x", ts.i8());
    c.declareLocal("y", ts.i8());
       
    c.beginBlock("entry"); {
      c.assignConst("x", 'A');
      c.callFunction("foo", "after_foo", {
	  Function::Arg('B'),
	  Function::Arg("x")
	}, "y");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.writeOut("x");
      c.writeOut("y");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.i8(), "arg1", ts.i8(), "arg2", ts.i8()); {
    c.beginBlock("entry"); {
      c.returnFromFunction("arg1");
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
