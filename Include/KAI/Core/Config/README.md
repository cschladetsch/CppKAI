# KAI Config

Configuration of the system. Defaults are sane, but can overridden:

* KAI_USE_NAMESPACES. Turn this off and KAI tries to make thing work without using namespaces. Hey, it could still work. But I haven't tried it in years. YMMV. And if you still don't want namespaces, you probably don't want KAI in general either.

* KAI_NAMESPACE_NAME. The name that KAI lives in. Defaults to **kai**. Yes, you can really change the namespace of the entire library by changing this. Try it! There's no hard-coded references to kai:: anywhere in the source. So you can put it it whatever (nested!) namespace you want.

* KAI_DEBUG. Adds some debugging features. Like, being able to see object values indirectly via a debugger. Otherwise, KAI objects are just unique id's into their Registry so it makes it hard to see what is going on. Turn this on until you want to release or do some performance testing.

* KAI_DEBUG_TRACE. Set trace level. Defaults to 0. 9 is very noisy.

* KAI_ENABLE_EXCEPTIONS. It defauts to ON, so if you do nothing you're gold. However, If you really don't want exceptions, turn this off. I haven't tried a build with this unset for 6 years, but you will be able to get a build that doesn't use exceptions pretty easily if you turn this off. It may take some edits. If you get it working, send me a pull request. I simply haven't tried because I want exceptions in general.
