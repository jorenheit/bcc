// Tests pass-by-value of an array of i16 values, verifying multi-cell argument copying across a function call.
// Expect: ABCDABCD
Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

auto array2 = ts.array(ts.i16(), 2);

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("x", array2);

    c.beginBlock("entry"); {
      Slot x0 = c.arrayElementConst("x", 0);
      Slot x1 = c.arrayElementConst("x", 1);

      c.assign(x0, values::value(ts.i16(), CAT('A', 'B')));
      c.assign(x1, values::value(ts.i16(), CAT('C', 'D')));

      c.writeOut("x");
      c.callFunction("foo", "after_foo", {
	  values::var("x")
	});
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.voidT(), "x", array2); {
    c.beginBlock("entry"); {
      c.writeOut("x");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
