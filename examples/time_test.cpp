#include <fractal.h>
#include <atomic>

using namespace std;
using namespace fractal;

class A : public Module{
public:
  virtual void update(double dt){
    cout << "A: " << t() << ", " << dt << endl;
  }
};

class B : public Module{
public:
  virtual void update(double dt){
    cout << "B: " << t() << ", " << dt << endl;
    t = t + 1.0;
  }
};

int main(void){
  System s;

  A a;
  B b;
  b.t.reset();
  b.t = 0;
  s.push(&a);
  s.push(&b);

  s.start();
  s.me();
  return 0;
}
