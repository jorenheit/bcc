// Tests mixed parameter packing with i8, i16, and i8 array arguments in a single function call.
// Expect: ABCDEF

TEST_BEGIN
auto array3 = ts::array(ts::i8(), 3);

c.function("main").begin(); {
  c.declareLocal("a", ts::i8());
  c.declareLocal("b", ts::i16());
  c.declareLocal("arr", array3);

  c.block("entry").begin(); {
    auto arr0 = c.arrayElement("arr", 0);
    auto arr1 = c.arrayElement("arr", 1);
    auto arr2 = c.arrayElement("arr", 2);

    c.assign("a",  literal::i8('A'));
    c.assign("b",  literal::i16(CAT('B', 'C')));
    c.assign(arr0, literal::i8('D'));
    c.assign(arr1, literal::i8('E'));
    c.assign(arr2, literal::i8('F'));

    c.callFunction("foo", "after_foo").arg("a").arg("b").arg("arr").done();
  } c.endBlock();

  c.block("after_foo").begin(); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo").param("p0", ts::i8()).param("p1", ts::i16()).param("p2", array3).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    c.writeOut("p0");
    c.writeOut("p1");
    c.writeOut("p2");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
