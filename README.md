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

