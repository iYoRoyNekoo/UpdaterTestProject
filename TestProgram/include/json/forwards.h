// Copyright 2007-2010 Baptiste Lepilleur and The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSON_FORWARDS_H_INCLUDED
#define JSON_FORWARDS_H_INCLUDED

#define DLLImport _declspec(dllimport)

#if !defined(JSON_IS_AMALGAMATION)
#include "config.h"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace Json {

// writer.h
class DLLImport FastWriter;
class DLLImport StyledWriter;

// reader.h
class DLLImport Reader;

// features.h
class DLLImport Features;

// value.h
typedef unsigned int ArrayIndex;
class DLLImport StaticString;
class DLLImport Path;
class DLLImport PathArgument;
class DLLImport Value;
class DLLImport ValueIteratorBase;
class DLLImport ValueIterator;
class DLLImport ValueConstIterator;

} // namespace Json

#endif // JSON_FORWARDS_H_INCLUDED
