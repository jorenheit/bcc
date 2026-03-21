// Tests basic i8 return-value propagation from callee to caller local.
// Expected: X
using namespace types;
Compiler c;
auto const &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {

  // main: void
  c.beginFunction("main"); {
    c.declareLocal("x", ts.i8());

    c.beginBlock("entry"); {
      c.callFunction("foo", "after_foo", "x");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.writeOut("x");
      c.returnFromFunction();
    } c.endBlock();

  } c.endFunction();

  // foo: returns i8
  c.beginFunction("foo", ts.i8()); {

    c.beginBlock("entry"); {
      c.returnFromFunction(values::value(ts.i8(), 'X'));
    } c.endBlock();

  } c.endFunction();

} c.end();

return c.dumpBrainfuck();
