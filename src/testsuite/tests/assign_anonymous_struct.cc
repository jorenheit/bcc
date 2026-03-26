Compiler c;
c.setEntryPoint("main");

auto point = c.defineStruct("Point",
			    "x", TypeSystem::i8(),
			    "y", TypeSystem::i8());

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("s", point);

    c.beginBlock("entry"); {
      c.assign("s", values::structT("Point",
				    values::i8('A'),
				    values::i8('B')));
      c.writeOut("s");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();
  
return c.dumpBrainfuck();
