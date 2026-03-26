Compiler c;
c.setEntryPoint("main");

auto point = c.defineStruct("Point",
			    "x", TypeSystem::i8(),
			    "y", TypeSystem::i8());

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("s", point);

    c.beginBlock("entry"); {
      c.callFunctionReturn("makePoint", "after_makePoint", "s");
    } c.endBlock();

    c.beginBlock("after_makePoint"); {
      c.writeOut("s");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("makePoint", point); {
    c.declareLocal("p", point);
    c.beginBlock("entry"); {
      auto x = c.getStructField("p", "x");
      auto y = c.getStructField("p", "y");

      c.assign(x, values::i8('Q'));
      c.assign(y, values::i8('R'));
      c.returnFromFunction("p");
    } c.endBlock();
  } c.endFunction();
} c.end();
  
return c.dumpBrainfuck();
