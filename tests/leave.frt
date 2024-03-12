\ This is a test of operator 'leave'

: _main
  ." We have huge test:" cr
  20 0 do  
     10 i  . . ." : " 10 i < dup . if leave else i . cr then \ ) i . cr
  2  +loop cr ;
                                              
." Check condition " 10 0 > . cr 
_main

