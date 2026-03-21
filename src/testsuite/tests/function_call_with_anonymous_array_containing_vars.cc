// Tests passing a nested anonymous array by value to a function
// Expect: AB
Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

auto array2 = ts.array(ts.i8(), 2);

using namespace types;
c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("x", ts.i8());
    c.beginBlock("entry"); {
      c.assign("x", values::value(ts.i8(), 'B'));
      c.callFunction("foo", "after_foo", {
	  values::value(array2, 'A', "x")
	});
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
