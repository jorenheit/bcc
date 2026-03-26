// Tests returning an i16 array from a callee into a caller local array.
// Expected: ABCDEFGH
using namespace types;
Compiler c;
c.setEntryPoint("main");

auto array4 = TypeSystem::array(TypeSystem::i16(), 4);

c.begin(); {
    
  c.beginFunction("main"); {
    c.declareLocal("x", array4);
      
    c.beginBlock("entry"); {
      c.callFunctionReturn("foo", "after_foo", "x");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.writeOut("x");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", array4); {
    c.declareLocal("x", array4);
    
    c.beginBlock("entry"); {
      Slot x0 = c.arrayElementConst("x", 0);
      Slot x1 = c.arrayElementConst("x", 1);
      Slot x2 = c.arrayElementConst("x", 2);
      Slot x3 = c.arrayElementConst("x", 3);

      c.assign(x0, values::i16(CAT('A','B')));
      c.assign(x1, values::i16(CAT('C','D')));
      c.assign(x2, values::i16(CAT('E','F')));
      c.assign(x3, values::i16(CAT('G','H')));

      c.returnFromFunction("x");
    } c.endBlock();
  } c.endFunction();
} c.end();
  
return c.dumpBrainfuck();
