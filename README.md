# Conceptum
Conceptum is a stack-based, lightweight, Turing-equivalent virtual machine running a small set of bytecodes for benchmarking VM performance.

## Technical details
This is a project still in development. Please refer to our [Wiki](https://github.com/Conceptual-Inertia/Conceptum/wiki) for further details.

## Grammar
Conceptum uses the Polish Notation (PN). Being a stack-based VM Conceptum's grammar is very simple. Everything is coded as
```
<instruction(all lower cases)> <one_single_parameter(optional)>
```
E.g.
```
iadd
```
or
```
cconst c
```
or
```
sconst susan is a fool
```
or
```
iconst 0
```
or
```
fadd
```
etc.

The source code shall be very readable, so please don't hesitate to refer to the source code itself when in doubt :)

## To Contribute
Please refer to [Wiki/About::Contribute](https://github.com/Conceptual-Inertia/Conceptum/wiki/About::Contribute).

## Design Philosophy
Please refer to [Wiki/About::DesignPhilosophy](https://github.com/Conceptual-Inertia/Conceptum/wiki/About::DesignPhilosophy).

## License
GNU General Public License v3.0.

## Author
Alex Fang
