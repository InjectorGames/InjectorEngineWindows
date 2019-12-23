
// Copyright 2019 Nikita Fediuchin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <string>
#include <exception>
#include <stdexcept>

using namespace std;

// Argument exception container class
class ArgumentException : public runtime_error
{
public:
	// Creates a new argument exception class instance
	ArgumentException(const string& message) : runtime_error(message) { }
	// Creates a new argument exception class instance
	ArgumentException(const char* message) : runtime_error(message) { }
};

// Argument null exception container class
class ArgumentNullException : public runtime_error
{
public:
	// Creates a new argument null exception class instance
	ArgumentNullException(const string& message) : runtime_error(message) { }
	// Creates a new argument null exception class instance
	ArgumentNullException(const char* message) : runtime_error(message) { }
};

// Argument out of range exception container class
class ArgumentOutOfRangeException : public runtime_error
{
public:
	// Creates a new argument out of range exception class instance
	ArgumentOutOfRangeException(const string& message) : runtime_error(message) { }
	// Creates a new argument out of range exception class instance
	ArgumentOutOfRangeException(const char* message) : runtime_error(message) { }
};

// Graphics exception container class
class GraphicsException : public runtime_error
{
public:
	// Creates a new graphics exception class instance
	GraphicsException(const string& message) : runtime_error(message) { }
	// Creates a new graphics exception class instance
	GraphicsException(const char* message) : runtime_error(message) { }
};