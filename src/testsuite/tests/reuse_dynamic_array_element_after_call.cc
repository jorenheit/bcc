// Write to a runtime-selected array element, make a function call, then read the same element again.
// Expected: "X"

TEST_BEGIN

auto array4 = TypeSystem::array(TypeSystem::i8(), 4);

c.beginFunction("main"); {
  c.declareLocal("arr", array4);
  c.declareLocal("idx", TypeSystem::i8());
  c.declareLocal("out", TypeSystem::i8());

  c.beginBlock("entry"); {
    c.assign("arr", values::array(TypeSystem::i8(),
				  values::i8('A'),
				  values::i8('B'),
				  values::i8('C'),
				  values::i8('D')));
    c.assign("idx", values::i8(1));

    auto elem = c.arrayElement("arr", "idx");
    c.assign(elem, values::i8('X'));

    c.callFunction("noop", "after_call");
  } c.endBlock();

  c.beginBlock("after_call"); {
    auto elem = c.arrayElement("arr", "idx");
    c.assign("out", elem);
    c.writeOut("out");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("noop"); {
  c.beginBlock("entry"); {
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

TEST_END
