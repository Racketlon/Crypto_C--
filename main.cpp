#include "src/Interpreter.cpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: crypto <file>" << std::endl;
        return 1;
    }

    try {
        Interpreter interpreter;
        interpreter.interpret(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
