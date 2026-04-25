// Tests mixed parameter packing with i8, i16, and i8 array arguments in a single function call.
// Expect: ABCDEF

TEST_BEGIN
auto array3 = TypeSystem::array(TypeSystem::i8(), 3);

c.beginFunction("main"); {
  c.declareLocal("a", TypeSystem::i8());
  c.declareLocal("b", TypeSystem::i16());
  c.declareLocal("arr", array3);

  c.beginBlock("entry"); {
    auto arr0 = c.arrayElement("arr", 0);
    auto arr1 = c.arrayElement("arr", 1);
    auto arr2 = c.arrayElement("arr", 2);

    c.assign("a",  values::i8('A'));
    c.assign("b",  values::i16(CAT('B', 'C')));
    c.assign(arr0, values::i8('D'));
    c.assign(arr1, values::i8('E'));
    c.assign(arr2, values::i8('F'));

    c.callFunction("foo", "after_foo")("a", "b", "arr");
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = TypeSystem::function(TypeSystem::voidT(), TypeSystem::i8(), TypeSystem::i16(), array3);
c.beginFunction("foo", fooSig, {"p0", "p1", "p2"}); {
  c.beginBlock("entry"); {
    c.writeOut("p0");
    c.writeOut("p1");
    c.writeOut("p2");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
