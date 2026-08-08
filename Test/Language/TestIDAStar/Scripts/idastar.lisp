; Iterative Deepening A* on a flat W*H grid, 4-connected, unit step cost.
;
; Same algorithm, move ordering and pruning as idastar.rho, so the node counts
; are directly comparable.
;
; search returns -1 for FOUND, otherwise the smallest f it had to reject. That
; textbook formulation removes the need for a mutable global to carry the next
; threshold out of the recursion -- which matters here, because Lisp's set!
; binds in the current scope rather than writing through to an enclosing one.
; State that must survive a call lives in arrays instead, since those are
; references.
;
; The per-move work is split into tryMove rather than nested inside search, to
; keep the nesting shallow. tryMove calls search and search calls tryMove;
; mutual recursion resolves at call time.
;
; Leaves (cost nodes) as a 2-element array; cost is -1 if no path was found.

(define W 20)
(define H 20)
(define WALLX 10)
(define GAPY 6)
(define INF 1000000)

(define STARTX 0)
(define STARTY 0)
(define GOALX 19)
(define GOALY 0)

(define (blocked x y)
  (cond
    ((and (= x STARTX) (= y STARTY)) 0)
    ((and (= x GOALX) (= y GOALY)) 0)
    ((and (= x WALLX) (/= y GAPY)) 1)
    (else 0)))

(define (manhattan x y)
  (define dx (- GOALX x))
  (if (< dx 0) (set! dx (- 0 dx)))
  (define dy (- GOALY y))
  (if (< dy 0) (set! dy (- 0 dy)))
  (+ dx dy))

(define grid (array))
(define i 0)
(while (< i (* W H))
  (push! grid (blocked (mod i W) (/ i W)))
  (set! i (+ i 1)))

; Move order is right, down, up, left -- identical in every implementation.
(define dirx (array 1 0 0 -1))
(define diry (array 0 1 -1 0))

; One move from (x,y) in direction d. Returns -1 if the goal was reached
; through it, otherwise the smallest f rejected beyond it. INF means the move
; was off-grid, blocked, already on the path, or reached no better than before,
; and so contributes nothing to the next threshold.
(define (tryMove x y g threshold onPath bestG stats d)
  (define nx (+ x (nth dirx d)))
  (define ny (+ y (nth diry d)))
  (if (and (>= nx 0) (< nx W) (>= ny 0) (< ny H))
    (begin
      (define nidx (+ (* ny W) nx))
      (if (and (= (nth grid nidx) 0) (= (nth onPath nidx) 0))
        (begin
          (define ng (+ g 1))
          (if (< ng (nth bestG nidx))
            (begin
              (set-nth! bestG nidx ng)
              (search nx ny ng threshold onPath bestG stats))
            INF))
        INF))
    INF))

; Returns -1 when the goal was reached inside the threshold, otherwise the
; smallest f that exceeded it (INF if nothing did).
(define (search x y g threshold onPath bestG stats)
  (set-nth! stats 0 (+ (nth stats 0) 1))
  (define f (+ g (manhattan x y)))
  (cond
    ((> f threshold) f)
    ((and (= x GOALX) (= y GOALY)) -1)
    (else
      (begin
        (define idx (+ (* y W) x))
        (set-nth! onPath idx 1)
        (define minExceeded INF)
        (define found 0)
        (define d 0)
        (while (< d 4)
          (if (= found 0)
            (begin
              (define r (tryMove x y g threshold onPath bestG stats d))
              (if (= r -1)
                (set! found 1)
                (if (< r minExceeded) (set! minExceeded r)))))
          (set! d (+ d 1)))
        (set-nth! onPath idx 0)
        (if (= found 1) -1 minExceeded)))))

(define stats (array 0))
(define threshold (manhattan STARTX STARTY))
(define cost -1)
(define searching 1)

(while (= searching 1)
  (define onPath (array))
  (define bestG (array))
  (define j 0)
  (while (< j (* W H))
    (push! onPath 0)
    (push! bestG INF)
    (set! j (+ j 1)))
  (set-nth! bestG (+ (* STARTY W) STARTX) 0)

  (define r (search STARTX STARTY 0 threshold onPath bestG stats))
  (if (= r -1)
    (begin (set! cost threshold) (set! searching 0))
    (if (>= r INF)
      (set! searching 0)
      (set! threshold r))))

(array cost (nth stats 0))
