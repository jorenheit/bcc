// Tests global variable access across a call: callee reads and mutates a referenced global, caller observes the updated value.
// Expected: AF

TEST_BEGIN

c.declareGlobal("g", TypeSystem::i8());
  
c.beginFunction("main"); {
  c.declareLocal("x", TypeSystem::i8());
  c.referGlobals({"g"});

  c.beginBlock("entry"); {
    c.assign("g", values::i8('A'));
    c.callFunction("foo", "after")();
  } c.endBlock();

  c.beginBlock("after"); {
    c.writeOut("g");          // should print 'F'
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();

c.beginFunction("foo"); {
  c.referGlobals({"g"});

  c.beginBlock("entry"); {
    c.writeOut("g");          // should print 'A'
    c.assign("g", values::i8('F'));  // modify global shadow
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
  

TEST_END

