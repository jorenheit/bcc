// Tests assigning a function return value directly into a global and reading it back in a later call.
// Expected result: YY

using namespace types;
Compiler c;
auto const &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
  c.declareGlobal("g", ts.i8());

  c.beginFunction("main"); {
    c.referGlobals({"g"});
      
    c.beginBlock("entry"); {
      c.callFunctionReturn("foo", "after_foo", "g");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.writeOut("g");
      c.callFunction("bar", "after_bar");
    } c.endBlock();

    c.beginBlock("after_bar"); {
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

  c.beginFunction("foo", ts.i8()); {
    c.declareLocal("y", ts.i8());
    c.beginBlock("entry"); {
      c.assign("y", values::i8(ts, 'Y'));
      c.returnFromFunction("y");
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
