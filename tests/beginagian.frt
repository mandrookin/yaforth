variable  counter
variable  idx
0 counter !
0 idx !

: _main
    begin
      ." Press space to exit program: "
      key
      dup 32 = if leave then
      dup 97 = if ." Key A pressed" leave then
      dup -1 = if ." EOF" leave then
      .
      cr
    again
;

." No _main call due to interacvtive test" cr
_main
