// Select an array field from a struct, then write to a runtime-selected element inside that field.
// Expected: "AQCD"

TEST_BEGIN

auto inner = ts::array(ts::i8(), 3);
auto holder = ts::defineStruct("Holder")("tag", ts::i8(),
				      "data", inner);

c.beginFunction("main"); {
  c.declareLocal("h", holder);
  c.declareLocal("idx", ts::i8());

  c.beginBlock("entry"); {
    auto data = literal::array(ts::i8())(literal::i8('B'),
						literal::i8('C'),
						literal::i8('D'));
    
    c.assign("h", literal::structT(holder)(literal::i8('A'), data));
    c.assign("idx", literal::i8(0));

    auto dataField = c.structField("h", "data");
    auto elem = c.arrayElement(dataField, "idx");
    c.assign(elem, literal::i8('Q'));

    c.writeOut("h");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
