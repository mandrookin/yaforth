: _main
    begin
      ." Press space to exit program"
      key
      dup 32 = if leave then
      cr
    again
;