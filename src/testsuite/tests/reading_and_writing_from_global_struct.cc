// Reading from, writing to (mutating) fields of a global struct variable
// Expected: ABAC

Compiler c;
c.setEntryPoint("main");

auto point = c.defineStruct("Point",
			    "x", TypeSystem::i8(),
			    "y", TypeSystem::i8());
  
c.begin(); {

  c.declareGlobal("g", point);

  c.beginFunction("main"); {
    c.referGlobals({"g"});
      
    c.beginBlock("entry"); {
      auto gx = c.getStructField("g", "x");
      auto gy = c.getStructField("g", "y");

      c.assign(gx, values::i8('A'));
      c.assign(gy, values::i8('B'));
      c.writeOut("g");

      c.callFunction("foo", "after_foo");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.writeOut("g");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo"); {
    c.referGlobals({"g"});
      
    c.beginBlock("entry"); {
      auto gy = c.getStructField("g", "y");
      c.assign(gy, values::i8('C'));
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
    
} c.end();
  
return c.dumpBrainfuck();
