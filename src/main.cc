#include <iostream>
#include "compiler.h"

#define main21 main

int main1() { // AF
  Compiler c;
  types::TypeSystem &ts = c.typeSystem();
  c.setEntryPoint("main");
    
  c.begin(); {
    c.declareGlobal("g", ts.i8());

    c.beginFunction("main"); {
      c.declareLocal("x", ts.i8());
      c.referGlobals({"g"});

      c.beginBlock("entry"); {
	c.assignConst("g", 'A');
	c.callFunction("foo", "after");
      } c.endBlock();

      c.beginBlock("after"); {
	c.writeOut("g");          // should print 'F'
	c.assignConst("x", '\n');
	c.writeOut("x");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo"); {
      c.referGlobals({"g"});

      c.beginBlock("entry"); {
	c.writeOut("g");          // should print 'A'
	c.assignConst("g", 'F');  // modify global shadow
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  
  } c.end();
  
  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}


int main2() { // A1B2ab12...
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.declareGlobal("g1", ts.i8());
    c.declareGlobal("g2", ts.i8());
    c.declareGlobal("g3", ts.i8());
    c.declareGlobal("g4", ts.i8());

    c.beginFunction("main"); {
      c.referGlobals({"g1", "g2", "g3", "g4"});

      c.beginBlock("entry"); {
	c.assignConst("g1", 'A');
	c.assignConst("g2", 'B');
	c.assignConst("g3", '1');
	c.assignConst("g4", '2');
	c.callFunction("alpha", "after");
      } c.endBlock();

      c.beginBlock("after"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("alpha"); {
      c.referGlobals({"g1", "g3"});

      c.beginBlock("entry"); {
	c.writeOut("g1");   // A
	c.writeOut("g3");   // 1

	c.assignConst("g1", 'a');
	c.assignConst("g3", 'b');

	c.callFunction("beta", "after");
      } c.endBlock();

      c.beginBlock("after"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("beta"); {
      c.referGlobals({"g2", "g4"});

      c.beginBlock("entry"); {
	c.writeOut("g2");   // B
	c.writeOut("g4");   // 2

	c.assignConst("g2", '1');
	c.assignConst("g4", '2');

	c.callFunction("gamma", "after");
      } c.endBlock();

      c.beginBlock("after"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("gamma"); {
      c.referGlobals({"g1", "g2", "g3", "g4"});

      c.beginBlock("entry"); {
	c.writeOut("g1");   // a
	c.writeOut("g3");   // b
	c.writeOut("g2");   // 1
	c.writeOut("g4");   // 2

	c.assignConst("g1", 'A');
	c.assignConst("g2", 'B');
	c.assignConst("g3", '1');
	c.assignConst("g4", '2');

	c.callFunction("alpha", "after");
      } c.endBlock();

      c.beginBlock("after"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

  } c.end();
  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}


int main3() { // X
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {

    // main: void
    c.beginFunction("main"); {
      c.declareLocal("x", ts.i8());

      c.beginBlock("entry"); {
        c.callFunction("foo", "after_foo", "x");
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.writeOut("x");
        c.returnFromFunction();
      } c.endBlock();

    } c.endFunction();


    // foo: returns i8
    c.beginFunction("foo", ts.i8()); {

      c.beginBlock("entry"); {
        c.returnConstFromFunction('X');
      } c.endBlock();

    } c.endFunction();

  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}

int main4() { // XY
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {

    c.beginFunction("main"); {
      c.declareLocal("x", ts.i8());

      c.beginBlock("entry"); {
	c.assignConst("x", 'X');
	c.writeOut("x");
        c.callFunction("foo", "after_foo", "x");
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.writeOut("x");
        c.returnFromFunction();
      } c.endBlock();

    } c.endFunction();


    c.beginFunction("foo", ts.i8()); {
      c.declareLocal("y", ts.i8());

      c.beginBlock("entry"); {
        c.assignConst("y", 'Y');
        c.returnFromFunction("y");
      } c.endBlock();

    } c.endFunction();

  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}

int main5() { // GG
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.declareGlobal("g", ts.i8());

    c.beginFunction("main"); {
      c.declareLocal("x", ts.i8());
      
      c.beginBlock("entry"); {
        c.callFunction("foo", "after_foo", "x");
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.writeOut("x");
	c.callFunction("bar", "after_bar");
      } c.endBlock();

      c.beginBlock("after_bar"); {
        c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();

    c.beginFunction("foo", ts.i8()); {
      c.referGlobals({"g"});
      c.beginBlock("entry"); {
	c.assignConst("g", 'G');
	c.returnFromFunction("g");
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("bar"); {
      c.referGlobals({"g"});
      c.beginBlock("entry"); {
	c.writeOut("g");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
    
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}

int main6() { // YY
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.declareGlobal("g", ts.i8());

    c.beginFunction("main"); {
      c.referGlobals({"g"});
      
      c.beginBlock("entry"); {
        c.callFunction("foo", "after_foo", "g");
      } c.endBlock();

      c.beginBlock("after_foo"); {
	c.writeOut("g");
	c.callFunction("bar", "after_bar");
      } c.endBlock();

      c.beginBlock("after_bar"); {
	c.returnFromFunction();
      } c.endBlock();
      
    } c.endFunction();

    c.beginFunction("foo", ts.i8()); {
      c.declareLocal("y", ts.i8());
      c.beginBlock("entry"); {
        c.assignConst("y", 'Y');
        c.returnFromFunction("y");
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("bar"); {
      c.referGlobals({"g"});
      c.beginBlock("entry"); {
	c.writeOut("g");
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
    
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}

int main7() { // B
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.declareGlobal("g", ts.i8());

    c.beginFunction("main"); {
      c.referGlobals({"g"});
      
      c.beginBlock("entry"); {
        c.callFunction("foo", "after_foo", "g");
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.callFunction("bar", "after_bar", "g");
      } c.endBlock();

      c.beginBlock("after_bar"); {
        c.callFunction("baz", "after_baz");
      } c.endBlock();

      c.beginBlock("after_baz"); {
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", ts.i8()); {
      c.declareLocal("x", ts.i8());
      c.beginBlock("entry"); {
        c.assignConst("x", 'A');
        c.returnFromFunction("x");
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("bar", ts.i8()); {
      c.declareLocal("y", ts.i8());
      c.beginBlock("entry"); {
        c.assignConst("y", 'B');
        c.returnFromFunction("y");
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("baz"); {
      c.referGlobals({"g"});
      c.beginBlock("entry"); {
        c.writeOut("g");
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
    
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}

int main8() { // ABCD
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.declareGlobal("x", ts.array(ts.i8(), 10));
    
    c.beginFunction("main"); {
      c.referGlobals({"x"});
      
      c.beginBlock("entry"); {
	Slot x0 = c.arrayElementConst("x", 0);
	Slot x1 = c.arrayElementConst("x", 1);
	Slot x2 = c.arrayElementConst("x", 2);
	Slot x3 = c.arrayElementConst("x", 3);
	
	c.assignConst(x0, 'A');
	c.assignConst(x1, 'B');
	c.assignConst(x2, 'C');
	c.assignConst(x3, 'D');

	c.callFunction("foo", "after_foo");
      } c.endBlock();

      c.beginBlock("after_foo"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo"); {
      c.referGlobals({"x"});
      
      c.beginBlock("entry"); {
	Slot x0 = c.arrayElementConst("x", 0);
	Slot x1 = c.arrayElementConst("x", 1);
	Slot x2 = c.arrayElementConst("x", 2);
	Slot x3 = c.arrayElementConst("x", 3);
	
	c.writeOut(x0);
	c.writeOut(x1);
	c.writeOut(x2);
	c.writeOut(x3);

	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();
  
  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
  
}

int main9() { // ABCDABCD
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    
    c.beginFunction("main"); {
      c.declareLocal("x", ts.array(ts.i8(), 4));
      
      c.beginBlock("entry"); {
	c.callFunction("foo", "after_foo", "x");
      } c.endBlock();

      c.beginBlock("after_foo"); {
	Slot x0 = c.arrayElementConst("x", 0);
	Slot x1 = c.arrayElementConst("x", 1);
	Slot x2 = c.arrayElementConst("x", 2);
	Slot x3 = c.arrayElementConst("x", 3);
	
	c.writeOut(x0);
	c.writeOut(x1);
	c.writeOut(x2);
	c.writeOut(x3);
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", ts.array(ts.i8(), 4)); {
      c.declareLocal("x", ts.array(ts.i8(), 4));
      
      c.beginBlock("entry"); {
	Slot x0 = c.arrayElementConst("x", 0);
	Slot x1 = c.arrayElementConst("x", 1);
	Slot x2 = c.arrayElementConst("x", 2);
	Slot x3 = c.arrayElementConst("x", 3);

	c.assignConst(x0, 'A');
	c.assignConst(x1, 'B');
	c.assignConst(x2, 'C');
	c.assignConst(x3, 'D');

	c.writeOut(x0);
	c.writeOut(x1);
	c.writeOut(x2);
	c.writeOut(x3);
	
	c.returnFromFunction("x");
      } c.endBlock();
    } c.endFunction();
  } c.end();
  
  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}


#define CAT(c1, c2) (((int)c1) | ((int)(c2 << 8)))

int main10() { // ABCDEFGH
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    
    c.beginFunction("main"); {
      c.declareLocal("x", ts.array(ts.i16(), 4));
      
      c.beginBlock("entry"); {
	c.callFunction("foo", "after_foo", "x");
      } c.endBlock();

      c.beginBlock("after_foo"); {
	c.writeOut("x");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", ts.array(ts.i16(), 4)); {
      c.declareLocal("x", ts.array(ts.i16(), 4));
      
      c.beginBlock("entry"); {
	Slot x0 = c.arrayElementConst("x", 0);
	Slot x1 = c.arrayElementConst("x", 1);
	Slot x2 = c.arrayElementConst("x", 2);
	Slot x3 = c.arrayElementConst("x", 3);

	c.assignConst(x0, CAT('A','B'));
	c.assignConst(x1, CAT('C','D'));
	c.assignConst(x2, CAT('E','F'));
	c.assignConst(x3, CAT('G','H'));

	c.returnFromFunction("x");
      } c.endBlock();
    } c.endFunction();
  } c.end();
  
  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}


int main11() { // AB
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("x", ts.i8());
      c.declareLocal("y", ts.i8());
       
      c.beginBlock("entry"); {
	c.assignConst("x", 'A');
	c.callFunction("foo", "after_foo", {
	    Function::Arg('B'),
	    Function::Arg("x")
	  }, "y");
      } c.endBlock();

      c.beginBlock("after_foo"); {
	c.writeOut("x");
	c.writeOut("y");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", ts.i8(), "arg1", ts.i8(), "arg2", ts.i8()); {
      c.beginBlock("entry"); {
	c.returnFromFunction("arg1");
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";  
  return 0;
}

int main12() { // ABCDABCD
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("x", ts.array(ts.i8(), 4));
       
      c.beginBlock("entry"); {
	Slot x0 = c.arrayElementConst("x", 0);
	Slot x1 = c.arrayElementConst("x", 1);
	Slot x2 = c.arrayElementConst("x", 2);
	Slot x3 = c.arrayElementConst("x", 3);

	c.assignConst(x0, 'A');
	c.assignConst(x1, 'B');
	c.assignConst(x2, 'C');
	c.assignConst(x3, 'D');

	c.writeOut("x");
	c.callFunction("foo", "after_foo", {
	    Function::Arg("x")
	  });
      } c.endBlock();

      c.beginBlock("after_foo"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", ts.voidT(), "x", ts.array(ts.i8(), 4)); {
      c.beginBlock("entry"); {
	c.writeOut("x");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";  
  return 0;
}


int main13() { // GHG
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.declareGlobal("g", ts.i8());
    
    c.beginFunction("main"); {
      c.referGlobals({"g"});
      
      c.beginBlock("entry"); {
	c.assignConst("g", 'G');
	c.callFunction("foo", "after_foo", {
	    Function::Arg("g")
	  });
      } c.endBlock();

      c.beginBlock("after_foo"); {
	c.writeOut("g");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", ts.voidT(), "arg1", ts.i8()); {
      c.beginBlock("entry"); {
	c.writeOut("arg1");
	c.assignConst("arg1", 'H');
	c.writeOut("arg1");
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";  
  return 0;
}

int main14() { // AB
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("x", ts.i16());
      c.declareLocal("y", ts.i16());

      c.beginBlock("entry"); {
        c.assignConst("x", CAT('C', 'D'));
        c.callFunction("foo", "after_foo", {
            Function::Arg(CAT('A', 'B')),
            Function::Arg("x")
          }, "y");
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.writeOut("y");
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", ts.i16(), "arg1", ts.i16(), "arg2", ts.i16()); {
      c.beginBlock("entry"); {
        c.returnFromFunction("arg1");
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}

int main15() { // ABCDABCD
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("x", ts.array(ts.i16(), 2));

      c.beginBlock("entry"); {
        Slot x0 = c.arrayElementConst("x", 0);
        Slot x1 = c.arrayElementConst("x", 1);

        c.assignConst(x0, CAT('A', 'B'));
        c.assignConst(x1, CAT('C', 'D'));

        c.writeOut("x");
        c.callFunction("foo", "after_foo", {
            Function::Arg("x")
          });
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", ts.voidT(), "x", ts.array(ts.i16(), 2)); {
      c.beginBlock("entry"); {
        c.writeOut("x");
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}

int main16() { // ABCDEF
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("a", ts.i8());
      c.declareLocal("b", ts.i16());
      c.declareLocal("arr", ts.array(ts.i8(), 3));

      c.beginBlock("entry"); {
        Slot arr0 = c.arrayElementConst("arr", 0);
        Slot arr1 = c.arrayElementConst("arr", 1);
        Slot arr2 = c.arrayElementConst("arr", 2);

        c.assignConst("a", 'A');
        c.assignConst("b", CAT('B', 'C'));
        c.assignConst(arr0, 'D');
        c.assignConst(arr1, 'E');
        c.assignConst(arr2, 'F');

        c.callFunction("foo", "after_foo", {
            Function::Arg("a"),
            Function::Arg("b"),
            Function::Arg("arr")
          });
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo",
                    ts.voidT(),
                    "p0", ts.i8(),
                    "p1", ts.i16(),
                    "p2", ts.array(ts.i8(), 3)); {
      c.beginBlock("entry"); {
        c.writeOut("p0");
        c.writeOut("p1");
        c.writeOut("p2");
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}

int main17() { // A
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.beginFunction("main"); {
      c.beginBlock("entry"); {
        c.callFunction("foo", "after_foo", {
            Function::Arg('A')
          });
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", ts.voidT(), "x", ts.i8()); {
      c.beginBlock("entry"); {
        c.callFunction("bar", "after_bar", {
            Function::Arg("x")
          });
      } c.endBlock();

      c.beginBlock("after_bar"); {
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("bar", ts.voidT(), "y", ts.i8()); {
      c.beginBlock("entry"); {
        c.writeOut("y");
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}

int main18() { // ABCABCABCABC...
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.beginFunction("main"); {
      c.beginBlock("entry"); {
        c.callFunction("foo", "after_foo", {
            Function::Arg('A'),
            Function::Arg('B'),
            Function::Arg('C')
          });
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo",
                    ts.voidT(),
                    "x", ts.i8(),
                    "y", ts.i8(),
                    "z", ts.i8()); {
      c.beginBlock("entry"); {
        c.writeOut("x");
        c.writeOut("y");
        c.writeOut("z");

        c.callFunction("foo", "after_recurse", {
            Function::Arg("x"),
            Function::Arg("y"),
            Function::Arg("z")
          });
      } c.endBlock();

      c.beginBlock("after_recurse"); {
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}

int main19() { // AZBY
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("a", ts.i8());
      c.declareLocal("b", ts.i8());

      c.beginBlock("entry"); {
        c.assignConst("a", 'Z');
        c.assignConst("b", 'Y');

        c.callFunction("foo", "after_foo", {
            Function::Arg('A'),
            Function::Arg("a"),
            Function::Arg('B'),
            Function::Arg("b")
          });
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo",
                    ts.voidT(),
                    "p0", ts.i8(),
                    "p1", ts.i8(),
                    "p2", ts.i8(),
                    "p3", ts.i8()); {
      c.beginBlock("entry"); {
        c.writeOut("p0");
        c.writeOut("p1");
        c.writeOut("p2");
        c.writeOut("p3");
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}

int main20() { // Z
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("r", ts.i8());

      c.beginBlock("entry"); {
        c.callFunction("foo", "after_foo", {
            Function::Arg('Z')
          }, "r");
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.writeOut("r");
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", ts.i8(), "x", ts.i8()); {
      c.declareLocal("tmp", ts.i8());

      c.beginBlock("entry"); {
        c.callFunction("bar", "after_bar", {
            Function::Arg("x")
          }, "tmp");
      } c.endBlock();

      c.beginBlock("after_bar"); {
        c.returnFromFunction("tmp");
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("bar", ts.i8(), "y", ts.i8()); {
      c.beginBlock("entry"); {
        c.returnFromFunction("y");
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}

int main21() { // AB
  Compiler c;
  auto &ts = c.typeSystem();
  c.setEntryPoint("main");

  c.begin(); {
    c.beginFunction("main"); {
      c.declareLocal("r", ts.i8());

      c.beginBlock("entry"); {
        c.callFunction("foo", "after_foo", {
            Function::Arg('A')
          }, "r");
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.writeOut("r");
        c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo", ts.i8(), "x", ts.i8()); {
      c.declareLocal("tmp", ts.i8());
      c.declareLocal("keep", ts.i8());

      c.beginBlock("entry"); {
        c.assignConst("keep", 'B');
        c.callFunction("bar", "after_bar", {
            Function::Arg("x")
          }, "tmp");
      } c.endBlock();

      c.beginBlock("after_bar"); {
        c.writeOut("tmp");
        c.returnFromFunction("keep");
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("bar", ts.i8(), "y", ts.i8()); {
      c.beginBlock("entry"); {
        c.returnFromFunction("y");
      } c.endBlock();
    } c.endFunction();
  } c.end();

  std::cout << c.dumpBrainfuck() << "\n";
  return 0;
}
