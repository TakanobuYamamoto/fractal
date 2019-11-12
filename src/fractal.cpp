#include <fractal.h>

namespace fractal{
  baggage_admin *baggage_admin::buf = NULL;

  baggage_admin::baggage_admin(void){
    buf = this;
  }
  void baggage_admin::recieve(void){
    for(auto ptr: baggage_list) ptr->recieve();
  }
  void baggage_admin::send(void){
    for(auto ptr: baggage_list) ptr->send();
  }
  void baggage_admin::addBaggage(baggage_component *ptr){
    baggage_list.push_back(ptr);
  }



  Module::Module(void){
    _t >> t;
    start = now();
  }

  void Module::disable(void){
    is_enabled = false;
    is_exit_message = false;
  }

  void Module::check(void){
    if( is_exit_message && is_enabled )
      disable();
  }

  void Module::debugTimeView(double dt){
    debug_time_view    = true;
    debug_time_view_dt = dt;
  }

  void Module::setSleepTime(double dt){
    sleep_time = dt;
  }

  std::string Module::name(void){
    char *name = abi::__cxa_demangle(typeid(*this).name(),0,0,NULL);
    std::string str(name);
    free(name);
    return str;
  }

  void Module::say(std::string str){
    std::stringstream ss;
    ss << name() << ":$ " << str << std::endl;
    std::cerr << ss.str();
  }

  void Module::me(int n){
    std::cerr << std::string(n,' ') << "|-" << name() << std::endl;
  }

  void Module::exitAll(void){
    is_all_exit_message = true;
  }

  void Module::exit(void){
    is_exit_message = true;
  }

  bool Module::isAllExitMessage(void){
    return is_all_exit_message;
  }

  bool Module::isExitMessage(void){
    return is_exit_message;
  }

  bool Module::isEnabled(void){
    return is_enabled;
  }

  std::chrono::system_clock::time_point Module::now(void){
    return std::chrono::system_clock::now();
  }

  void Module::sleep(double dt){
    std::chrono::duration<float, std::ratio<1, 1>> _dt(dt);
    std::this_thread::sleep_for(_dt);
  }

  void Module::updateOnce(bool parallel_mode){
    check();

    /* for load reduction */
    if( sleep_time > 0 ) sleep(sleep_time);
    else                 std::this_thread::yield();

    /* time measurement */
    std::chrono::duration<double> elapsed = now() - start;
    start = now();

    /* if internal time exceeds synchronization time in parallel mode */
    if( parallel_mode && _t() > t() ){
      recieve();
      return;
    }

    /* update internal time */
    _t = _t + elapsed.count();
    _t.send();
    t.recieve();

    /* calculate delta time */
    double dt;
    if(parallel_mode){
      dt = elapsed.count();
    }else{
      dt = t() - prev_t;
      prev_t = t();
    }

    /* processing time display mode */
    if( debug_time_view && elapsed.count() > debug_time_view_dt )
      say(std::to_string(elapsed.count()));

    /* update */
    recieve();
    update(dt);
    send();
  }

  void Module::operator ()(void){
    for(auto ptr: baggage_list) ptr->admin_name = name();
    say("Hello");
    while(is_enabled){
      updateOnce(true);
    }
    say("Bye");
  }






  void System::disable(){
    if( this->is_enabled == false ) return;
    for( Module *m : modules ) m->exit();
    for( std::thread &t : threads ) t.join();
    this->is_enabled = false;
    this->is_exit_message = false;
  }

  void System::check(){
    if( this->is_enabled == false ) return;
    if( this->is_exit_message ) disable();
    else{
      for( Module *m : modules ){
        if( m->isAllExitMessage() == true ){
          this->is_all_exit_message = true;
          disable();
        }}}
  }

  System::System(void){
    setSleepTime(-1);
  }

  void System::me(int n){
    std::string str = "\n\n";
    if( n > 0 ) str = std::string(n,' ') + "|-";
    std::cerr << str << this->name() << std::endl;
    for( Module *m : modules ) m->me(n+4);
  }

  void System::me(){
    this->me(0);
    std::cerr << std::endl << std::endl;
  }

  void System::parallelMode(void){
    parallel_mode = true;
  }

  void System::create(){
    for( Module *m : modules )
      threads.push_back( std::thread(std::ref(*m)) );
  }

  void System::join(){
    while( this->is_enabled )
      this->check();
  }

  void System::update(double dt){
    if(initialize){
      if(parallel_mode) create();
      else for( Module *m : modules ) m->setSleepTime(-1);
      initialize = false;
    }

    if(!parallel_mode)
      for( Module *m : modules ){
        if(m->isEnabled()) m->updateOnce();
      }
    this->check();
  }

}
