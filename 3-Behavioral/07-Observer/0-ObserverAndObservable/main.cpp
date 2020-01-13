#include <vector>
#include <string>
#include <iostream>
#include <boost/signals2.hpp>
#include <mutex>

using namespace std;
using namespace boost::signals2;

//T is the object to be observed
template <typename T>
class Observer{
public:
    virtual void field_change(T& source,
                              const string& field_name) = 0;
};


template <typename T>
class Observable{
    vector<Observer<T>*> observers; //set of subscribers
public:
    void notify(T& source, const string& field_name){
        for (auto observer: observers)
            observer->field_change(source, field_name);
    }
    
    void subscribe(Observer<T>& observer){
        observers.push_back(&observer); //vector is not a thread-safe class. pushing
    }
    
    void unsubscribe(Observer<T>& observer){
        observers.erase(remove(observers.begin(), observers.end(), &observer),
                        observers.end()); //vector is not a thread-safe class. removing
    }
};


template <typename T>
class SafeObservable{ //thread safe approach
    vector<Observer<T>*> observers; //set of subscribers
    typedef std::mutex mutex_t;
    mutex_t mtx;
    
public:
    void notify(T& source, const string& field_name){
        scoped_lock<mutex_t> lock{mtx}; //std::scoped_lock, since c++17
        for (auto observer: observers)
            if (observer)
                observer->field_change(source, field_name);
    }
    
    void subscribe(Observer<T>& observer){
        scoped_lock<mutex_t> lock{mtx};
        observers.push_back(&observer); //vector is not a thread-safe class. pushing
    }
    
    void unsubscribe(Observer<T>& observer){
//        scoped_lock<mutex_t> lock{mtx};
//        observers.erase(remove(observers.begin(), observers.end(), &observer),
//                        observers.end()); //vector is not a thread-safe class. removing
        auto it = std::find(begin(observers), end(observers), &observer);
        if (it != end(observer) )
            *it = nullptr;
    }
};


//CRTP
class Person: public Observable<Person> {
    int age;
public:
    Person(int age): age(age) {}
    
    int get_age() const{
        return age;
    }
    
    void set_age(int age){
        if (this->age == age) return;
        
        auto old_can_vote = get_can_vote();
        this->age = age;
        notify(*this, "age");
        if (old_can_vote != get_can_vote()) // this method doesnt' scale if we have multiple get() to monitor
            notify(*this, "can_vote");
    }
    
    bool get_can_vote() const{
        return age >= 16;
    }
};


struct ConsoleObserver: public Observer<Person>{
private:
    void field_change(Person& source, const std::string &field_name) override{
        cout << "Person's " << field_name << " has changed to ";
        if (field_name == "age") cout << source.get_age();
        if (field_name == "can_vote") cout << boolalpha << source.get_can_vote();
        cout << ".\n";
    }
};


struct TrafficAdminitration : Observer<Person>
{
    void field_change(Person &source, const string& field_name) override{
        if (field_name == "age"){
            if (source.get_age() < 17)
                cout << "whoa there, you are not old enough to drive!\n" << endl;
            else
                //no longer interesting to monitor
                cout << "we no longer care!\n";
            source.unsubscribe(*this);
        }
    }
};


//Boost
template <typename T> struct Observable2{
    signal<void(T&, const string&)> field_change; //signal handler
};

class Person2 : public Observable2<Person2>{
    int age;
public:
    int get_age() const { return age; }
    void set_age(int age) {
        if (this->age == age) return;
        this->age = age;
        field_change(*this, "age");
    }
};


int main(){
    //1. naive observer-observable pattern
    Person person{10};
    ConsoleObserver cpo;
    person.subscribe(cpo);
    person.set_age(11);
    person.set_age(12);
    person.unsubscribe(cpo);
    person.set_age(13);
    
    //2. boost
    Person2 p2;
    //connect the lambda function to this signal handler
    auto conn = p2.field_change.connect( [](Person2& p, const string& field_name){
        cout << field_name << " has changed\n";
    });
    p2.set_age(20);
    conn.disconnect();
    
    //3. dependency
    Person p3{10};
    ConsoleObserver cpo2;
    p3.subscribe(cpo2);
    p3.set_age(15);
    p3.set_age(16);
    
    //4. thread-safe
    TrafficAdminitration ta;
    Person p4{15};
    p4.subscribe(ta);
    p4.set_age(15);
    p4.set_age(16);
    try{
        p4.set_age(17);
    } catch (const exception& e){
        cout << "Oops, " << e.what() << endl;
    }
    
    
    return 0;
}
