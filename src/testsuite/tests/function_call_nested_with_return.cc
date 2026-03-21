// Tests nested calls with return values, propagating an i8 argument through foo -> bar and back to main.
// Expect: Z
Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("r", ts.i8());

    c.beginBlock("entry"); {
      c.callFunction("foo", "after_foo", {
	  values::value(ts.i8(), 'Z')
	}, "r");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.writeOut("r");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.i8(), "x", ts.i8()); {
    c.declareLocal("tmp", ts.i8());

    c.beginBlock("entry"); {
      c.callFunction("bar", "after_bar", {
	  values::var("x")
	}, "tmp");
    } c.endBlock();

    c.beginBlock("after_bar"); {
      c.returnFromFunction("tmp");
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("bar", ts.i8(), "y", ts.i8()); {
    c.beginBlock("entry"); {
      c.returnFromFunction("y");
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
