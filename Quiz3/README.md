
##### Android App:
To build the android app:

In android studios, open the android folder
`DIRECTORY\android`

Example: To open Quiz2 on android, go to `Quiz2\android`

Then sync gradle files, and then run the app

##### Web App:
To build the web app:

(do note to emsdk install latest as well if you have not)
You need to .\emsdk activate latest in your emsdk folder (cmd) and then run emsdk_env.bat

Then travel to the `DIRECTORY` folder in the same cmd terminal
(Example: to build Quiz2 the DIRECTORY would be Quiz2)

Type `build_web.bat` to build it.
Follow the instructions:
`cd web`
`python -m http.server 8080`

(please have python preinstalled in order to run this)

to view the website head over to
`http://localhost:8080/` to view it


##### Desktop / Windows App:
Go to `DIRECTORY` folder and open a cmd
(Example: to build Quiz2 the DIRECTORY would be Quiz2)

then type `script_build_and_run.bat`
The program will eventually load up and appear on your screen.

#### FOR QUIZ 3'S BONUS CHALLENGES
Holding the buttons for +SPD, -SPD, +ZOOM, -ZOOM will increase/decrease for every frame button is held down
