#include <string>
#include <variant>
#include <iostream>

using namespace std;

struct AddressPrinter
{
  void operator()(const string& house_name) const {
    cout << "A house called " << house_name << "\n";
  }

  void operator()(const int house_number) const {
    cout << "House number " << house_number << "\n";
  }
};

int main(int ac, char* av[])
{
  variant<string, int> house; //safe-type union
  //house = "Montefiore Castle";
  house = 221;

  AddressPrinter ap;
  std::visit(ap, house);

  // what if you want to create a visitor in-place?
  std::visit([](auto& arg) {
    //decltype(arg): extract type of arg
    //using decay_t = typename decay<T>::type (convert from lvalue reference to rvalue reference)
    //is_same_v<T,U> = is_same<T,U>::value, true if T and U are of the same type
    //constexpr means an expression can be inferencd by compiler at compile time
    if constexpr (is_same_v<decay_t<decltype(arg)>, string>)
    {
      cout << "A house called " << arg.c_str() << "\n";
    }
    else
    {
      cout << "House number " << arg << "\n";
    }
  }, house);

  return 0;
}
