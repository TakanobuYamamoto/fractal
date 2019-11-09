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
  system();
  system.me();
  return 0;
}
