# Rho language Tests
Rho is much like _Python_, but with an even less verbose syntax.

The 'return value' for a Rho function is whatever is left on the stack(s).

_Rho_ is internally translated to _Pi_ for use in a single _Executor_ that supports continuations natively.

## ContinuationMobilityDemo

The clearest single-file example of the migration story in this tree now lives
at
[Demo/ContinuationMobilityDemo/ContinuationMobilityDemo.rho](../../../../Demo/ContinuationMobilityDemo/ContinuationMobilityDemo.rho).

It is intentionally explicit:

- the script owns the narrative
- the HTML pages are just explanatory views
- the executable model is a reference implementation, not the source of truth
- the script concludes by evaluating the restored-agent count

Example:

```
fun a(b, c)
	b + c

assert(a(1, 2) == 3)

fun d(e)
	fun f(g)
		g*2
	f(e)
  
assert(d(2) == 4)
assert(exists a)
assert(exists d)
assert(!(exists f))

fun h(i,j,k)
	a(i, d(j)*k)

assert(h(1,2,3) == 16)
```
