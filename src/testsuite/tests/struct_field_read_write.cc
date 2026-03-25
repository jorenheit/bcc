// Tests writing to struct fields and reading back from them.
// Expected: "ABAB"

Compiler c;
auto &ts = c.typeSystem();

c.setEntryPoint("main");

// TODO: simplify syntax
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

      c.writeOut(x);
      c.writeOut(y);
      c.writeOut("s");
	
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

    
} c.end();

return c.dumpBrainfuck();
