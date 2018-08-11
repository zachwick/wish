(use-modules (ice-9 ftw))
(use-modules (web client))
(use-modules (web http))
(use-modules (gnutls))
(use-modules (ice-9 receive))
(use-modules (rnrs bytevectors))
(use-modules (json))
(use-modules (ice-9 pretty-print))

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
	(display "The following are defined in `/Users/zwick/wishrc.scm':") (newline)
	(display "help") (newline)))

(define* (ls #:optional n)
	(if (string? n)
			(display (scandir n))
			(display (scandir (getcwd))))
	(newline))

(define (prime? n)
	(let loop ((d 2))
		(cond ((< n (* d d)) #t)
					((zero? (modulo n d))
					 (display d)
					 (display " divides ")
					 (display n)
					 (newline))
					(else (loop (+ d 1))))))

(define (is-prime n)
	(let loop ((d 2))
		(cond ((< n (* d d)) #t)
					((zero? (modulo n d)) #f)
					(else (loop (+ d 1))))))

(define (make-primes n)
  (let ((bits (make-vector (+ n 1) #t)))
    (let loop ((p 2) (ps '()))
      (cond ((< n p) (reverse ps))
            ((vector-ref bits p)
              (do ((i (+ p p) (+ i p))) ((< n i))
                (vector-set! bits i #f))
              (loop (+ p 1) (cons p ps)))
            (else (loop (+ p 1) ps))))))
(define (primes n)
	(display (make-primes n))
	(newline))

(define (sort lst)
	(let loop ((lst lst) (result '()) (n (- (length lst) 1)))
		(cond
		 ((null? (cdr lst))
			(if (zero? n)
					(reverse (cons (car lst) result))
					(loop (reverse(cons (car lst) result)) '() (- n 1))))
		 ((> (car lst) (cadr lst))
			(begin
				(loop (cons (car lst) (cddr lst)) (cons (cadr lst) result) n)))
		 (else
			(begin
				(loop (cdr lst) (cons (car lst) result ) n))))))

;; Fetch Customers
(define (fetch-customers) (receive (response body)
				 (http-get "https://api.stripe.com/v1/customers"
									 #:headers '((authorization . (Basic XXYYZZ)))
									 #:decode-body? #t)
				 (let parsed ((json-string->scm (utf8->string body)))
					 (display (utf8->string body)))))

;; ;; Fetch own Account
(define (fetch-account) (receive (response body)
				 (http-get "https://api.stripe.com/v1/accounts"
									 #:headers '((authorization . (Basic XXYYZZ)))
									 #:decode-body? #t)
				 (let parsed ((json-string->scm (utf8->string body)))
					 (display (utf8->string body)))))

;; Fetch Balance
(define (fetch-balance) (receive (response body)
				 (http-get "https://api.stripe.com/v1/balance"
									 #:headers '((authorization . (Basic XXYYZZ)))
									 #:decode-body? #t)
				 (let parsed ((json-string->scm (utf8->string body)))
					 (display (utf8->string body)))))

;; Create Customer
(define (create-customer) (receive (response body)
				 (http-post "https://api.stripe.com/v1/customers"
										#:headers '((authorization . (Basic XXYYZZ))
																(content-type . (application/x-www-form-urlencoded)))
										#:decode-body? #t)
				 (display (utf8->string body))))
