
// Tests returning an i8 array from a callee into a caller local array.
// Expected: ABCDABCD

Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
    
  c.beginFunction("main"); {
    c.declareLocal("x", ts.array(ts.i8(), 4));
      
    c.beginBlock("entry"); {
      c.callFunction("foo", "after_foo", "x");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      Slot x0 = c.arrayElementConst("x", 0);
      Slot x1 = c.arrayElementConst("x", 1);
      Slot x2 = c.arrayElementConst("x", 2);
      Slot x3 = c.arrayElementConst("x", 3);
	
      c.writeOut(x0);
      c.writeOut(x1);
      c.writeOut(x2);
      c.writeOut(x3);
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.array(ts.i8(), 4)); {
    c.declareLocal("x", ts.array(ts.i8(), 4));
      
    c.beginBlock("entry"); {
      Slot x0 = c.arrayElementConst("x", 0);
      Slot x1 = c.arrayElementConst("x", 1);
      Slot x2 = c.arrayElementConst("x", 2);
      Slot x3 = c.arrayElementConst("x", 3);

      c.assignConst(x0, 'A');
      c.assignConst(x1, 'B');
      c.assignConst(x2, 'C');
      c.assignConst(x3, 'D');

      c.writeOut(x0);
      c.writeOut(x1);
      c.writeOut(x2);
      c.writeOut(x3);
	
      c.returnFromFunction("x");
    } c.endBlock();
  } c.endFunction();
} c.end();
  
return c.dumpBrainfuck();
