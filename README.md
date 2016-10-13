# esc - The Extensible Spin Compiler

**Branch `shunting-yard-parser`, which has my first attempt at a new way of doing
this, is much more interesting than branch `master`.  I gave up on that attempt
too, but it is much closer to what I want than branch `master` is.**

**I gave up on this particular attempt at a Spin compiler** (which never got past the
parser stage) long ago.  I am still planning and working on a new attempt.  This
repository currently contains a mess of failed attempts at a sort of
hierarchical parser that first extracted whole Spin blocks and then gave them
individually to other parsers.  This was a bad idea, since it doesn't allow
nested blocks and doesn't interact well with multi-line comments (or #ifdefs)
that contain block headers.  It also made it impossible to embed C inside of
special Spin blocks, since there was be no way to repurpose `{}` as block
delimiters instead of comments inside of C blocks.

I've since made a few attempts at a new parser that's basically a fancy
mid-parse-extensible shunting-yard parser with some modifications.  The idea
looks promising so far, but it's not on Github yet.  The first attempt at such a
parser became a mess.  It had all sorts of weird features (like dummy infix
operators that got inserted automatically whenever nothing else made sense), the
lexer was mostly hardcoded, and the whole thing had too many classes (which is
part of why I now prefer C over C++).  It can be found in this repository in
branch `shunting-yard-parser`.  The second attempt would have mostly worked but
was too rigidly line-based and I couldn't see a good way to add support for
comments or whitespace in a way that would allow the original source to be
exactly reconstructed from the parse tree (that way, it can be used as a
pre-processor to convert its input to standard Spin).

I'm currently writing a state machine and parser library (similar to my
[vui library](https://github.com/electrodude/vui), but with more focus on
parsing), that I plan to use in most of my C projects from now on (in fact, I'll
probably even rewrite vui to use it, since there's a huge amount of overlap of
functionality between the two libraries).  My next attempt at a Spin compiler
will most likely use my new parser library as its parser system.

When I finally get an attempt to the point where it looks like it might actually
get somewhere, it is possible that I will rename this repository to esc-old or
something and reuse the URL
[http://github.com/electrodude/esc](http://github.com/electrodude/esc) for the
new version.

---

Does not work yet.

A new, open-source, modular, mid-compile-extensible Spin compiler written in C++.
