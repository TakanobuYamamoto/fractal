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

#include <cstdarg>


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
    baggage<double> _t        = 0; //!< internal time
    double prev_t             = 0;
    bool debug_time_view      = false;
    double debug_time_view_dt = 0.0;
    double sleep_time         = 0.0005;

  protected:
    bool is_all_exit_message  = false; //!< exit flag for all modules
    bool is_exit_message      = false; //!< exit flag for this modules
    bool is_enable            = true;  //!< status of this module

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
    virtual void disable(void){ is_enable = false; is_exit_message = false; }
    //! @brief check exit message
    virtual void check(void){ if( is_exit_message && is_enable ) disable(); }
  public:
    baggage<double> t = 0;        //!< synchronization time
    baggage<std::string> message; //!< message

    void debugTimeView(double dt){
      debug_time_view    = true;
      debug_time_view_dt = dt;
    }

    void setSleepTime(double dt){
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
    void exitAll(void){ is_all_exit_message = true; }
    //! @brief exit this module
    void exit(void){ is_exit_message = true; }
    /**
     * @brief exit flag for all modules
     * @retrun true if there is exit message
     */
    inline bool isAllExitMessage(void){return is_all_exit_message;}
    /**
     * @brief exit flag for this module
     * @retrun true if there is exit message
     */
    inline bool isExitMessage(void){return is_exit_message;}
    /**
     * @brief status of this module
     * @retrun true if this module is enable
     */
    inline bool isEnable(void){return is_enable;}

    inline std::chrono::system_clock::time_point now(void){ return std::chrono::system_clock::now(); }

    inline void sleep(double dt){
        std::chrono::duration<float, std::ratio<1, 1>> _dt(dt);
        std::this_thread::sleep_for(_dt);
    }

    inline void updateOnce(bool parallel_mode = false){
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

    void operator ()(void){
      for(auto ptr: baggage_list) ptr->admin_name = name();
      say("Hello");
      while(is_enable){
        updateOnce(true);
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
    bool parallel_mode = false;
    bool initialize = true;

    //! @brief disable this system
    virtual void disable(){
      if( this->is_enable == false ) return;
      for( Module *m : modules ) m->exit();
      for( std::thread &t : threads ) t.join();
      this->is_enable = false;
      this->is_exit_message = false;
    }

  protected:
    //! @brief　check exit message
    virtual void check(){
      if( this->is_enable == false ) return;
      if( this->is_exit_message ) disable();
      else{
        for( Module *m : modules ){
          if( m->isAllExitMessage() == true ){
            this->is_all_exit_message = true;
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

    System(void){
      setSleepTime(-1);
    }

    template <class... Args>
    System(Args&... args){
      setSleepTime(-1);
      push(args...);
    }

    void parallelMode(void){
      parallel_mode = true;
    }

    virtual void update(double dt){
      if(initialize){
        if(parallel_mode) create();
        else for( Module *m : modules ) m->setSleepTime(-1);
        initialize = false;
      }

      if(!parallel_mode)
        for( Module *m : modules ){
          if(m->isEnable()) m->updateOnce();
        }
      this->check();
    }

    template <class T, class... Args>
    void push(T& module, Args&... args){
      push(module);
      push(args...);
    }

    template <class T>
    void push(T& module){
      module.send();
      modules.push_back((Module*)&module);
    }

    void create(){
      for( Module *m : modules )
        threads.push_back( std::thread(std::ref(*m)) );
    }

    void join(){
      while( this->is_enable )
        this->check();
    }

  };

}

#endif
