# shelly
This is my simple command line interpreter for Linux. Im writing it for self-education purposes.

## Features:
* passes-style code
* flexible parser that is easy to improve
* redirect stream (stdin, stdout)
* conveyor |

## Builtin commands:
* cd
* help
* exit

## Wish list:
* bg, fg
* regex (*, ?)
* better promt
* separete passes by functions (in process)
* refract terrible code at conveyor and execute conveyor passes
* improve parser
* conveyor &&
* "conveyor" ;
* environment variables
* redirect stream (stderr)
* Ctrl + C
* Ctrl + D

## How to use it?
There is two version of programm: normal and instrumented with address-sanitizer.
Normal version compiled by gcc, instrumented version compiled by clang.

* Build normal ELF:
```
$ make shelly
```
* Execute ELF:
```
$ ./shelly
```
* Build instrumented ELF:
```
$ export ASAN=1
$ make shelly
```