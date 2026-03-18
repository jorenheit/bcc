#include <iostream>
#include "compiler.h"

#define main7 main

int main1() { // AF
  Compiler c;
  c.setEntryPoint("main");
    
  c.begin(); {
    c.declareGlobal<types::i8>("g");

    c.beginFunction<void>("main"); {
      c.declareLocal<types::i8>("x");
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

    c.beginFunction<void>("foo"); {
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
  c.setEntryPoint("main");

  c.begin(); {
    c.declareGlobal<types::i8>("g1");
    c.declareGlobal<types::i8>("g2");
    c.declareGlobal<types::i8>("g3");
    c.declareGlobal<types::i8>("g4");

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
  c.setEntryPoint("main");

  c.begin(); {

    // main: void
    c.beginFunction<void>("main"); {
      c.declareLocal<types::i8>("x");

      c.beginBlock("entry"); {
        c.callFunction("foo", "after_foo", "x");
      } c.endBlock();

      c.beginBlock("after_foo"); {
        c.writeOut("x");
        c.returnFromFunction();
      } c.endBlock();

    } c.endFunction();


    // foo: returns i8
    c.beginFunction<types::i8>("foo"); {

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
  c.setEntryPoint("main");

  c.begin(); {

    c.beginFunction<void>("main"); {
      c.declareLocal<types::i8>("x");

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


    c.beginFunction<types::i8>("foo"); {
      c.declareLocal<types::i8>("y");

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
  c.setEntryPoint("main");

  c.begin(); {
    c.declareGlobal<types::i8>("g");

    c.beginFunction<void>("main"); {
      c.declareLocal<types::i8>("x");
      
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

    c.beginFunction<types::i8>("foo"); {
      c.referGlobals({"g"});
      c.beginBlock("entry"); {
	c.assignConst("g", 'G');
	c.returnFromFunction("g");
      } c.endBlock();
    } c.endFunction();

    c.beginFunction<void>("bar"); {
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
  c.setEntryPoint("main");

  c.begin(); {
    c.declareGlobal<types::i8>("g");

    c.beginFunction<void>("main"); {
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

    c.beginFunction<types::i8>("foo"); {
      c.declareLocal<types::i8>("y");
      c.beginBlock("entry"); {
        c.assignConst("y", 'Y');
        c.returnFromFunction("y");
      } c.endBlock();
    } c.endFunction();

    c.beginFunction<void>("bar"); {
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
  c.setEntryPoint("main");

  c.begin(); {
    c.declareGlobal<types::i8>("g");

    c.beginFunction<void>("main"); {
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

    c.beginFunction<types::i8>("foo"); {
      c.declareLocal<types::i8>("x");
      c.beginBlock("entry"); {
        c.assignConst("x", 'A');
        c.returnFromFunction("x");
      } c.endBlock();
    } c.endFunction();

    c.beginFunction<types::i8>("bar"); {
      c.declareLocal<types::i8>("y");
      c.beginBlock("entry"); {
        c.assignConst("y", 'B');
        c.returnFromFunction("y");
      } c.endBlock();
    } c.endFunction();

    c.beginFunction<void>("baz"); {
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
