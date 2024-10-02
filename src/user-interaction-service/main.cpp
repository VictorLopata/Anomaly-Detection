#include <iostream>
#include <stdlib.h>
#include "main.h"
using namespace std;

void coutFormEndl(string cod, string testo) {
    cout << cod << testo << "\033[0m" << endl;
}
void coutForm(string cod, string testo) {
    cout << cod << testo << "\033[0m";
}

void clearTerminal() {
    // CSI[2J clears screen, CSI[H moves the cursor to top-left corner
    cout << "\x1B[2J\x1B[H";
}

int main() {
    int num_sensor;
    int W;
    double threshold;

    /**
    cout << "\033[31m" << "TESTO ROSSO" << "\033[0m" << endl;
    cout << "\033[32m" << "Questo testo è verde" << "\033[0m" << endl;
    cout << "\033[1;34m" << "Questo testo è blu e in grassetto" << "\033[0m" << endl;
    cout << "\033[37;41m" << "Testo bianco su sfondo rosso" << "\033[0m" << endl;
    cout << "\033[30;42m" << "Testo nero su sfondo verde" << "\033[0m" << endl;
     **/
    string dec;
    string purpGrass = "\033[1;35m";
    string redGrass = "\033[1;33m";
    string bluGrass = "\033[1;34m";
    cout << "\033[4;1;35m" << "ANOMALY-SEEKER" << "\033[0m" << endl << endl;
    cout << "\033[1;35m" << "Benvenuto nel sistema di Anomaly Detection!" << "\033[0m" << endl;
    coutFormEndl(purpGrass, "Prima di procedere, assicurati che i seguenti servizi siano attivi:");
    cout << endl;
    coutFormEndl(purpGrass, "1) REDIS ");
    coutFormEndl(purpGrass, "2) POSTGRESQL");
    cout << endl;
    coutForm(purpGrass, "Continuare? [Y/n] ");

    while (1) {
        cin >> dec;

        if (dec == "Y" || dec == "y") {
            break;
        } else if (dec == "N" || dec == "n") {
            coutForm(purpGrass, "Va bene, ciao.");
            return 0;
        } else {
            coutForm(redGrass, "Input non valido! Inserisci 'Y' se vuoi continuare oppure 'n' se non vuoi continuare.");
            cin.clear();
        }
    }
    system("clear"); // NON FUNZIOONAAAAA :(
    // coutForm(purpGrass, "ECCOCI.");
    cout << endl;
    coutForm(bluGrass, "Quante stream vuoi analizzare? ");
    cin >> num_sensor;
    cout << endl;
    coutForm(bluGrass, "Inserisci l'ampiezza temporale W (in secondi, 0 se vuoi utilizzare valore di default 25): ");
    cin >> W;
    cout << endl;
    coutForm(bluGrass, "Inserisci il valore di threshold (0 se vuoi utilizzare valore di default 0.25): ");
    cin >> threshold;





    return 0;

}