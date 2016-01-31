# D.N.A.H.B GAMES , (a.k.a angry  birds)
Assignment under Graphics Course

Story of motivation:
There's dnahb_ball fired through canon , you need to collect all coins to be called a D.N.A.H.B Guy ;)

Rules and controls:

    To play the game:
        run the file sample2D in terminal , just by typing ./sample2D in terminal.

    To compile the code , run
        g++ -o sample2D Sample_GL3_2D.cpp -lGL -lGLU -lGLEW -lglut -lm

    Controls:

        canon_controls:
            a ==> decrease_canon_rotation
            d ==> increase_canon_rotation

        dnahb_ball's control:
            w ==> increase power of shoot
            s ==> decrease power of shoot
            spacebar/left_mouse_click ==> shoot
        Note: there's a power meter just below canon to show current shooting power

        Screen control:
            arrow_key_UP ==> zoom in
            arrow_key_DOWN ==> zoom out
            arrow_key_LEFT/right_mouse_click ==> panx increnment
            arrow_key_RIGHT ==> panx decrement


    Some twists in game:

        * You only have single life, to complete the game of total 83 levels.
        * there's a trappy box(brown cloured) , if anyhow your dnahb_ball ever get touched by it , you gotta lose you life in a jiff!!
        * all over around there is friction at ground and on blocks.
        * the block at ground has maximum friction.

    Score and levels:

        * your score and current_level will be showed in terminal .
