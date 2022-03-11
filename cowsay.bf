>,----------  read first char into (1) making sure it starts with null
>+<           increment char counter
[             start text reading loop
[             start line reading loop
>[>+<-]       move char counter up one place
,----------   read first char into (n)
>+<           increment char counter
]             end line reading loop (decrementing by 10 means newline is 0)
>->           dont count newline char
]             end text reading loop

final layout:
* 0
* $line array
* 0 (newline)
* $length
& 0 (with pointer)
