\ Multiplication table

: _main
    ." 10x10 test" cr cr
    11 1 do i . ." : "
         11 1 do 
\	   14 j < if leave then 
 	   i j * .
         loop cr 
    loop cr
;

_main