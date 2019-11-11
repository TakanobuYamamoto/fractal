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
    virtual void recieve(void) = 0;
    virtual void send(void) = 0;
  };

  class baggage_admin{
  public:
    static baggage_admin *buf;
    std::vector<baggage_component *> baggage_list;

    baggage_admin(void);
    void recieve(void);
    void send(void);
    void addBaggage(baggage_component *ptr);
  };



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
      void lock(){mtx.lock();}
      void unlock(){mtx.unlock();}
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
    std::string name(void){
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
    T& operator()(){
      return data;
    }

    //! @brief set data
    T& operator()(const T &d){
      return data = d;
    }

    //! @brief set data
    T& operator()(T &&d){
      return data = d;
    }

    auto& operator[](int n){
      return data[n];
    }

    //! @brief cast
    operator T () {return data;}
    //! @brief set data
    T operator = (const auto &d){(*this)(d); return d;}
    //! @brief set data
    T operator = (auto &&d){(*this)(d); return d;}

    auto operator + (baggage &d){return data+d();}
    auto operator - (baggage &d){return data-d();}
    auto operator * (baggage &d){return data*d();}
    auto operator / (baggage &d){return data/d();}
    auto operator += (baggage &d){return data+=d();}
    auto operator -= (baggage &d){return data-=d();}
    auto operator *= (baggage &d){return data*=d();}
    auto operator /= (baggage &d){return data/=d();}

    auto operator + (const auto &d){return data+d;}
    auto operator - (const auto &d){return data-d;}
    auto operator * (const auto &d){return data*d;}
    auto operator / (const auto &d){return data/d;}
    auto operator + (auto &&d){return data+d;}
    auto operator - (auto &&d){return data-d;}
    auto operator * (auto &&d){return data*d;}
    auto operator / (auto &&d){return data/d;}

    auto operator += (const auto &d){return data+=d;}
    auto operator -= (const auto &d){return data-=d;}
    auto operator *= (const auto &d){return data*=d;}
    auto operator /= (const auto &d){return data/=d;}
    auto operator += (auto &&d){return data+=d;}
    auto operator -= (auto &&d){return data-=d;}
    auto operator *= (auto &&d){return data*=d;}
    auto operator /= (auto &&d){return data/=d;}

    auto operator ++ (){return data++;}
    auto operator -- (){return data--;}

    bool operator == (const T &d){return data==d;}
    bool operator != (const T &d){return data!=d;}
    bool operator < (const T &d){return data<d;}
    bool operator > (const T &d){return data>d;}
    bool operator <= (const T &d){return data<=d;}
    bool operator >= (const T &d){return data>=d;}

    bool operator == (T &&d){return data==d;}
    bool operator != (T &&d){return data!=d;}
    bool operator < (T &&d){return data<d;}
    bool operator > (T &&d){return data>d;}
    bool operator <= (T &&d){return data<=d;}
    bool operator >= (T &&d){return data>=d;}

    //! @brief link data
    baggage<T>& operator >> (baggage<T>& b){
      b.recieve_ptr.reset();
      b.recieve_ptr = send_ptr;
      return *this;
    }

  };



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
    bool is_enabled           = true;  //!< status of this module

    Module(void);

    /**
     * @brief update function
     * @param dt delta time
     * @details redefine in derived classes
     */
    virtual void update(double dt) = 0;

    //! @brief　disable this module
    virtual void disable(void){
      is_enabled = false;
      is_exit_message = false;
    }

    //! @brief check exit message
    virtual void check(void){
      if( is_exit_message && is_enabled )
        disable();
    }


  public:
    baggage<double> t = 0;        //!< synchronization time
    baggage<std::string> message; //!< message

    std::chrono::system_clock::time_point now(void);
    void debugTimeView(double dt);
    void setSleepTime(double dt);
    void sleep(double dt);
    void updateOnce(bool parallel_mode = false);
    void operator ()(void);

    /**
     * @brief get class name of myself
     * @return class name
     */
    std::string name(void);

    /**
     * @brief standard output any string
     * @param str character string
     */
    void say(std::string str);

    /**
     * @brief show class name
     * @param n layer number
     */
    virtual void me(int n){
      std::cerr << std::string(n,' ') << "|-" << name() << std::endl;
    }

    //! @brief exit all modules
    void exitAll(void);

    //! @brief exit this module
    void exit(void);

    /**
     * @brief exit flag for all modules
     * @retrun true if there is exit message
     */
    bool isAllExitMessage(void);

    /**
     * @brief exit flag for this module
     * @retrun true if there is exit message
     */
    bool isExitMessage(void);

    /**
     * @brief status of this module
     * @retrun true if this module is enable
     */
    bool isEnabled(void);
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
      if( this->is_enabled == false ) return;
      for( Module *m : modules ) m->exit();
      for( std::thread &t : threads ) t.join();
      this->is_enabled = false;
      this->is_exit_message = false;
    }

  protected:
    //! @brief　check exit message
    virtual void check(){
      if( this->is_enabled == false ) return;
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
    void me();
    void parallelMode(void);
    void create();
    void join();

    virtual void update(double dt){
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

    template <class... Args>
      System(Args&... args){
      setSleepTime(-1);
      push(args...);
    }

    System(void);
  };

}




#endif
