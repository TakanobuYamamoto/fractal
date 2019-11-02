/**
 * @file fractal.h
 * @brief Parallel System Module
 * @author Takanobu Yamamoto
 * @date 2019
 */
#ifndef _Fractal
#define _Fractal
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <unistd.h>

#include <functional>

#include <typeinfo>
#include <cxxabi.h>
#include <sstream>


namespace fractal{
  constexpr double gravity = 9.80665;

  class baggage_component{
  public:
    std::string admin_name = "not name"; //!< admin class name
    virtual void recieve(void){}
    virtual void send(void){}
  };

  class baggage_admin{
  public:
    static baggage_admin *buf;
    std::vector<baggage_component *> baggage_list;
    baggage_admin(void){
      buf = this;
    }
    inline void recieve(void){
      for(auto ptr: baggage_list) ptr->recieve();
    }
    inline void send(void){
      for(auto ptr: baggage_list) ptr->send();
    }
    inline void addBaggage(baggage_component *ptr){
      baggage_list.push_back(ptr);
    }
  };

  baggage_admin *baggage_admin::buf;



  /**
   * @brief Baggage Class
   * @tparam T type name
   * @details this class automatically performs exclusive processing
   */
  template <class T>
  class baggage : public baggage_component{
  private:
    /**
     * @brief safe data structure
     * @details mutex and data
     */
    struct safe_data{
      std::mutex mtx; //!< mutex class
      T data;         //!< data
      inline void lock(){mtx.lock();}
      inline void unlock(){mtx.unlock();}
    };

    T data;                               //!< data
    std::shared_ptr<safe_data> send_ptr;  //!< send pointer
    std::weak_ptr<safe_data> recieve_ptr; //!< receive pointer

  public:
    baggage() : send_ptr( std::make_shared<safe_data>() ) {
      baggage_admin::buf->addBaggage(this);
    }

    baggage(const auto &v) : send_ptr( std::make_shared<safe_data>() ) {
      data = v;
      send();
      baggage_admin::buf->addBaggage(this);
    }

    baggage(auto &&v) : send_ptr( std::make_shared<safe_data>() ) {
      data = v;
      send();
      baggage_admin::buf->addBaggage(this);
    }

    /**
     * @brief get class name of myself
     * @return class name
     */
    inline std::string name(void){
      char *name = abi::__cxa_demangle(typeid(*this).name(),0,0,NULL);
      std::string str(name);
      free(name);
      return str;
    }

    virtual void recieve(void){
      if( recieve_ptr.expired() ) return;
      recieve_ptr.lock()->lock();
      data = recieve_ptr.lock()->data;
      recieve_ptr.lock()->unlock();
    }

    virtual void send(void){
      send_ptr->lock();
      send_ptr->data = data;
      send_ptr->unlock();
    }

    //! @brief get data
    inline T& operator()(){
      return data;
    }

    //! @brief set data
    inline T& operator()(const T &d){
      return data = d;
    }

    //! @brief set data
    inline T& operator()(T &&d){
      return data = d;
    }

    inline auto& operator[](int n){
      return data[n];
    }

    //! @brief cast
    inline operator T () {return data;}
    //! @brief set data
    inline T operator = (const auto &d){(*this)(d); return d;}
    //! @brief set data
    inline T operator = (auto &&d){(*this)(d); return d;}

    inline auto operator + (baggage &d){return data+d();}
    inline auto operator - (baggage &d){return data-d();}
    inline auto operator * (baggage &d){return data*d();}
    inline auto operator / (baggage &d){return data/d();}
    inline auto operator += (baggage &d){return data+=d();}
    inline auto operator -= (baggage &d){return data-=d();}
    inline auto operator *= (baggage &d){return data*=d();}
    inline auto operator /= (baggage &d){return data/=d();}

    inline auto operator + (const auto &d){return data+d;}
    inline auto operator - (const auto &d){return data-d;}
    inline auto operator * (const auto &d){return data*d;}
    inline auto operator / (const auto &d){return data/d;}
    inline auto operator + (auto &&d){return data+d;}
    inline auto operator - (auto &&d){return data-d;}
    inline auto operator * (auto &&d){return data*d;}
    inline auto operator / (auto &&d){return data/d;}

    inline auto operator += (const auto &d){return data+=d;}
    inline auto operator -= (const auto &d){return data-=d;}
    inline auto operator *= (const auto &d){return data*=d;}
    inline auto operator /= (const auto &d){return data/=d;}
    inline auto operator += (auto &&d){return data+=d;}
    inline auto operator -= (auto &&d){return data-=d;}
    inline auto operator *= (auto &&d){return data*=d;}
    inline auto operator /= (auto &&d){return data/=d;}

    inline auto operator ++ (){return data++;}
    inline auto operator -- (){return data--;}

    inline bool operator == (const T &d){return data==d;}
    inline bool operator != (const T &d){return data!=d;}
    inline bool operator < (const T &d){return data<d;}
    inline bool operator > (const T &d){return data>d;}
    inline bool operator <= (const T &d){return data<=d;}
    inline bool operator >= (const T &d){return data>=d;}

    inline bool operator == (T &&d){return data==d;}
    inline bool operator != (T &&d){return data!=d;}
    inline bool operator < (T &&d){return data<d;}
    inline bool operator > (T &&d){return data>d;}
    inline bool operator <= (T &&d){return data<=d;}
    inline bool operator >= (T &&d){return data>=d;}

    //! @brief link data
    baggage<T>& operator >> (baggage<T>& b){
      b.recieve_ptr.reset();
      b.recieve_ptr = send_ptr;
      return *this;
    }

  };

  template <class T>
  std::ostream& operator << (std::ostream& stream, baggage<T>& value){
    stream << value();
    return stream;
  }

  /**
   * @brief Empty Class
   * @details this class is dummy
   */
  class Dummy{};


  /**
   * @brief Module Class
   * @details base class for modules
   */
  class Module : public baggage_admin{
  private:
    std::chrono::system_clock::time_point start;
    baggage<double> _t         = 0; //!< internal time
    double prev_t              = 0;
    bool _debug_time_view      = false;
    double _debug_time_view_dt = 0.0;
    double sleep_time          = 0.0005;

  protected:
    bool _isAllExitMessage     = false; //!< exit flag for all modules
    bool _isExitMessage        = false; //!< exit flag for this modules
    bool _isEnable             = true;  //!< status of this module

    Module(void){
      _t >> t;
      start = now();
    }

    /**
     * @brief update function
     * @param dt delta time
     * @details redefine in derived classes
     */
    virtual void update(double dt){}

    //! @brief　disable this module
    virtual void disable(void){ _isEnable = false; _isExitMessage = false; }
    //! @brief check exit message
    virtual void check(void){ if( _isExitMessage && _isEnable ) disable(); }
  public:
    baggage<double> t = 0;        //!< synchronization time
    baggage<std::string> message; //!< message

    void debug_time_view(double dt){
      _debug_time_view    = true;
      _debug_time_view_dt = dt;
    }

    void set_sleep_time(double dt){
      sleep_time = dt;
    }

    /**
     * @brief get class name of myself
     * @return class name
     */
    inline std::string name(void){
      char *name = abi::__cxa_demangle(typeid(*this).name(),0,0,NULL);
      std::string str(name);
      free(name);
      return str;
    }
    /**
     * @brief standard output any string
     * @param str character string
     */
    virtual void say(std::string str){
      std::stringstream ss;
      ss << name() << ":$ " << str << std::endl;
      std::cerr << ss.str();
    }
    /**
     * @brief show class name
     * @param n layer number
     */
    virtual void me(int n){
      std::cerr << std::string(n,' ') << "|-" << name() << std::endl;
    }
    //! @brief exit all modules
    void exitAll(void){ _isAllExitMessage = true; }
    //! @brief exit this module
    void exit(void){ _isExitMessage = true; }
    /**
     * @brief exit flag for all modules
     * @retrun true if there is exit message
     */
    inline bool isAllExitMessage(void){return _isAllExitMessage;}
    /**
     * @brief exit flag for this module
     * @retrun true if there is exit message
     */
    inline bool isExitMessage(void){return _isExitMessage;}
    /**
     * @brief status of this module
     * @retrun true if this module is enable
     */
    inline bool isEnable(void){return _isEnable;}

    inline std::chrono::system_clock::time_point now(void){ return std::chrono::system_clock::now(); }

    inline void sleep(double dt){
        std::chrono::duration<float, std::ratio<1, 1>> _dt(dt);
        std::this_thread::sleep_for(_dt);
    }

    inline void update_once(bool parallel_mode = false){
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
      if( _debug_time_view && elapsed.count() > _debug_time_view_dt )
        say(std::to_string(elapsed.count()));

      /* update */
      recieve();
      update(dt);
      send();
    }

    void operator ()(void){
      for(auto ptr: baggage_list) ptr->admin_name = name();
      say("Hello");
      while(_isEnable){
        update_once(true);
      }
      say("Bye");
    }
  };

  /**
   * @brief System Class
   * @details base class for system
   */
  class System : public Module{
  private:
    std::vector<std::thread> threads; //!< thread list
    std::vector<Module *> modules;    //!< module list
    bool _parallel_mode;
    bool initialize = true;

    //! @brief disable this system
    virtual void disable(){
      if( this->_isEnable == false ) return;
      for( Module *m : modules ) m->exit();
      for( std::thread &t : threads ) t.join();
      this->_isEnable = false;
      this->_isExitMessage = false;
    }

  protected:
    //! @brief　check exit message
    virtual void check(){
      if( this->_isEnable == false ) return;
      if( this->_isExitMessage ) disable();
      else{
        for( Module *m : modules ){
          if( m->isAllExitMessage() == true ){
            this->_isAllExitMessage = true;
            disable();
          }}}
    }

  public:
    /**
     * @brief show system layer
     * @param n layer number
     */
    virtual void me(int n){
      std::string str = "\n\n";
      if( n > 0 ) str = std::string(n,' ') + "|-";
      std::cerr << str << this->name() << std::endl;
      for( Module *m : modules ) m->me(n+4);
    }
    //! @brief show system structure
    void me(){ this->me(0); std::cerr << std::endl << std::endl; }

    System(bool parallel_mode = false){
      set_sleep_time(-1);
      _parallel_mode = parallel_mode;
    }

    virtual void update(double dt){
      if(initialize){
        if(_parallel_mode) create();
        else for( Module *m : modules ) m->set_sleep_time(-1);
        initialize = false;
      }

      if(!_parallel_mode)
        for( Module *m : modules ) m->update_once();
      this->check();
    }

    void push(Module *ptr){
      ptr->send();
      modules.push_back( ptr );
    }

    void create(){
      for( Module *m : modules )
        threads.push_back( std::thread(std::ref(*m)) );
    }

    void join(){
      while( this->_isEnable )
        this->check();
    }

  };

}

#endif
