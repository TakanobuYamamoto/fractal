#include <fractal.h>
#include <atomic>

using namespace std;
using namespace fractal;

class WriteA : public Module{
public:
  baggage<string> text = "WriteA";

  virtual void update(double dt){
    text = "aaaaaa";
  }
};

class WriteB : public Module{
public:
  baggage<string> text = "WriteB";

  virtual void update(double dt){
    text = "bbbbbb";
  }
};

class View : public Module{
public:
  baggage<string> text = "View";

  virtual void update(double dt){
    stringstream ss;
    ss.precision(5);
    ss << text() << ": " << dt << endl;
    cout << ss.str();
    if(t() > 1.0) exitAll();
  }
};

int main(void){
  System s;

  WriteA wa;
  WriteB wb;
  View va, vb;
  wa.text >> va.text;
  wb.text >> vb.text;
  s.push(&wa);
  s.push(&wb);
  s.push(&va);
  s.push(&vb);

  s.start();
  s.me();
  return 0;
}
