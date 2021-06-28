# What is this project about
It is an final educational project for the second term of MIPT

1. Unlimited amount of variables (almost)
2. if, while, +, -, *, /, sqrt
3. in, out functions
## Requirments:
1. GraphViz - latest version
2. x86-64 CPU
3. NASM - The Netwide Assembler 
## Build
For buildung this project you need to type
```sh
make
```
It creates executables file
## comp
This executable compile programmes written on my languges and creates ELF file.
There are only 4 ways how you can run this programm:

1.
```sh
./comp input
```
It generetes file with  name "res"

2.
```sh
./comp input output
```
It generetes file with name output

3.
```sh
./comp input -d
```
It generetes file with  name "res" and create dump of AST-tree in file result.pdf


4.
```sh
./comp input output -d
```
It generetes file with  name output and create dump of AST-tree in file result.pdf

You can check some examples of correctly written programms in folder examples. 
