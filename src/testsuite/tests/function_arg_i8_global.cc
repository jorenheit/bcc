// Tests passing a global i8 by value as a function argument, verifying that modifying the parameter does not mutate the global.
// Expected: GHG
Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
  c.declareGlobal("g", ts.i8());
    
  c.beginFunction("main"); {
    c.referGlobals({"g"});
      
    c.beginBlock("entry"); {
      c.assign("g", values::value(ts.i8(), 'G'));
      c.callFunction("foo", "after_foo", {
	  values::var("g")
	});
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.writeOut("g");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.voidT(), "arg1", ts.i8()); {
    c.beginBlock("entry"); {
      c.writeOut("arg1");
      c.assign("arg1", values::value(ts.i8(), 'H'));
      c.writeOut("arg1");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();

