// Return a value from a function into a struct field reached through a dynamic array element.
// Expected: "MNOQ"

TEST_BEGIN

auto point = ts::defineStruct("Point")("x", ts::i8(),
				     "y", ts::i8());
auto pointArray2 = ts::array(point, 2);

c.beginFunction("main"); {
  c.declareLocal("pts", pointArray2);
  c.declareLocal("idx", ts::i8());

  c.beginBlock("entry"); {
    auto p0 = literal::structT(point)(literal::i8('M'), literal::i8('N'));
    auto p1 = literal::structT(point)(literal::i8('O'), literal::i8('P'));
    c.assign("pts", literal::array(point)(p0, p1));

    c.assign("idx", literal::i8(1));

    auto selectedPoint = c.arrayElement("pts", "idx");
    auto dest = c.structField(selectedPoint, "y");
    c.callFunction("makeQ", "after_makeQ", dest)();
  } c.endBlock();

  c.beginBlock("after_makeQ"); {
    c.writeOut("pts");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto sig = ts::function(ts::i8())();
c.beginFunction("makeQ", sig); {
  c.beginBlock("entry"); {
    c.returnFromFunction(literal::i8('Q'));
  } c.endBlock();
} c.endFunction();

TEST_END
