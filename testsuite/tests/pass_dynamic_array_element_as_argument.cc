// Pass a runtime-selected array element as a function argument and print it inside the callee.
// Expected: "C"

TEST_BEGIN

auto array4 = ts::array(ts::i8(), 4);

c.beginFunction("main"); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", ts::i8());

  c.beginBlock("entry"); {
    c.assign("arr", literal::array(ts::array(ts::i8(), 4)).push(literal::i8('A')).push(literal::i8('B')).push(literal::i8('C')).push(literal::i8('D')).done());
    c.assign("idx", literal::i8(2));

    auto elem = c.arrayElement("arr", "idx");
    c.callFunction("printChar", "return").arg(elem).done();
  } c.endBlock();

  c.beginBlock("return"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = ts::function().ret(ts::void_t()).param(ts::i8()).done();
c.beginFunction("printChar", sig, {"ch"}); {
  c.beginBlock("entry"); {
    c.writeOut("ch");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
