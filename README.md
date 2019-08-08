
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg?style=flat-square)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![HitCount](http://hits.dwyl.io/ilovebl/autocom.svg)](http://hits.dwyl.io/ilovebl/autocom)
# autocom
autocom is used to execute other program and interact with it automatically, it is a light weight implement like 'expect'

# instruction
somrtimes we want to execute a program e.g. 'scp', but the program need you input some thing, e.g. scp need you to input 'password', it is not friendly  when we hae many jobs to do, so autocom can help you to enter  somthing automatically. you can do as 
``` autocom -c "scp fole_from username@x.x.x.x/file_to" -r "password,ACTION_SEND,yourpassword;" ```
here 'password' is prompt by scp, 'ACTION_SEND' is what action you want to do when match string 'password', 'yourpassword' is password you want to input.

currently not suport interactive programs like (ssh) , due to autocom not read from STDIN and send to ssh.
    
# files
* defrule.txt defult rules to be load
* autocom.cpp the source code file
