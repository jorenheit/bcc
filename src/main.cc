#include <iostream>
#include "compiler.h"

#if 0
int main() {
  Compiler c;
  c.setEntryPoint("main");
    
  c.begin(); {
    auto g = c.declareGlobal<types::i8>("g");

    c.beginFunction("main"); {
      auto x = c.declareLocal<types::i8>("x");
      c.referGlobals({"g"});
      auto gLocal = c.local("g");

      c.beginBlock("entry"); {
	c.assignConst(gLocal, 'A');
	c.callFunction("foo", "after");
      } c.endBlock();

      c.beginBlock("after"); {
	c.writeOut(gLocal);          // should print 'F'
	c.assignConst(x, '\n');
	c.writeOut(x);
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("foo"); {
      auto y = c.declareLocal<types::i8>("y");
      c.referGlobals({"g"});
      auto gLocal = c.local("g");

      c.beginBlock("entry"); {
	c.writeOut(gLocal);          // should print 'A'
	c.assignConst(gLocal, 'F');  // modify global shadow
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

  } c.end();
  
  //  std::cout << c.dumpPrimitives() << "\n\n";
  std::cout << c.dumpBrainfuck() << "\n";
}

#else

int main() {
  Compiler c;
  c.setEntryPoint("main");

  c.begin(); {
    auto g1 = c.declareGlobal<types::i8>("g1");
    auto g2 = c.declareGlobal<types::i8>("g2");
    auto g3 = c.declareGlobal<types::i8>("g3");
    auto g4 = c.declareGlobal<types::i8>("g4");

    c.beginFunction("main"); {
      c.referGlobals({"g1", "g2", "g3", "g4"});
      auto g1Local = c.local("g1");
      auto g2Local = c.local("g2");
      auto g3Local = c.local("g3");
      auto g4Local = c.local("g4");

      c.beginBlock("entry"); {
	c.assignConst(g1Local, 'A');
	c.assignConst(g2Local, 'B');
	c.assignConst(g3Local, '1');
	c.assignConst(g4Local, '2');
	c.callFunction("alpha", "after");
      } c.endBlock();

      c.beginBlock("after"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("alpha"); {
      c.referGlobals({"g1", "g3"});
      auto g1Local = c.local("g1");
      auto g3Local = c.local("g3");

      c.beginBlock("entry"); {
	c.writeOut(g1Local);   // A
	c.writeOut(g3Local);   // 1

	c.assignConst(g1Local, 'a');
	c.assignConst(g3Local, 'b');

	c.callFunction("beta", "after");
      } c.endBlock();

      c.beginBlock("after"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("beta"); {
      c.referGlobals({"g2", "g4"});
      auto g2Local = c.local("g2");
      auto g4Local = c.local("g4");

      c.beginBlock("entry"); {
	c.writeOut(g2Local);   // B
	c.writeOut(g4Local);   // 2

	c.assignConst(g2Local, '1');
	c.assignConst(g4Local, '2');

	c.callFunction("gamma", "after");
      } c.endBlock();

      c.beginBlock("after"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

    c.beginFunction("gamma"); {
      c.referGlobals({"g1", "g2", "g3", "g4"});
      auto g1Local = c.local("g1");
      auto g2Local = c.local("g2");
      auto g3Local = c.local("g3");
      auto g4Local = c.local("g4");

      c.beginBlock("entry"); {
	c.writeOut(g1Local);   // a
	c.writeOut(g3Local);   // b
	c.writeOut(g2Local);   // 1
	c.writeOut(g4Local);   // 2

	c.assignConst(g1Local, 'A');
	c.assignConst(g2Local, 'B');
	c.assignConst(g3Local, '1');
	c.assignConst(g4Local, '2');

	c.callFunction("alpha", "after");
      } c.endBlock();

      c.beginBlock("after"); {
	c.returnFromFunction();
      } c.endBlock();
    } c.endFunction();

  } c.end();
  std::cout << c.dumpBrainfuck() << "\n";
}
#endif
