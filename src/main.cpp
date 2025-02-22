#include <iostream>
#include "../include/lexer.hpp"

int main() {
	Lexer lexer("hello = 4 + 7");
	std::vector<Token> tokens = lexer.tokenize();

	for (const Token &token : tokens) {
		std::cout << "Token: " << token.getValue() << " of kind " 
                  << static_cast<int>(token.getKind()) << std::endl;
	}

	return 0;
}
