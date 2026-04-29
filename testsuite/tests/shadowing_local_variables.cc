// Tests shadowing of local variables in enclosed nested scopes
// Expected: "ABXCDA"

TEST_BEGIN
c.function("main").begin(); {
  c.declareLocal("x", ts::i8());

  c.block("entry").begin(); {
    c.assign("x", literal::i8('A'));
    c.writeOut("x");
    c.setNextBlock("main", "next");
  } c.endBlock();

  c.scope().begin(); {
    c.declareLocal("x", ts::array(ts::i8(), 2));
    c.block("next").begin(); {
      c.assign("x", literal::array(ts::array(ts::i8(), 2)).push(literal::i8('B')).push(literal::i8('X')).done());
      c.writeOut("x");
      c.setNextBlock("main", "next2");
    } c.endBlock();

    c.scope().begin(); {
      c.declareLocal("x", ts::i8());
      c.declareLocal("y", ts::i8());	
      c.block("next2").begin(); {
	c.assign("x", literal::i8('C'));
	c.assign("y", literal::i8('D'));
	c.writeOut("x");
	c.writeOut("y");	  
	c.setNextBlock("main", "last");
      } c.endBlock();
    } c.endScope();
      
  } c.endScope();

      
  c.block("last").begin(); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();
    
} c.endFunction();

TEST_END
