// Tests reading through an array of pointers passed to a function.
// foo reads both pointees, main then prints the original locals.
// Expected: ABAB

TEST_BEGIN

auto i8   = TypeSystem::i8();
auto i8p  = TypeSystem::pointer(i8);
auto i8pa = TypeSystem::array(i8p, 2);

c.beginFunction("main"); {
  c.declareLocal("p", i8pa);
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);

  c.beginBlock("entry"); {
    c.assign("a", values::i8('A'));
    c.assign("b", values::i8('B'));

    c.assign(c.arrayElement("p", 0), c.addressOf("a"));
    c.assign(c.arrayElement("p", 1), c.addressOf("b"));

    auto args = c.constructFunctionArguments("p");
    c.callFunction("foo", "after", args);
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("a");
    c.writeOut("b");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = c.constructFunctionSignature(TypeSystem::voidT(),
					"p", i8pa);
c.beginFunction("foo", sig); {
  c.beginBlock("entry"); {
    auto p0Deref = c.dereferencePointer(c.arrayElement("p", 0));
    auto p1Deref = c.dereferencePointer(c.arrayElement("p", 1));

    c.writeOut(p0Deref);
    c.writeOut(p1Deref);
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
