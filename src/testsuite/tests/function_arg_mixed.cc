// Tests mixed parameter packing with i8, i16, and i8 array arguments in a single function call.
// Expect: ABCDEF
Compiler c;
c.setEntryPoint("main");

auto array3 = TypeSystem::array(TypeSystem::i8(), 3);

c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("a", TypeSystem::i8());
    c.declareLocal("b", TypeSystem::i16());
    c.declareLocal("arr", array3);

    c.beginBlock("entry"); {
      Slot arr0 = c.arrayElementConst("arr", 0);
      Slot arr1 = c.arrayElementConst("arr", 1);
      Slot arr2 = c.arrayElementConst("arr", 2);

      c.assign("a",  values::i8('A'));
      c.assign("b",  values::i16(CAT('B', 'C')));
      c.assign(arr0, values::i8('D'));
      c.assign(arr1, values::i8('E'));
      c.assign(arr2, values::i8('F'));

      c.callFunction("foo", "after_foo", "a",
		     "b",
		     "arr");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", TypeSystem::voidT(),
		  "p0", TypeSystem::i8(),
		  "p1", TypeSystem::i16(),
		  "p2", array3); {
    c.beginBlock("entry"); {
      c.writeOut("p0");
      c.writeOut("p1");
      c.writeOut("p2");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
