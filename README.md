# ASCII Cooperative Tetris server and client

To compile the tetris server, use:

    make

It will output a binary file Tetris that is can be run with

Tetris PORTNUMBER

This will start a tetris game session that clients can join.

To compile the tetris client, use:

	gcc -g tclient.c -lnsl -o tclient

It will output a binary file tclient that can be run with

tclient HOSTIP HOSTPORT

This will connect to an active tetris game session at HOSTIP HOSTPORT.

Everyone sees the same board and acts on the same board at the same time
A maximum of 5 players are allowed to connect at once.
Each player is assigned a player number (0-4) when they join. The host is always 0.
Each player's pieces correspond to their player number. Any client can join or leave 
without adversely affecting the running of their game provided they are within the player limit.
The host cannot leave without shutting down the game session for everyone.

Example of what the game looks like with 1 client connected: 

[LEVEL: 1 | SCORE: 0]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . 0 0 . . . . . . . . . . . . . . !
!. . . . . . . . . 0 . . . . . . . . . . . . . . . !
!. . . . . . . . . 0 . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . 1 1 . . . . . . !
!. . . . . . . . . . . . . . . . . 1 1 . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!. . . . . . . . . . . . . . . . . . . . . . . . . !
!0 . . . . . 1 . . . . . . . . . . . . . . . . . . !
!0 . . . 0 1 1 . 0 0 0 . . . . . . . . . . . . . . !
!0 0 0 0 0 1 0 0 0 0 0 . . . . . . . . . . . . . . !
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Controls (for host and clients):

    w: 		rotate
    a: 		move left
    s: 		fast fall
    d: 		move right

