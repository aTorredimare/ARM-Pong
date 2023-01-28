# ARM-Pong
Pong for LandTiger-LPC1768 (Keil uVision5)


![image](https://user-images.githubusercontent.com/68816181/156605748-2443499b-49fd-4318-b1fb-307e79b4722d.png)

(LPC1768 simulator for Keil uVision5, kindly provided by teacher and students from Politecnico di Torino)






This project is a simple single-player replica of the iconic Atari videogame "Pong", developed for the course "Computer Architecture" (A.Y. 2021/22).
Because of Covid-related restrictions and limited access to labs and its equipment, the original project was written for an LPC1768 simulator, then adapted to work on the board itself (the main differences are higher/lower timer inizialization intervals and sound frequencies, due to emulation slowness and scaling factor).
It'is almost completely written in C, the libraries for each peripheral were provided by the teachers and are included in the project.
All non-idealities of the peripherals (such as button bouncing and potentiometer non-ideality) are managed via software.




Simple list of buttons/peripheral controls:
  - Potentiometer: left-right movement of the paddle
  - INT0: reset game
  - KEY1: pause game
  - KEY2: resume game
  
  
Note: paddle movement was specifically requested to be controlled with the potentiometer, but it could be easier implemented with the joystick (since it's hardware debounced).

![image](https://user-images.githubusercontent.com/68816181/156605895-afd201da-77c4-4391-9495-160665a12ad8.png)
