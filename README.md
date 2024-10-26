### Running the code

```bash
$ clang++ toselect.cpp
$ ./a.out
```

### Example

Given the expression:
```c++
 "(let ((x 3)) (if (< x 3) 1 2))"
```

the `parser` in `parser.cpp` generates an AST and from this AST `to_anf` in `toanf.cpp` will generate this a normal form:

```c++
(let ((x 3)) (let ((temp_0 (< x 3))) (if temp_0 1 2))) //anf
```

and finally the instructor selector generates this x86:

```c++
{{movq, 3, -8(%rbp)},
 {cmpq, 3, -8(%rbp)}, 
 {setl, %al}, 
 {movzbq, %al, %rsi}, 
 {cmpq, $1, %rsi}, 
 {je, block_16}, 
 {jmp, block_17}, 
 {label, block_16}, 
 {movq, 1, %rdi},
 {callq, print_int}, 
 {label, block_17}, 
 {movq, 2, %rdi}, 
 {callq, print_int}}
```

