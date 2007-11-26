/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef CORE_EXCEPTION_H
#define CORE_EXCEPTION_H

#include <string>

class core_exception: public std::string {
 public:
  core_exception(const std::string& message): std::string(message) {}
};

#endif
