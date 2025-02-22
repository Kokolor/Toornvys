#include <iostream>
#include "../include/lexer.hpp"
#include "../include/parser.hpp"

std::string tokenKindToString(Token::Kind kind) {
    switch(kind) {
        case Token::Kind::TOKEN_PLUS: return "+";
        case Token::Kind::TOKEN_MINUS: return "-";
        case Token::Kind::TOKEN_STAR: return "*";
        case Token::Kind::TOKEN_SLASH: return "/";
        default: return "?";
    }
}

void printAST(const Node* node, int indent = 0) {
    std::string indentation(indent, ' ');
    
    if (const NodeNumber* num = dynamic_cast<const NodeNumber*>(node)) {
        std::cout << indentation << "Number(" << num->getValue() << ")\n";
    }
    else if (const NodeIdentifier* id = dynamic_cast<const NodeIdentifier*>(node)) {
        std::cout << indentation << "Identifier(" << id->getName() << ")\n";
    }
    else if (const NodeBinaryOp* op = dynamic_cast<const NodeBinaryOp*>(node)) {
        std::cout << indentation << "BinaryOp(" 
                  << tokenKindToString(op->getOp()) << ")\n";
        printAST(op->getLeft(), indent + 2);
        printAST(op->getRight(), indent + 2);
    }
    else {
        std::cout << indentation << "Unknown Node\n";
    }
}

int main() {
	Lexer lexer("4 + 7 * 2");
	std::vector<Token> tokens = lexer.tokenize();

	for (const Token &token : tokens) {
		std::cout << "Token: " << token.getValue() << " of kind " 
                  << static_cast<int>(token.getKind()) << std::endl;
	}

	try {
        Parser parser(tokens);
        std::unique_ptr<Node> ast = parser.parse();
        
        std::cout << "\nAST Structure:\n";
        printAST(ast.get());
    } 
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

	return 0;
}
