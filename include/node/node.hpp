#pragma once

#include <memory>
#include "lexer.hpp"

class Node
{
public:
	virtual ~Node() = default;

	virtual int getLine() const = 0;
};
