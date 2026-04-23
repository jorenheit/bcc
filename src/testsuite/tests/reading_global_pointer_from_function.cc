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
    c.assign("pg", c.addressOf("g")); 
    c.assign("g", values::i8('G'));
    c.assign("x", values::i8('X'));
    auto args = c.constructFunctionArguments("pg");
    c.callFunction("foo", "after1", args);
  } c.endBlock();

  c.beginBlock("after1"); {
    c.assign("g", values::i8('H'));
    auto args = c.constructFunctionArguments("pg");    
    c.callFunction("foo", "after2", args);
  } c.endBlock();

  c.beginBlock("after2"); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();

} c.endFunction();

auto sig = TypeSystem::function(TypeSystem::voidT(), i8p);
c.beginFunction("foo", sig, {"p"}); {
  c.beginBlock("entry"); {
    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref);
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
    

TEST_END
