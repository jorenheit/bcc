// Tests recursive pointer relocation through array -> struct -> pointer.
// foo reads through both pointer fields, then writes through both.
// Expected: ABXY

TEST_BEGIN

auto i8  = TypeSystem::i8();
auto i8p = TypeSystem::pointer(i8);

auto holderFields = c.constructFields("p", i8p);
auto holder = c.defineStruct("Holder", holderFields);
auto holders = TypeSystem::array(holder, 2);

c.beginFunction("main"); {
  c.declareLocal("arr", holders);
  c.declareLocal("a", i8);
  c.declareLocal("b", i8);

  c.beginBlock("entry"); {
    c.assign("a", values::i8('A'));
    c.assign("b", values::i8('B'));

    c.assign(c.structField(c.arrayElement("arr", 0), "p"), c.addressOf("a"));
    c.assign(c.structField(c.arrayElement("arr", 1), "p"), c.addressOf("b"));

    auto args = c.constructFunctionArguments("arr");
    c.callFunction("foo", "after", args);
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("a");
    c.writeOut("b");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();


auto sig = c.constructFunctionSignature(TypeSystem::voidT(), "arr", holders);
c.beginFunction("foo", sig); {
  c.beginBlock("entry"); {
    auto p0 = c.structField(c.arrayElement("arr", 0), "p");
    auto p1 = c.structField(c.arrayElement("arr", 1), "p");

    auto d0 = c.dereferencePointer(p0);
    auto d1 = c.dereferencePointer(p1);

    c.writeOut(d0);
    c.writeOut(d1);

    c.assign(d0, values::i8('X'));
    c.assign(d1, values::i8('Y'));

    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
