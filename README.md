fractal
===============

# About fractal

fractal is a framework for parallel systems. This library contains
fractal::System, fractal::Module, and fractal::baggage that perform
exclusive operations. When you add a module derived from
fractal::Module to fractal::System, the module works. The update
function in each module is then executed repeatedly in parallel. If
you want to exchange information between modules, you can use
fractal:: baggage for security. The link feature of fractal::baggage
sends the information automatically.



# Getting started

Explain how to build a system. The following code creates and
displays a string.

```c++
/************************/
/* example/wav_test.cpp */
/************************/

#include <iostream>
#include <string>
#include <fractal.h>

class Write : public fractal::Module{
public:
  fractal::baggage<std::string> str;

  virtual void update(double dt){
    str = std::to_string((int)t);
  }
};

class Add : public fractal::Module{
public:
  fractal::baggage<std::string> in, out;

  virtual void update(double dt){
    out = in + "TEXT";
  }
};

class View : public fractal::Module{
public:
  fractal::baggage<std::string> str;

  virtual void update(double dt){
    std::cout << str << std::endl;
    if(t > 5.0) exitAll();
  }
};

int main(void){
  Write write;
  Add add;
  View view;

  write.str >> add.in;
  add.out >> view.str;

  fractal::System system(write, add, view);
  system.parallelMode(); // default is serial mode
  system();
  system.me();
  return 0;
}
```

The details of this code are described below.

## fractal::baggage

fractal::baggage is a wrapper for exclusive handling. This class
corresponds to the basic operator and std::ostream. operator () is
used to access the variable directly. It can also be used to assign
values. (When wrapping a complex class such as Eigen, the input/output
by operator () is stable.)

```
fractal::baggage<int> n, m;
int value = n(); // get value
m(value);        // set value
```

In addition, fractal::baggage has linking
capabilities. By using operator >>, you can send the left side
information to the right side.

```
fractal::baggage<int> n, m;
n >> m;
```

## fractal::Module

The module for fractal::System is created based on
fractal::Module. The inherited module class must contain the virtual
function:

```
virtual void update(double dt)
```

fractal::System executes this function automatically and saves the
time width in dt.

exit() sends the exit message to the module, and exitAll() sends the
exit message throughout the system.

The internal time is defined as

```
fractal::baggage<double> t
```

It is automatically updated and freely available.

## fractal::System

Modules are registered by supplying an instance of the module to the
fractal::System constructor argument.

```
fractal::System system(module1, module2, module3, ...);
```

Since the system defaults to serial mode, parallelMode() must be run
to enable parallel mode.

```
system.parallelMode();
```

The system is started by operator ().

```
system();
```