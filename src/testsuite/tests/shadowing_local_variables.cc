// Tests shadowing of local variables in enclosed nested scopes
// Expected: "ABXCDA"

Compiler c;
auto &ts = c.typeSystem();
  
c.setEntryPoint("main");

using namespace types;
c.begin(); {
  c.beginFunction("main"); {
    c.declareLocal("x", ts.i8());

    c.beginBlock("entry"); {
      c.assign("x", values::i8(ts, 'A'));
      c.writeOut("x");
      c.setNextBlock("main", "next");
    } c.endBlock();

    c.beginScope(); {
      c.declareLocal("x", ts.array(ts.i8(), 2));
      c.beginBlock("next"); {
	c.assign("x", values::array(ts, ts.i8(), 'B', 'X'));
	c.writeOut("x");
	c.setNextBlock("main", "next2");
      } c.endBlock();

      c.beginScope(); {
	c.declareLocal("x", ts.i8());
	c.declareLocal("y", ts.i8());	
	c.beginBlock("next2"); {
	  c.assign("x", values::i8(ts, 'C'));
	  c.assign("y", values::i8(ts, 'D'));
	  c.writeOut("x");
	  c.writeOut("y");	  
	  c.setNextBlock("main", "last");
	} c.endBlock();
      } c.endScope();
      
    } c.endScope();

      
    c.beginBlock("last"); {
      c.writeOut("x");
      c.returnFromFunction();
    } c.endBlock();
    
  } c.endFunction();
} c.end();

return c.dumpBrainfuck();

