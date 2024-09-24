#include "main.h";

int setWindow(int argc, char *argv[]);

int main(int argc, char *argv[]) {

    int windowSize = setWindow(argc, argv);

}

int setWindow(int argc, char *argv[]) {

    // default size if user don't put any argument
    int size = 10;

    // check if user provided a value for the size
    if (argc > 1) {
        size = atoi(argv[1]);
    }

    // check if the value for the size is acceptable
    if (size < 1) {
        cout << "ERROR: window size must be greater or equal to 1" << endl;
        exit(1);
    }
    return size;
}
