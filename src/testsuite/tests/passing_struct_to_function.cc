Compiler c;
auto &ts = c.typeSystem();

c.setEntryPoint("main");

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
      c.writeOut("s");

      c.callFunction("foo", "return", values::List{
	  values::var("s")
	});
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
