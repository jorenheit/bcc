// Tests returning an i16 array from a callee into a caller local array.
// Expected: ABCDEFGH

Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
    
  c.beginFunction("main"); {
    c.declareLocal("x", ts.array(ts.i16(), 4));
      
    c.beginBlock("entry"); {
      c.callFunction("foo", "after_foo", "x");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.writeOut("x");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.array(ts.i16(), 4)); {
    c.declareLocal("x", ts.array(ts.i16(), 4));
      
    c.beginBlock("entry"); {
      Slot x0 = c.arrayElementConst("x", 0);
      Slot x1 = c.arrayElementConst("x", 1);
      Slot x2 = c.arrayElementConst("x", 2);
      Slot x3 = c.arrayElementConst("x", 3);

      c.assignConst(x0, CAT('A','B'));
      c.assignConst(x1, CAT('C','D'));
      c.assignConst(x2, CAT('E','F'));
      c.assignConst(x3, CAT('G','H'));

      c.returnFromFunction("x");
    } c.endBlock();
  } c.endFunction();
} c.end();
  
return c.dumpBrainfuck();
