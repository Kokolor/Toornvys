#include <iostream>
#include "../include/lexer.hpp"
#include "../include/parser.hpp"

int main() {
	Lexer lexer("hello = 4 + 7");
	std::vector<Token> tokens = lexer.tokenize();

	/* for (const Token &token : tokens) {
		std::cout << "Token: " << token.getValue() << " of kind " 
                  << static_cast<int>(token.getKind()) << std::endl;
	} */

	Parser parser(tokens);
	std::unique_ptr<Node> ast = parser.parse();

	std::cout << "AST Created" << std::endl;

	return 0;
}
