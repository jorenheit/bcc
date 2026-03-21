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
      auto ab = values::value(array2, 'A', 'B');
      auto cd = values::value(array2, 'C', 'D');
      c.callFunction("foo", "after_foo", {
	  values::value(array22, ab, cd)
	});
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
