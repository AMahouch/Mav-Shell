# Mav Shell (msh)

Bash-like shell in C using ```fork``` and ```exec``` system calls.

* Supports and executes any command in /bin/, /usr/bin/, and /usr/local/bin/.
* Implemented with ```fork()```, ```wait()```, and ```execvp()```.

In addition to supporting and executing any command located in the /bin/, /usr/bin/, and /usr/local/bin/ directories, this shell implementation also includes custom commands, which provide additional functionalities shown below:

### Additional Commands
|Command|Description|
|-------|-----------|
|```cd <directory>```|Changes the current working directory to the specified directory|
|```history```|Lists the last 15 commands entered by the user. Supports ```-p``` parameter that lists associated PID with command|
|```!n```|Reruns the nth command stored in history|

### Demo
![Screen Shot 2023-05-18 at 1 06 13 PM](https://github.com/AMahouch/Mav-Shell/assets/97981126/9ddeaad9-d36c-45be-b1e7-98575dbdc59b)



![Screen Shot 2023-05-18 at 1 08 56 PM](https://github.com/AMahouch/Mav-Shell/assets/97981126/45b4454f-c282-4acd-bd93-f02356eb05a9)


### Run
Ensure in parent directory. Build with ```make``` and run with ```./msh```.
