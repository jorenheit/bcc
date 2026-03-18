// Tests mixed parameter packing with i8, i16, and i8 array arguments in a single function call.
// Expect: ABCDEF
Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("a", ts.i8());
    c.declareLocal("b", ts.i16());
    c.declareLocal("arr", ts.array(ts.i8(), 3));

    c.beginBlock("entry"); {
      Slot arr0 = c.arrayElementConst("arr", 0);
      Slot arr1 = c.arrayElementConst("arr", 1);
      Slot arr2 = c.arrayElementConst("arr", 2);

      c.assignConst("a", 'A');
      c.assignConst("b", CAT('B', 'C'));
      c.assignConst(arr0, 'D');
      c.assignConst(arr1, 'E');
      c.assignConst(arr2, 'F');

      c.callFunction("foo", "after_foo", {
	  Function::Arg("a"),
	  Function::Arg("b"),
	  Function::Arg("arr")
	});
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo",
		  ts.voidT(),
		  "p0", ts.i8(),
		  "p1", ts.i16(),
		  "p2", ts.array(ts.i8(), 3)); {
    c.beginBlock("entry"); {
      c.writeOut("p0");
      c.writeOut("p1");
      c.writeOut("p2");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
