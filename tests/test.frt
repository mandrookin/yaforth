: func 100 + ; 
1 3 + 2 * func .
3 5 and . cr 0 invert . 2 invert .
3 5 mod . cr ." What's up?" cr 22 4 /mod . .
5 dup + . 7 1 swap . . cr 3 0 over . . .  cr  3 4 5 rot . . .  cr
\ This is comment \\ real comment
1 1 <> if ( Just comment ) ." true" else ." false" then cr
0 5 <> if ." yes" else ." no" then 2 1 > . cr
86 170 xor . cr
variable MY_VAR 3 cells allot
12345 MY_VAR !
33 119 111 87 emit emit emit emit cr
MY_VAR @ ." variable is " . ." is" MY_VAR ? cr

2 constant step

: demo 
  ." Press any key" key . cr
  10 0 do i . ." : "
     20 0 do 10 j < if leave then j . step +loop cr 
  loop cr
  3 1 lshift . 1 3 lshift . cr ;

: _main
  demo ." One" cr demo ." Two" cr demo ." Three" cr
;

_main