// Tests shadowing of local variables in enclosed nested scopes
// Expected: "ABXCDA"

TEST_BEGIN
c.beginFunction("main"); {
  c.declareLocal("x", ts::i8());

  c.beginBlock("entry"); {
    c.assign("x", literal::i8('A'));
    c.writeOut("x");
    c.setNextBlock("main", "next");
  } c.endBlock();

  c.beginScope(); {
    c.declareLocal("x", ts::array(ts::i8(), 2));
    c.beginBlock("next"); {
      c.assign("x", literal::array(ts::i8())(literal::i8('B'),
						    literal::i8('X')));
      c.writeOut("x");
      c.setNextBlock("main", "next2");
    } c.endBlock();

    c.beginScope(); {
      c.declareLocal("x", ts::i8());
      c.declareLocal("y", ts::i8());	
      c.beginBlock("next2"); {
	c.assign("x", literal::i8('C'));
	c.assign("y", literal::i8('D'));
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

TEST_END
