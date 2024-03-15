variable  counter
variable  idx
0 counter !
0 idx !

: _main
    ." Press space to exit program: "
    begin
      key
      dup 32 = if leave then
      dup 97 = if ." Key A pressed" leave then
      dup -1 = if ." EOF" leave then
      dup 255 = if ." EOF" leave then
      ." "
      .
      cr
    again
    cr
;

\ ." No _main call due to interacvtive test" cr
_main
