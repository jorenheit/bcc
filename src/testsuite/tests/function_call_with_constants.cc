// Tests correct argument ordering when constants and i8 locals are interleaved in a function call.
// Expect: AZBY
Compiler c;
c.setEntryPoint("main");

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("a", TypeSystem::i8());
    c.declareLocal("b", TypeSystem::i8());

    c.beginBlock("entry"); {
      c.assign("a", values::i8('Z'));
      c.assign("b", values::i8('Y'));

      c.callFunction("foo", "after_foo",
		     values::i8('A'),
		     values::ref("a"),
		     values::i8('B'),
		     values::ref("b"));
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", TypeSystem::voidT(),
		  "p0", TypeSystem::i8(),
		  "p1", TypeSystem::i8(),
		  "p2", TypeSystem::i8(),
		  "p3", TypeSystem::i8()); {
    c.beginBlock("entry"); {
      c.writeOut("p0");
      c.writeOut("p1");
      c.writeOut("p2");
      c.writeOut("p3");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
