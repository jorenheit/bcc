// Tests passing an anonymous array by value to a function
// Expect: ABCD
Compiler c;
c.setEntryPoint("main");

auto array4 = TypeSystem::array(TypeSystem::i8(), 4);

c.begin(); {
  c.beginFunction("main"); {
    c.beginBlock("entry"); {
      c.callFunction("foo", "after_foo", values::array(TypeSystem::i8(), 'A', 'B', 'C', 'D'));
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", TypeSystem::voidT(),
		  "arr", array4); {
    c.beginBlock("entry"); {
      c.writeOut("arr");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
