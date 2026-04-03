# KAI Detail

While this is probably the area that requires most documentation,
"detail" is where I put things that are not documented.

Basically, __Detail__ means "do not look at or use this otherwise".

That said, there are decade's of work here. Dating from late-model 2008
to early-model 2015, this is where all the 'detail' lies.

I have already done some 'detailing' by using __CallableBase__, rather
than my old CallableBase that use boost::pre_processor.

Anyway. Point is, you shouldn't have to worry about what's in __detail__ unless you are a maintainer.

## Event System

The event system in *Detail/Event.h* is quite old and should one day be replaced with my [better implementation](https://github.com/cschladetsch/CppEvents) on GitHub.
