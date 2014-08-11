/**
 * @file opaque.h  Macros and C++11 templates for opaque private implementations.
 *
 * @author Copyright (c) 2014 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * @par License
 * BSD: http://opensource.org/licenses/BSD-2-Clause
 *
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */
 
#ifndef LIBOPAQUE_H
#define LIBOPAQUE_H

#include <cassert>
#include <algorithm>
#include <utility>

/*
 * Version history:
 * 1. Initial version based on the PIMPL macros from Doomsday.
 */
#define OPAQ_VERSION 1

#if !defined(NDEBUG) || defined(_DEBUG)
#  define OPAQ_DEBUG
#endif

namespace opaq {

/**
* Macro for starting the definition of an opaque implementation struct. The
* struct holds a reference to the public instance, which must be specified in
* the call to the base class constructor. @see opaq::Private
*
* Example:
* <pre>
*    OPAQ_IMPLEMENT(MyClass)
*    {
*        Instance(Public &inst) : Base(inst) {
*            // constructor
*        }
*        // private data and methods
*    };
* </pre>
*/
#define OPAQ_IMPLEMENT(ClassName) \
    typedef ClassName Public; \
    struct ClassName::Instance : public opaq::Private<ClassName>

/**
 * Macro for starting the definition of a private implementation struct without
 * a reference to the public instance. This is useful for simpler classes where
 * the private implementation mostly holds member variables.
 */
#define OPAQ_IMPLEMENT_NOREF(ClassName) \
    struct ClassName::Instance : public opaq::IPrivate

/**
 * Macro for publicly declaring a pointer to the private implementation.
 * opaq::PrivateAutoPtr owns the private instance and will automatically delete
 * it when the PrivateAutoPtr is destroyed.opaq::
 */
#define OPAQ_DECLARE(Var) \
    struct Instance; \
    opaq::PrivateAutoPtr<Instance> Var;

/**
 * Interface for all private instance implementation structs.
 * In a debug build, also contains a verification code that can be used
 * to assert whether the pointed object really is derived from IPrivate.
 */
struct IPrivate {
    virtual ~IPrivate() {}
#ifdef OPAQ_DEBUG
    unsigned int _privateInstVerification;
#define OPAQ_IPRIVATE_VERIFICATION 0xBEEFDEAD
    IPrivate() : _privateInstVerification(OPAQ_IPRIVATE_VERIFICATION) {}
    unsigned int privateInstVerification() const { return _privateInstVerification; }
#endif
};

/**
 * Pointer to the private implementation. Behaves like std::unique_ptr, but with
 * the additional requirement that the pointed/owned instance must be derived
 * from opaq::IPrivate.
 */
template <typename T>
class PrivateAutoPtr
{
public:
    PrivateAutoPtr(T *p = 0) : ptr(p) {}
    ~PrivateAutoPtr() { reset(); }
    
    PrivateAutoPtr(PrivateAutoPtr const &) = delete; // no copy
    PrivateAutoPtr &operator = (PrivateAutoPtr const &) = delete; // no assign

    T &operator * () const { return *ptr; }
    T *operator -> () const { return ptr; }
    void reset(T *p = 0) {
        IPrivate *ip = reinterpret_cast<IPrivate *>(ptr);
        if(ip)
        {
            assert(ip->privateInstVerification() == OPAQ_IPRIVATE_VERIFICATION);
            delete ip;
        }
        ptr = p;
    }
    T *get() const {
        return ptr;
    }
    T const *getConst() const {
        return ptr;
    }
    operator T *() const {
        return ptr;
    }
    T *release() {
        T *p = ptr;
        ptr = 0;
        return p;
    }
    void swap(PrivateAutoPtr &other) {
        std::swap(ptr, other.ptr);
    }
    bool isNull() const {
        return !ptr;
    }
#ifdef OPAQ_DEBUG
    bool isValid() const {
        return ptr && reinterpret_cast<IPrivate *>(ptr)->privateInstVerification() == OPAQ_IPRIVATE_VERIFICATION;
    }
#endif

private:
    T *ptr;
};

/**
 * Utility template for defining private implementation data (pimpl idiom). Use
 * this in source files, not in headers.
 */
template <typename T>
struct Private : public IPrivate {
    T &self;
#define this_public (&self)      // To be used inside private implementations.

    typedef Private<T> Base;

    Private(T &i) : self(i) {}
    Private(T *i) : self(*i) {}
};

} // namespace opaq

#endif // LIBOPAQUE_H
