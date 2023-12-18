# Build Rootkit (Kernel Programming)

## Project Overview

In this project, I implemented a Linux kernel module as well as rootkit simulation that modifies system behavior, demonstrating features such as file redirection and content manipulation.

sneaky_process.c simulates a scenario where an attacker loads the sneaky module into a compromised machine, executes unauthorized actions, such as adding a rogue user, and upon completion of malicious activities, removes the sneaky module. The goal is to demonstrate a potential security threat by manipulating system calls and files, emphasizing the importance of safeguarding against such attacks

<div style="text-align: center;">
  <img src="imgs/3.png" alt="rootkit" width="600" height="400">
</div>

* The sneaky_process.c program copies content in etc/passwd to a temporary file, here I copy over to tmp/passwd.
  
* The sneaky_process.c program adds unauthorized user with root privilge to etc/passwd.
  
* The sneaky_process.c loads the sneaky_module in.
  
* The sneaky module hides the sneaky_processor.c from "ls" and "find" commands run by a compromised user.
  
* The sneaky module also hides the process id of sneaky_process.c from "ls /proc" or "ps".
  
* When user tries to look at etc/passwd, it redirects the request to tmp/passwd, fooling the user.
  
* Lastly, the installment of rootkit (sneaky_module.c) module is hidden from the users. When "lsmod" is run, all other modules except for the sneaky module.


