: print_number 
 	. 
	32 emit
;

: info 
	dup 0 = 
	if 
		." Делить на ноль нельзя!" 
		drop 
		drop 
	else 
		/mod 
		." Частное " 
		print_number
		." Остаток " 
		print_number
	then 
	cr 
;

: _main
	." 7 / 2 = " 7 2 info
	." 7 / 0 = " 7 0 info
	." 16 / 5 = " 16 5 info
	." 9 / 7 = " 9 7 info
	." 133 / 5 = " 133 5 info
;

_main