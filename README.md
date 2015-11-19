# esc - the Extensible Spin Compiler

Not useable yet.

## Status
* Parser has reached the point where it won't die for all inputs I've tried (which isn't terribly extensive), but it still doesn't necessarily output the right thing in some places.

## Planned Features
* Macro assembler
  * PropBasic-like capabilities
  * custom PASM instructions
    * will aid with P1V and P2 development

* Custom Spin operators
  * Write `x .+ y` instead of `f.fadd(x, y)`

* Custom Spin builtins
  * for modified Spin interpreter

* Custom Spin constructs
  * structs by pointer
  * coroutines and other stack abuse

* Optimizations
  * Compile-time function execution
    * Build large structures at compile time, rather than at every boot

* Interface coercion
  * External files that further describe a Spin object without directly modifying the file
  * Many serial objects have the same interface but use different names for their functions.  Simple map files with a few methods for special cases could fix this.
  * Many serial objects use circular buffers that all work the same way.  Interfaces would allow you to easily connect one object's formatting functions to another's output cog.

* Custom Compiler Plugins
  * Include Forth blocks in your Spin program
  * Write Float32Full.ffunc scripts in normal algebraic notation
  * Spin compiler will be written as one of these
