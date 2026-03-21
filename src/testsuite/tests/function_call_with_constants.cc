// Tests correct argument ordering when constants and i8 locals are interleaved in a function call.
// Expect: AZBY
Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("a", ts.i8());
    c.declareLocal("b", ts.i8());

    c.beginBlock("entry"); {
      c.assign("a", values::constant(ts.i8(), 'Z'));
      c.assign("b", values::constant(ts.i8(), 'Y'));

      c.callFunction("foo", "after_foo", {
	  values::constant(ts.i8(), 'A'),
	  values::var("a"),
	  values::constant(ts.i8(), 'B'),
	  values::var("b")
	});
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.voidT(),
		  "p0", ts.i8(),
		  "p1", ts.i8(),
		  "p2", ts.i8(),
		  "p3", ts.i8()); {
    c.beginBlock("entry"); {
      c.writeOut("p0");
      c.writeOut("p1");
      c.writeOut("p2");
      c.writeOut("p3");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
