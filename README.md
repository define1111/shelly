# shelly
Shelly (Оболочечка) is my simple command line interpreter for Linux. Im writing it for self-education purposes.

## Features:
* passes-style code
* flexible parser that is easy to improve
* abstraction command
* abstraction conveyor
* single and double quotes
* redirect stream (stdin, stdout (rewrite/append), stderr (rewrite/append))
* conveyor |
* Ctrl + D
* comment #
* escaping characters

## Builtin commands:
* cd
* mur
* help
* exit

## Wish list:
* abstraction job
* bg, fg
* regex (*, ?)
* better promt
* "conveyor" &&
* "conveyor" ;
* environment variables (and export)
* Ctrl + C
* history

## How to use it?
There is two version of programm: normal and instrumented with address-sanitizer.
Normal version compiled by gcc, instrumented version compiled by clang.

* Build normal ELF:
```
$ unset ASAN
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
