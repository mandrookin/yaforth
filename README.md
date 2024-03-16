# Yet Another Forth

Here is some view from C++ to Forth and his name is the yaforth.

```bash
make
make check
```

This is Forth language source code.  
It builds the `yaforth` executable for Windows or Linux.  
Source code written in C++.  

`yaforth` has two functions - interpretation and compilation.  
`yaforth` has two modes - interactive and batch.  

Type `./yaforth` and press enter to go interactive mode.  
Type `bye` and press enter to exit.  
Type `./yaforth tests/hello.frt` to see hello-world test.
Type `./yaforth -a tests/hello.frt` to generate `test/hello.asm`.  

### Command line options ###

| Option       | Description                                     |
| ------------ | ----------------------------------------------- |
|-a            | compile Forth to Assembler                      |
|-ansi         | enable ANSI colore                              |
|-no-stdin     | close stdin, return EOF on getc()               |

