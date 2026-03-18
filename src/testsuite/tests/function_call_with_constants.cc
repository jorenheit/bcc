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
      c.assignConst("a", 'Z');
      c.assignConst("b", 'Y');

      c.callFunction("foo", "after_foo", {
	  Function::Arg('A'),
	  Function::Arg("a"),
	  Function::Arg('B'),
	  Function::Arg("b")
	});
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo",
		  ts.voidT(),
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
