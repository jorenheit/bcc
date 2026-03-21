// Tests forwarding an i8 parameter through a nested call chain (main -> foo -> bar).
// Expect: A
Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
  c.beginFunction("main"); {
    c.beginBlock("entry"); {
      c.callFunction("foo", "after_foo", {
	  values::constant(ts.i8(), 'A')
	});
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.voidT(), "x", ts.i8()); {
    c.beginBlock("entry"); {
      c.callFunction("bar", "after_bar", {
	  values::var("x")
	});
    } c.endBlock();

    c.beginBlock("after_bar"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("bar", ts.voidT(), "y", ts.i8()); {
    c.beginBlock("entry"); {
      c.writeOut("y");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
