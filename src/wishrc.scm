(define wish_config
  (lambda ()
    (display "Loading wish config") (newline)))
(define wshome '"/home/zwick/")
(define wish_help
	(lambda ()
	(display "(w)ick's (i)nteractive (sh)ell") (newline)
	(display "A minimalist shell for launching programs") (newline)
	(display "The following are built into wish:") (newline)
	(display "cd") (newline)
	(display "exit") (newline) (newline)
	(display "The following are defined in `wishrc.scm':") (newline)
	(display "help") (newline)))
