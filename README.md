# autocom
autocom is used to execute other program and interact with it automatically, it is a light weight implement like 'expect'

# instruction
somrtimes we want to execute a program e.g. 'ssh', but the program need you input some thing, e.g. ssh need you to input 'password', it is not friendly  when we hae many jobs to do, so autocom can help you to enter  somthing automatically. you can do as 
``` autocom -c "ssh username@x.x.x.x" -r "password,ACTION_SEND,yourpassword" ```
here 'password' is prompt by ssh, 'ACTION_SEND' is what action youwant to do when match 'password' e.g. send a string to ssh, 'yourpassword' is password you want to input ```
# files
* defrule.txt defult rules to be load
* autocom.cpp the source code file
