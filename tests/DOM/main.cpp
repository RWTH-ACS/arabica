#ifdef _MSC_VER
#pragma warning(disable: 4786 4250 4503 4244)
#endif

#include "dom_test_suite.hpp"

////////////////////////////////////////////////
int main(int argc, const char* argv[])
{
  bool ok = DOM_test_suite<std::string, Arabica::default_string_adaptor<std::string> >(argc, argv);
  return !ok;
  // return 77;
} // main

// end of file
