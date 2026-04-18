// Select an array field from a struct, then write to a runtime-selected element inside that field.
// Expected: "AQCD"

TEST_BEGIN

auto inner = TypeSystem::array(TypeSystem::i8(), 3);
auto holderFields = c.constructFields("tag", TypeSystem::i8(),
				      "data", inner);
auto holder = c.defineStruct("Holder", holderFields);

c.beginFunction("main"); {
  c.declareLocal("h", holder);
  c.declareLocal("idx", TypeSystem::i8());

  c.beginBlock("entry"); {
    auto data = values::array(TypeSystem::i8(), 'B', 'C', 'D');
    c.assign("h", values::structT(holder, values::i8('A'), data));
    c.assign("idx", values::i8(0));

    auto dataField = c.structField("h", "data");
    auto elem = c.arrayElement(dataField, "idx");
    c.assign(elem, values::i8('Q'));

    c.writeOut("h");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
