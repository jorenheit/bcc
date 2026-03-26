// Tests passing a nested anonymous array by value to a function
// Expect: ABCD
Compiler c;
c.setEntryPoint("main");

auto array2 = TypeSystem::array(TypeSystem::i8(), 2);
auto array22 = TypeSystem::array(array2, 2);

using namespace types;
c.begin(); {
  c.beginFunction("main"); {
    c.beginBlock("entry"); {
      auto ab = values::array(TypeSystem::i8(), 'A', 'B');
      auto cd = values::array(TypeSystem::i8(), 'C', 'D');
      c.callFunction("foo", "after_foo", values::array(array2, ab, cd));
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", TypeSystem::voidT(),
		  "arr", array22); {
    c.beginBlock("entry"); {
      c.writeOut("arr");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
