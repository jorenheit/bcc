// Tests mixed parameter packing with i8, i16, and i8 array arguments in a single function call.
// Expect: ABCDEF
Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

auto array3 = ts.array(ts.i8(), 3);

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("a", ts.i8());
    c.declareLocal("b", ts.i16());
    c.declareLocal("arr", array3);

    c.beginBlock("entry"); {
      Slot arr0 = c.arrayElementConst("arr", 0);
      Slot arr1 = c.arrayElementConst("arr", 1);
      Slot arr2 = c.arrayElementConst("arr", 2);

      c.assign("a",  values::i8(ts, 'A'));
      c.assign("b",  values::i16(ts, CAT('B', 'C')));
      c.assign(arr0, values::i8(ts, 'D'));
      c.assign(arr1, values::i8(ts, 'E'));
      c.assign(arr2, values::i8(ts, 'F'));

      c.callFunction("foo", "after_foo", 
		     values::ref("a"),
		     values::ref("b"),
		     values::ref("arr"));
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.voidT(),
		  "p0", ts.i8(),
		  "p1", ts.i16(),
		  "p2", array3); {
    c.beginBlock("entry"); {
      c.writeOut("p0");
      c.writeOut("p1");
      c.writeOut("p2");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
