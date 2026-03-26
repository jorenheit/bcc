Compiler c;
c.setEntryPoint("main");

auto point = c.defineStruct("Point",
			    "x", TypeSystem::i8(),
			    "y", TypeSystem::i8());

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("s", point);

    c.beginBlock("entry"); {
      auto x = c.getStructField("s", "x");
      auto y = c.getStructField("s", "y");

      c.assign(x, values::i8('A'));
      c.callFunctionReturn("makeZ", "after_makeZ", y);
    } c.endBlock();

    c.beginBlock("after_makeZ"); {
      c.writeOut("s");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("makeZ", TypeSystem::i8()); {
    c.beginBlock("entry"); {
      c.returnFromFunction(values::i8('Z'));
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
