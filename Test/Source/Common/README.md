# Common Test Class Bases

Contains code that is used by many other test suites in the system.

## TestCommon

Class provides a minimalistic *Registry* and *Tree* structure. This is a base class for simple tests that use just a *Registry*.

## TestLangCommon

Provides a more expansive landscape including a *Console* and associated *Executor* and direct access to _Data_ and _Context_ stacks.

*TestLangCommon* is used as a base class for tests that simulate interaction with the user, and or test various languages.

### Note

**TestCommon** and **TestLangCommon** are poorly named. They do not actually perform any tests. Rather, they are base classes for other test suites.
