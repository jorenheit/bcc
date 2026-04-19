// Assign an anonymous i8 value into an array element selected by a runtime index.
// Expected: "ABZD"

TEST_BEGIN

auto array4 = TypeSystem::array(TypeSystem::i8(), 4);

c.beginFunction("main"); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("arr", values::array(TypeSystem::i8(),
				  values::i8('A'),
				  values::i8('B'),
				  values::i8('C'),
				  values::i8('D')));
    
    c.assign("idx", values::i8(2));

    auto elem = c.arrayElement("arr", "idx");
    c.assign(elem, values::i8('Z'));

    c.writeOut("arr");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
