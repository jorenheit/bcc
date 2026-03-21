// Tests pass-by-value of an i8 array argument, verifying full array copy and correct element ordering in caller and callee.
// Expected: ABCDABCD

Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

auto array4 = ts.array(ts.i8(), 4);

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("x", array4);
       
    c.beginBlock("entry"); {
      Slot x0 = c.arrayElementConst("x", 0);
      Slot x1 = c.arrayElementConst("x", 1);
      Slot x2 = c.arrayElementConst("x", 2);
      Slot x3 = c.arrayElementConst("x", 3);

      c.assign(x0, values::value(ts.i8(), 'A'));
      c.assign(x1, values::value(ts.i8(), 'B'));
      c.assign(x2, values::value(ts.i8(), 'C'));
      c.assign(x3, values::value(ts.i8(), 'D'));

      c.writeOut("x");
      c.callFunction("foo", "after_foo", values::List{
	  values::var("x")
	});
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.voidT(), "x", array4); {
    c.beginBlock("entry"); {
      c.writeOut("x");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
