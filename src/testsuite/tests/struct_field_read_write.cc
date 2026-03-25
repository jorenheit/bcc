// Tests writing to struct fields and reading back from them.
// Expected: "ABAB"

Compiler c;
auto &ts = c.typeSystem();

c.setEntryPoint("main");

// TODO: simplify syntax
auto point = c.defineStruct("Point",
			    types::StructType::Field{"x", ts.i8()},
			    types::StructType::Field{"y", ts.i8()});
  
c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("s", point);

      
    c.beginBlock("entry"); {
      auto x = c.getStructField(values::Var("s"), "x");
      auto y = c.getStructField(values::Var("s"), "y");
      
      c.assign(x, values::value(ts.i8(), 'A'));
      c.assign(y, values::value(ts.i8(), 'B'));

      c.writeOut(x);
      c.writeOut(y);
      c.writeOut("s");
	
      c.returnFromFunction();
    } c.endBlock();
      
  } c.endFunction();

    
} c.end();

return c.dumpBrainfuck();
