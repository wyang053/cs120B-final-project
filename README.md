# cs120B-final-project

This a classic pong game project.
Rules:
The classic pong game consists of two paddles on opposite sides of the screen. The user is controlling one of the paddles while the computer controls the other paddle. There is a ball that bounces in between the paddles. The ball speeds up and changes angle (as well as direction) when it hits the corner of either paddle. If the ball hits the center of the paddle it continues at the same angle, but reverses direction.  At every round each player has 3 points, if any player misses the pong the user will lose a point. After all 3 points are lost, the player loses and an LED will lid indicating the win of the other player. A reset button can be used to reset the game at anytime.
Controls: The system has 5 buttons for user inputs, which are player1 up, player1 down, player2 up, player2 down and reset. And it has 3 outputs, which are the 2 win indication LEDs and the 8x8 LED matrix.

Technologies and components used:
The main components used in the project are two 8-bit shift registers, one Atmega1284, five buttons, two LEDs, and one 8x8 LED matrix. The code is implemented in C using Atmel Studio 7. The 16 pins of the LED matrix are driven by two shift registers. The two shift registers are connected in chianing mode, so it only needs three pins to be controlled by the Atmega1284.

![Breadboard](https://github.com/wyang053/cs120B-final-project/blob/master/pong_breadboard.jpg)
