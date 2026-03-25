Compiler c;
auto &ts = c.typeSystem();

c.setEntryPoint("main");

auto point = c.defineStruct("Point",
			    "x", ts.i8(),
			    "y", ts.i8());

  
c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("s", point);

    c.beginBlock("entry"); {
      auto x = c.getStructField("s", "x");
      auto y = c.getStructField("s", "y");
      
      c.assign(x, values::i8(ts, 'A'));
      c.assign(y, values::i8(ts, 'B'));
      c.writeOut("s");

      c.callFunction("foo", "return", "s");
    } c.endBlock();
      
    c.beginBlock("return"); {
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

  c.beginFunction("foo", ts.voidT(), "s", point); {
    c.beginBlock("entry"); {
      c.writeOut("s");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

    
    
} c.end();

return c.dumpBrainfuck();
