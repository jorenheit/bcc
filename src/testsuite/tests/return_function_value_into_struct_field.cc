// Store the return-value of a function into a struct-field
// Expected: "A"

TEST_BEGIN

auto pointFields = c.constructFields("x", TypeSystem::i8(),
				     "y", TypeSystem::i8());
auto point = c.defineStruct("Point", pointFields);

c.beginFunction("main"); {
  c.declareLocal("s", point);

  c.beginBlock("entry"); {
    auto x = c.structField("s", "x");
    auto y = c.structField("s", "y");

    c.assign(x, values::i8('A'));
    c.callFunction("makeZ", "after_makeZ", {}, y);
  } c.endBlock();

  c.beginBlock("after_makeZ"); {
    c.writeOut("s");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = TypeSystem::function(TypeSystem::i8());
c.beginFunction("makeZ", sig); {
  c.beginBlock("entry"); {
    c.returnFromFunction(values::i8('Z'));
  } c.endBlock();
} c.endFunction();

TEST_END
