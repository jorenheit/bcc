// Initialize a struct from an anonymous value
// Expected: "AB"

TEST_BEGIN
auto pointFields = c.constructFields("x", TypeSystem::i8(),
				     "y", TypeSystem::i8());
auto point = c.defineStruct("Point", pointFields);

c.beginFunction("main"); {
  c.declareLocal("s", point);

  c.beginBlock("entry"); {
    c.assign("s", values::structT(point,
				  values::i8('A'),
				  values::i8('B')));
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
