// Tests referenced global array access across a call, including element-wise writes in caller and reads in callee.
// Expected output: ABCD

Compiler c;
auto &ts = c.typeSystem();
c.setEntryPoint("main");

c.begin(); {
  c.declareGlobal("x", TypeSystem::array(TypeSystem::i8(), 10));
    
  c.beginFunction("main"); {
    c.referGlobals({"x"});
      
    c.beginBlock("entry"); {
      Slot x0 = c.arrayElementConst("x", 0);
      Slot x1 = c.arrayElementConst("x", 1);
      Slot x2 = c.arrayElementConst("x", 2);
      Slot x3 = c.arrayElementConst("x", 3);
	
      c.assignConst(x0, 'A');
      c.assignConst(x1, 'B');
      c.assignConst(x2, 'C');
      c.assignConst(x3, 'D');

      c.callFunction("foo", "after_foo");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo"); {
    c.referGlobals({"x"});
      
    c.beginBlock("entry"); {
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
} c.end();
  
return c.dumpBrainfuck();
