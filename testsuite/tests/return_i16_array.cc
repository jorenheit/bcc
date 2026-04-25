// Tests returning an i16 array from a callee into a caller local array.
// Expected: ABCDEFGH

TEST_BEGIN

auto array4 = TypeSystem::array(TypeSystem::i16(), 4);
    
c.beginFunction("main"); {
  c.declareLocal("x", array4);
      
  c.beginBlock("entry"); {
    c.callFunction("foo", "after_foo", "x")();
  } c.endBlock();

  c.beginBlock("after_foo"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

auto fooSig = TypeSystem::function(array4);
c.beginFunction("foo", fooSig); {
  c.declareLocal("x", array4);
    
  c.beginBlock("entry"); {
    auto x0 = c.arrayElement("x", 0);
    auto x1 = c.arrayElement("x", 1);
    auto x2 = c.arrayElement("x", 2);
    auto x3 = c.arrayElement("x", 3);

    c.assign(x0, values::i16(CAT('A','B')));
    c.assign(x1, values::i16(CAT('C','D')));
    c.assign(x2, values::i16(CAT('E','F')));
    c.assign(x3, values::i16(CAT('G','H')));

    c.returnFromFunction("x");
  } c.endBlock();
} c.endFunction();

TEST_END
