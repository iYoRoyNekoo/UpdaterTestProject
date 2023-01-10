// Copyright 2007-2010 Baptiste Lepilleur and The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSON_FORWARDS_H_INCLUDED
#define JSON_FORWARDS_H_INCLUDED

#define DLLExport _declspec(dllexport)

#if !defined(JSON_IS_AMALGAMATION)
#include "config.h"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace Json {

// writer.h
class DLLExport FastWriter;
class DLLExport StyledWriter;

// reader.h
class DLLExport Reader;

// features.h
class DLLExport Features;

// value.h
typedef unsigned int ArrayIndex;
class DLLExport StaticString;
class DLLExport Path;
class DLLExport PathArgument;
class DLLExport Value;
class DLLExport ValueIteratorBase;
class DLLExport ValueIterator;
class DLLExport ValueConstIterator;

} // namespace Json

#endif // JSON_FORWARDS_H_INCLUDED
