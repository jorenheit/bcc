// Tests recursive self-calls with multiple i8 parameters, stressing repeated frame creation and parameter re-passing.
// Expect: ABCABCABCABC...
Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
  c.beginFunction("main"); {
    c.beginBlock("entry"); {
      c.callFunction("foo", "after_foo", {
	  values::value(ts.i8(), 'A'),
	  values::value(ts.i8(), 'B'),
	  values::value(ts.i8(), 'C')
	});
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.voidT(),
		  "x", ts.i8(),
		  "y", ts.i8(),
		  "z", ts.i8()); {
    c.beginBlock("entry"); {
      c.writeOut("x");
      c.writeOut("y");
      c.writeOut("z");

      c.callFunction("foo", "after_recurse", {
	  values::var("x"),
	  values::var("y"),
	  values::var("z")
	});
    } c.endBlock();

    c.beginBlock("after_recurse"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();

