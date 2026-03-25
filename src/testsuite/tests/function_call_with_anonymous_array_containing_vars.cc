// Tests passing a nested anonymous array by value to a function
// Expect: AB
Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

auto array2 = ts.array(ts.i8(), 2);

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("x", ts.i8());
    c.beginBlock("entry"); {
      c.assign("x", values::i8(ts, 'B'));
      c.callFunction("foo", "after_foo", values::array(ts, ts.i8(), 'A', "x"));
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.voidT(),
		  "arr", array2); {
    c.beginBlock("entry"); {
      c.writeOut("arr");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
