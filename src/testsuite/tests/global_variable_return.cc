// Tests returning a global value into a caller local while preserving the global for later reads.
// GG
using namespace types;
Compiler c;
auto const &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
  c.declareGlobal("g", ts.i8());

  c.beginFunction("main"); {
    c.declareLocal("x", ts.i8());
      
    c.beginBlock("entry"); {
      c.callFunction("foo", "after_foo", "x");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.writeOut("x");
      c.callFunction("bar", "after_bar");
    } c.endBlock();

    c.beginBlock("after_bar"); {
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

  c.beginFunction("foo", ts.i8()); {
    c.referGlobals({"g"});
    c.beginBlock("entry"); {
      c.assign("g", values::constant(ts.i8(), 'G'));
      c.returnFromFunction("g");
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("bar"); {
    c.referGlobals({"g"});
    c.beginBlock("entry"); {
      c.writeOut("g");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
    
} c.end();

return c.dumpBrainfuck();
