/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include <cstdio>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include "result.h"

configuration_result::configuration_result(size_t _no): result(_no) {
  XMLCh* core_impl_name=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode("core");
  XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation* impl=XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementationRegistry::getDOMImplementation(core_impl_name);
  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&core_impl_name);
  tag=impl->createDocument();//NULL, doc_name, NULL);
}

configuration_result::~configuration_result() {
  tag->release();
}
