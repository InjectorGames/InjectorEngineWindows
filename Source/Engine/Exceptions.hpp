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
	ArgumentException(string message) : runtime_error(message) { }
	// Creates a new argument exception class instance
	ArgumentException(const char* message) : runtime_error(message) { }
};

// Argument null exception container class
class ArgumentNullException : public runtime_error
{
public:
	// Creates a new argument null exception class instance
	ArgumentNullException(string message) : runtime_error(message) { }
	// Creates a new argument null exception class instance
	ArgumentNullException(const char* message) : runtime_error(message) { }
};

// Argument out of range exception container class
class ArgumentOutOfRangeException : public runtime_error
{
public:
	// Creates a new argument out of range exception class instance
	ArgumentOutOfRangeException(string message) : runtime_error(message) { }
	// Creates a new argument out of range exception class instance
	ArgumentOutOfRangeException(const char* message) : runtime_error(message) { }
};

// Graphics exception container class
class GraphicsException : public runtime_error
{
public:
	// Creates a new graphics exception class instance
	GraphicsException(string message) : runtime_error(message) { }
	// Creates a new graphics exception class instance
	GraphicsException(const char* message) : runtime_error(message) { }
};