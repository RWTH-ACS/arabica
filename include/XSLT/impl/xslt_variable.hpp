#ifndef ARABICA_XSLT_VARIABLE_HPP
#define ARABICA_XSLT_VARIABLE_HPP

#include "xslt_variable_impl.hpp"

namespace Arabica
{
namespace XSLT
{

class Variable : public Variable_impl
{
public:
  Variable(const std::string& namespace_uri, 
           const std::string& name, 
           Arabica::XPath::XPathExpressionPtr<std::string> select,
           const Precedence precedence) :
      Variable_impl(namespace_uri, name, select, precedence)
  {
  } // Variable

  virtual ~Variable() { }

  virtual void execute(const DOM::Node<std::string>& node, ExecutionContext& context) const 
  {
    context.declareVariable(node, *this);
  } // declare
}; // Variable

} // namespace XSLT
} // namespace Arabica
#endif

