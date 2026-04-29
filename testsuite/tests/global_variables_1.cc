// Tests global variable access across a call: callee reads and mutates a referenced global, caller observes the updated value.
// Expected: AF

TEST_BEGIN

c.declareGlobal("g", ts::i8());
  
c.function("main").begin(); {
  c.declareLocal("x", ts::i8());
  c.referGlobals({"g"});

  c.block("entry").begin(); {
    c.assign("g", literal::i8('A'));
    c.callFunction("foo", "after").done();
  } c.endBlock();

  c.block("after").begin(); {
    c.writeOut("g");          // should print 'F'
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.function("foo").begin(); {
  c.referGlobals({"g"});

  c.block("entry").begin(); {
    c.writeOut("g");          // should print 'A'
    c.assign("g", literal::i8('F'));  // modify global shadow
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
  

TEST_END

