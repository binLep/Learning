## PL and Static Analysis

PL（Programme languages）

- Theory
    - Language design
    - Type system
    - Semantics and logics
    - ... ...
- Environment
    - Compliers
    - Runtime system
    - ... ...
- Application
    - Program analysis
    - Program verification
    - Program synthesis
    - ... ...

## Why We Learn Static Analysis？

- Program Reliability
    - NULL pointer dereference
    - Memory leak
    - ... ...
- Program Security
    - Private information leak
    - Injection attack
    - ... ...
- Complier Optimization
    - Dead code elimination
    - Code motion
    - ... ...

## Rice's Theorem

> Any non-trivial property of the behavior programes in a r.e. language is undecidable.

r.e. (recursively enumerable) = recognizable by a Turing-machine

- trivial 就是简单的，non-trivial 就是重要的
- recursively enumerable（递归可枚举）
- 该话的意思大致是：如果这个程序是由递归可枚举的现代正常的语言写的程序，它的一些 non-trivial 的 property 都是不可判断的



> A property is trivial if either it is not satisfied by any r.e. language, or if it is satisfied by all r.e. languages; otherwise it is non-trivial.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;non-trivial properties

~= interesting properties

~= the properties related with run-time behaviors of programs



## Static Analysis Features and Examples



## Teaching Plan



## Evaluation Criteria