2 constant step
here . cr

: _main 
  10 0 do i . ." : "
     20 0 do 
	14 j < if leave then 
	j i + . 
     step +loop cr 
  loop cr
  3 1 lshift . 1 3 lshift . cr
;

_main \ ." One" cr _main ." Two" cr _main ." Three" cr
