// Pass a pointer to a global variable to a function that outputs its pointee. Then change its value and pass it again.
// Expected: "GHX"

TEST_BEGIN

types::TypeHandle i8 = ts::i8();
types::TypeHandle i8p = ts::pointer(ts::i8());
  

c.declareGlobal("g", i8);
    
c.function("main").begin(); {
  c.referGlobals({"g"});
  c.declareLocal("pg", i8p);
  c.declareLocal("x", i8);
   
  c.block("entry").begin(); {
    c.assign("pg", c.addressOf("g")); 
    c.assign("g", literal::i8('G'));
    c.assign("x", literal::i8('X'));
    c.callFunction("foo", "after1").arg("pg").done();
  } c.endBlock();

  c.block("after1").begin(); {
    c.assign("g", literal::i8('H'));
        
    c.callFunction("foo", "after2").arg("pg").done();
  } c.endBlock();

  c.block("after2").begin(); {
    c.writeOut("x");
    c.returnFromFunction();
  } c.endBlock();

} c.endFunction();

c.function("foo").param("p", i8p).ret(ts::void_t()).begin(); {
  c.block("entry").begin(); {
    auto pDeref = c.dereferencePointer("p");
    c.writeOut(pDeref);
    c.returnFromFunction();
  } c.endBlock();
} c.endFunction();
    

TEST_END
