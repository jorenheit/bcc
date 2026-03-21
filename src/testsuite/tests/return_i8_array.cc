
// Tests returning an i8 array from a callee into a caller local array.
// Expected: ABCDABCD
using namespace types;

Compiler c;
auto const &ts = c.typeSystem();
c.setEntryPoint("main");

auto array4 = ts.array(ts.i8(), 4);

c.begin(); {
    
  c.beginFunction("main"); {
    c.declareLocal("x", array4);
      
    c.beginBlock("entry"); {
      c.callFunction("foo", "after_foo", "x");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.writeOut(c.arrayElementConst("x", 0));
      c.writeOut(c.arrayElementConst("x", 1));
      c.writeOut(c.arrayElementConst("x", 2));
      c.writeOut(c.arrayElementConst("x", 3));
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

      c.assign(x0, values::value(ts.i8(), 'A'));
      c.assign(x1, values::value(ts.i8(), 'B'));
      c.assign(x2, values::value(ts.i8(), 'C'));
      c.assign(x3, values::value(ts.i8(), 'D'));

      c.writeOut("x");	
      c.returnFromFunction("x");
    } c.endBlock();
  } c.endFunction();
} c.end();
  
return c.dumpBrainfuck();
