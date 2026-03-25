// Tests passing a nested anonymous array by value to a function
// Expect: ABCD
Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

auto array2 = ts.array(ts.i8(), 2);
auto array22 = ts.array(array2, 2);

using namespace types;
c.begin(); {
  c.beginFunction("main"); {
    c.beginBlock("entry"); {
      auto ab = values::array(ts, ts.i8(), 'A', 'B');
      auto cd = values::array(ts, ts.i8(), 'C', 'D');
      c.callFunction("foo", "after_foo", values::array(ts, array2, ab, cd));
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.voidT(),
		  "arr", array22); {
    c.beginBlock("entry"); {
      c.writeOut("arr");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
