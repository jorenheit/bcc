// Pass a pointer to a global variable to a function that outputs its pointee. Then change its value and pass it again.
// Expected: "GHX"

TEST_BEGIN

types::TypeHandle i8 = TypeSystem::i8();
types::TypeHandle i8p = TypeSystem::pointer(TypeSystem::i8());
  

c.declareGlobal("g", i8);
    
c.beginFunction("main"); {
  c.referGlobals({"g"});
  c.declareLocal("pg", i8p);
  c.declareLocal("x", i8);
   
  c.beginBlock("entry"); {
    c.assign("pg", values::pointer(i8, "g"));
    c.assign("g", values::i8('G'));
    c.assign("x", values::i8('X'));
    c.callFunction("foo", "after1", "pg");
  } c.endBlock();

  c.beginBlock("after1"); {
    c.assign("g", values::i8('H'));
    c.callFunction("foo", "after2", "pg");
  } c.endBlock();

  c.beginBlock("after2"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();

} c.endFunction();

c.beginFunction("foo", TypeSystem::voidT(), "p", i8p); {
  c.beginBlock("entry"); {
    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref);
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
    

TEST_END
