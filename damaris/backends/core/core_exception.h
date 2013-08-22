/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

 ****************************************************************************/
#ifndef CORE_EXCEPTION_H
#define CORE_EXCEPTION_H

#include <stdexcept>
#include <string>

/**
 * Common base class for all DAMARIS exceptions. Note that a string error message must be provided upon initialization
 */
class DamarisException: public std::runtime_error
{
public:
    explicit DamarisException(const std::string& msg) throw (): std::runtime_error(msg) {}
    explicit DamarisException(const char* msg) throw (): std::runtime_error(msg) {}
    virtual ~DamarisException() throw () {}

    /**
     * Override the what() function to automatically allow adding a prefix (simplifies catch statements)
     */
    virtual const char* what() const throw ()
    {
        std::string msg(std::runtime_error::what());
        msg.insert(0, prefix());
        return msg.c_str();
    }
protected:
    virtual const std::string prefix() const { return "ERROR (DamarisException): "; }
};

/**
 * Core-specific exception
 */
class core_exception: public DamarisException
{
public:
    explicit core_exception(const std::string& msg) throw (): DamarisException(msg) {}
    explicit core_exception(const char* msg) throw (): DamarisException(msg) {}
    virtual ~core_exception() throw () {}
protected:
    virtual const std::string prefix() const { return "ERROR (core_exception): "; }
};

/**
 * Recoverable exception
 */
class RecoverableException: public DamarisException
{
public:
    explicit RecoverableException(const std::string& msg) throw (): DamarisException(msg) {}
    explicit RecoverableException(const char* msg) throw (): DamarisException(msg) {}
    virtual ~RecoverableException() throw () {}
protected:
    virtual const std::string prefix() const { return "ERROR (RecoverableException): "; }
};

#endif
