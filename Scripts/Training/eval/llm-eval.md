# LLM Evaluation Cases

case: rho-repair
language: Rho
prompt: repair this broken function declaration: foo fum(a,b) ...
must_contain: foo fun(a, b)
must_contain: transpile to Pi
must_not_contain: fum(

case: pi-stack
language: Pi
prompt: explain the stack effect of: 1 2 +
must_contain: stack
must_contain: [0]
must_contain: 3

case: tau-idl
language: Tau
prompt: repair this interface declaration: interface ICalc { int Add(int a, int b); }
must_contain: interface
must_contain: proxy
must_contain: agent
