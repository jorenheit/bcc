// Add i8 integers together using add and addAssign
// Expected: ADG

TEST_BEGIN

c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i8());
  c.declareLocal("y", TypeSystem::i8());
  c.declareLocal("z", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("x", values::i8('A'));
    c.assign("y", values::i8(3));

    c.writeOut("x"); // A
    c.addAssign("x", "y"); // x += y
    c.writeOut("x"); // D
    c.assign("z", c.add("x", "y")); // z = x + y
    c.writeOut("z"); // G
	
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
