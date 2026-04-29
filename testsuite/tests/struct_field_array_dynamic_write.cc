// Select an array field from a struct, then write to a runtime-selected element inside that field.
// Expected: "AQCD"

TEST_BEGIN

auto inner = ts::array(ts::i8(), 3);
auto holder = ts::defineStruct("Holder").field("tag", ts::i8()).field("data", inner).done();

c.beginFunction("main"); {
  c.declareLocal("h", holder);
  c.declareLocal("idx", ts::i8());

  c.beginBlock("entry"); {
    auto data = literal::array(ts::array(ts::i8(), 3)).push(literal::i8('B')).push(literal::i8('C')).push(literal::i8('D')).done();
    
    c.assign("h", literal::struct_t(holder).init("tag", literal::i8('A')).init("data", data).done());
    c.assign("idx", literal::i8(0));

    auto dataField = c.structField("h", "data");
    auto elem = c.arrayElement(dataField, "idx");
    c.assign(elem, literal::i8('Q'));

    c.writeOut("h");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
