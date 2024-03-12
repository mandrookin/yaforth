\ ." sum of 10 and 25 is " 10 25 + . cr

variable index

: _main
	5 index !
	BEGIN index ? ." Good loop "  
	index @ 1 - dup . index ! cr
	0 index @ >  UNTIL
;

_main
