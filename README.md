### Example

```bash
$ clang++ toselect.cpp
$ ./a.out

(let ((x 3)) (let ((temp_0 (< x 3))) (if temp_0 1 2))) //anf

{{movq, 3, -8(%rbp)}, {cmpq, 3, -8(%rbp)}, {setl, %al}, {movzbq, %al, %rsi}, {cmpq, $1, %rsi}, {je, block_16}, {jmp, block_17}, {label, block_16}, {movq, 1, %rdi}, {callq, print_int}, {label, block_17}, {movq, 2, %rdi}, {callq, print_int}}

```

