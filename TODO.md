# Proposed features

## Optimizations
* Dead code/unused method removal
* Common subexpression elimination
* Memory reuse - automatically place a PASM image that's only used once inside a buffer that isn't needed at cog launch time
* Compile-time execution
  * compile time function execution
  * compile time constant expression evaluation
  * compile time execution of most initialization code
* Inlining of PRI methods when it's worth it
* Tunable balance between size and speed
  * perhaps automatic, i.e. if out of RAM then optimize for size more, otherwise optimize for speed

## Basic Language Extensions

* Macro system
  * `(PUB|PRI) macro name(params)`
  * Superpowered methods - can usually assign to constants and such
  * Macros have access to more complex compile-time-only data types and functions like hash tables and automatic strings
  * Can partially be turned into normal methods when it makes sense via something akin to function currying
  * Things like `mov x, somemacro(@foo, 3)` become possible
  * Combined with string mixins, can define new methods
  * Block macros for custom SPIN blocks: (PUB|PRI) block TYP NAM(params, locals, code)
    * params, locals: compile-time lists of names and types of params and locals
    * code: string of block body

  * DAT macros
    * All PASM instructions will actually be macros that emit `long %opcode_ ...` - makes it trivial to add new PASM instructions, even if the instruction format is changed

* Basic type system
  * Only makes compiler warnings by default
  * Infers types where possible
  * Only complains if two types are obviously contradictory
  * A cast, if implemented, would be pretty much equivalent to a permanent acknowledgement of the compiler warning

* Late CON constant value resolution - ability to put pointers and such in constant expressions
  * Example:
```spin
CON
  ptr = @thingamajig

  val = long[ptr]

  val += 7 ' should this be legal? It makes sense for | but might get confusing

  #warnf("val = %d", val") ' printfs 10

DAT
thingamajig long 3
```

* Better local variables - automatic local overlaying for locals that are never used at the same time
  * defined like so: `PUB func(p) : ret | normallocal || autolocal`
  * also `PUB func(p) :: ret` when result can be 

* Smarter lexer (probably integrated into parser)
  * Spin locals can have the same name as PASM reserved words
  * PASM :locals can have same name as Spin reserved words

* Namespaces
  * Named DAT blocks
  * Access child objects' symbols without getter functions and at compile time
  * Should I add visibility modifiers for everything then?
  * Example:
```spin
CON
  lmmcode += lmm.kernel ' dump symbols from DAT lmm.kernel into DAT lmmcode's namespace
OBJ
  lmm : "lmm.spin" ' blockloading LMM kernel
DAT lmmcode
      org start   ' start imported from lmm
code  call #jump  ' jump imported from lmm
```

* Interfaces/Template Types/Object Overlaying
  * We have `serial_implementor.spin` and `serial_interface.spin`.  `serial_interface.spin` has things like `PUB str` that call `PUB tx`, but the body of `tx` is empty.  `serial_implementor.spin` does not necessarily have a `PUB ser`.  `modbus.spin` doesn't require any specific serial interface, so instead of having `OBJ ser : "serial_implementor.spin"` it just has `OBJ ser :: "serial_interface.spin"`  `main.spin` needs to use `modbus.spin`, so it has `OBJ mb : "modbus.spin"`.  However, `modbus.spin` is a parametric class, requiring the parent object to specify a serial implementation, so `main.spin` has to have `OBJ mb.ser : "serial_implementor.spin"`.
  * General syntax: `OBJ name : "thing.spin" : "thing2.spin" : "thing3.spin" ...`
  * Use `::` to leave something unspecified, forcing parent object to specify it
  * An object should be able to optionally declare that it is actually intended to implement/subclass something - good syntax, anyone?
    * this could allow an object to declare a default but overridable implementor of one of its sub-objects

* Optional empty parenthenses after nullary method calls

* `PNUT.dbase`, `PNUT.pbase`, etc., or something to that effect, for access to Spin state variables
  * Allows for coroutines, spaghetti stacks, and other fun


* Operators to get (possibly relative) pointers to or (possibly relative) indices of methods, objects, etc.
  * Self-modifying Spin, to an extent:
```spin
PUB modify
  @a := @b ' sets method table entry for a to point to b instead

  printdec(a) ' prints 2

PUB a
  return 1
PUB b
  return 2
```

## Custom Language Extension Modules

* Mid-compile time loading of optional compiler modules
* Mid-parse parser extension
  * Will probably require custom parser generator, something I'm willing to do
* could this just be done through macros instead?
* custom backends: compile Spin to Tachyon or PASM (or even x86)

## Miscellaneous

* Mode to convert Extended Spin to Parallax Spin
  * probably would generate an unreadable mess, but should still be implemented
* UTF-8 support - almost trivial

* Decompiler

