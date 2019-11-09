#include <string>
#include <fractal.h>

using namespace std;
using namespace fractal;

class View : public Module{
public:
  string str;
  View(const string s) : str(s) {}
  virtual void update(double dt){
    cout << str;
    if(t > 3.0) exitAll();
  }
};

int main(void){
  View A("A"), B("B"), C("C\n");
  System system(&A, &B, &C);
  system();
  return 0;
}
