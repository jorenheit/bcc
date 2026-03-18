// Tests pass-by-value of an i8 array argument, verifying full array copy and correct element ordering in caller and callee.
// Expected: ABCDABCD

Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("x", ts.array(ts.i8(), 4));
       
    c.beginBlock("entry"); {
      Slot x0 = c.arrayElementConst("x", 0);
      Slot x1 = c.arrayElementConst("x", 1);
      Slot x2 = c.arrayElementConst("x", 2);
      Slot x3 = c.arrayElementConst("x", 3);

      c.assignConst(x0, 'A');
      c.assignConst(x1, 'B');
      c.assignConst(x2, 'C');
      c.assignConst(x3, 'D');

      c.writeOut("x");
      c.callFunction("foo", "after_foo", {
	  Function::Arg("x")
	});
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.voidT(), "x", ts.array(ts.i8(), 4)); {
    c.beginBlock("entry"); {
      c.writeOut("x");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
