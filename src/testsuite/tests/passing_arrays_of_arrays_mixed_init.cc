Compiler c;
auto &ts = c.typeSystem();

types::TypeHandle array2 = ts.array(ts.i8(), 2);
types::TypeHandle array22 = ts.array(array2, 2);
  
c.setEntryPoint("main");

using namespace types;
c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("B1", ts.i8());
    c.declareLocal("B2", ts.i8());
    c.declareLocal("E", ts.i8());

    c.beginBlock("entry"); {
      auto B = values::value(ts.i8(), 'B');
      auto C = values::value(ts.i8(), 'C');
      c.assign("B1", B);
      c.assign("B2", "B1");
      auto arr0 = values::value(array2, 'A', "B2");
      auto arr1 = values::value(array2, C, 'D');
      auto arrarr = values::value(array22, arr0, arr1);
	
      c.callFunction("foo", "after_foo", { arrarr }, "E");
    } c.endBlock();

    c.beginBlock("after_foo"); {
      c.writeOut("E");
      c.returnFromFunction();
    } c.endBlock();
  } c.endFunction();

  c.beginFunction("foo", ts.i8(), "arr", array22); {
    c.declareLocal("E", ts.i8());
    c.beginBlock("entry"); {
      c.writeOut("arr");
      auto E = values::value(ts.i8(), 'E');
      c.assign("E", E);
	
      c.returnFromFunction("E");
    } c.endBlock();
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();
