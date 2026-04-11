// Reading from, writing to (mutating) fields of a global struct variable
// Expected: ABAC


TEST_BEGIN

auto point = c.defineStruct("Point",
			    "x", TypeSystem::i8(),
			    "y", TypeSystem::i8());

c.declareGlobal("g", point);

c.beginFunction("main"); {
  c.referGlobals({"g"});
      
  c.beginBlock("entry"); {
    auto gx = c.structField("g", "x");
    auto gy = c.structField("g", "y");

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
    auto gy = c.structField("g", "y");
    c.assign(gy, values::i8('C'));
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
    
TEST_END
