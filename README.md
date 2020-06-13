# Execution

To compile and run the program, use the following command:

```shell
flex lang.l && yacc -d lang.y && gcc y.tab.c lex.yy.c compiler.c && a < t-example.java > t-example.mixal
mixbuilder t-example.mixal
```

# Available tools

As proposed in [TAOCP->MIXware](https://www-cs-faculty.stanford.edu/~knuth/taocp.html). The best with floating point support are MIXBuilder and JMixSim.

* [GNU's MIX Development Kit](http://www.gnu.org/software/mdk/mdk.html): No floating point support.
* [JMixSim, an OS-independent assembler and simulator, by Christian Kandeler](http://sourceforge.net/projects/jmixsim): Supports floats, but not float literals.
* [MixIDE, another OS-independent assembler and simulator, by Andrea Tettamanzi](http://mixide.sourceforge.net/): Supports floats, but not float literals. Not very usable.
* [MIXBuilder: an editor, assembler, simulator, and interactive debugger for Win32 platforms, by Bill Menees](http://www.menees.com/): Supports floats, but not float literals.
* [EMIX: an expandable MIX emulator for the Win32 platform, by Daniel Andrade and Marcus Pereira](http://members.tripod.com/~dandrade): From 1998, unusable.
* [MIX/MIXAL in C with lex and CWEB documentation and a source debug facility, by Douglas Laing and Sergey Poznyakoff](http://gray.gnu.org.ua/mix.html): No floating point support.
* [David Smallberg's version that can be compiled for Linux](http://web.archive.org/web/20080801094331/http://swiss.csail.mit.edu/~adler/MIX/): From 1992, no binaries.
* [Dan Taflin's assembler and interactive simulator in HTML and Javascript](http://www.recreationalmath.com/mixal): Not found.
* [Darius Bacon and Eric Raymond's open-source load-and-go assembler and simulator, from The Retrocomputing Museum](http://www.catb.org/~esr/mixal/): Merged to MDK.
* [John R. Ashmun's MIXware for the Be \[Haiku\] operating system, with extended support for interrupts](http://www.bebits.com/app/3723): Not found.
* [Rutger van Bergen's MIX emulator in .NET/C\#](http://rbergen.home.xs4all.nl/mixemul.html): Recent from 2017, but weird floating point module.
* [Chaoji Li's MIX assembler and simulator, in Perl](http://litchie.net/programs/mixsim.html): Found elsewhere, no floating point support.
* [Ruslan Batdalov's MIX emulator, in Scala, which allows execution both forwards and backwards](http://www.mix-emulator.org/): Web based, no floating point support.


* [http://home.telkomsa.net/dlaing/mix.html]: No floating point suppoort.

# Documentation links

* http://www.jklp.org/profession/books/mix/index.html
* https://en.wikipedia.org/wiki/MIX
* https://esolangs.org/wiki/MIX_(Knuth): floats, character set etc

