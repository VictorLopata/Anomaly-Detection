#include <iostream>
#include <cstdlib>
#include "main.h"
#include "../utils/configuration.h"
#include "../con2redis/con2redis.h"

using namespace std;

void printColored(const string& colorCode, const string& message, bool newLine = true) {
    cout << colorCode << message << "\033[0m" ;
    if (newLine) {
        cout << endl;
    }
}

int main() {

    config cfg;
    redisContext *c;
    redisReply *reply;

    string dec;

    printColored(PURP_GRASS, "ANOMALY-SEEKER");
    cout << endl;
    printColored(PURP_GRASS, "Benvenuto nel sistema di Anomaly Detection!");
    printColored(PURP_GRASS, "Prima di procedere, assicurati che i seguenti servizi siano attivi:");
    cout << endl;
    printColored(PURP_GRASS, "1) REDIS ");
    printColored(PURP_GRASS, "2) POSTGRESQL");
    cout << endl;
    printColored(PURP_GRASS, "Continuare? [Y/n] ");

    while (true) {
        cin >> dec;

        if (dec == "Y" || dec == "y") {
            break;
        } else if (dec == "N" || dec == "n") {
            printColored(PURP_GRASS, "Va bene, ciao.", false);
            return 0;
        } else {
            printColored(YELL_GRASS, "Input non valido! Inserisci 'Y' se vuoi continuare oppure 'n' se non vuoi continuare.", false);
            cin.clear();
        }
    }

    system("clear"); // Only works for unix systems...
    printColored(BLUE_GRASS, "Quante stream vuoi analizzare? ", false);
    cin >> cfg.num_streams;
    cout << endl;

    printColored(BLUE_GRASS, "Inserisci l'ampiezza temporale W (in secondi, 0 se vuoi utilizzare valore di default 25): ", false);
    cin >> cfg.W;
    if (cfg.W == 0) {
        cfg.W = DEFAULT_W;
    }
    cout << endl;

    printColored(BLUE_GRASS, "Inserisci il valore di threshold (0 se vuoi utilizzare valore di default 0.25): ", false);
    cin >> cfg.threshold;
    if (cfg.threshold == 0) {
        cfg.threshold = DEFAULT_THRESHOLD;
    }


    // Avvia la trasmissione della configuarazione
    c = redisConnect(REDIS_SERVER, REDIS_PORT);
    reply = RedisCommand(c, "XADD conf * num_streams %s W %s threshold %s", (to_string(cfg.num_streams)).c_str(), (to_string(cfg.W)).c_str(), (to_string(cfg.threshold)).c_str());

    assertReply(c, reply);

    system("clear");

    // Detection started
    printColored(YELL_GRASS, "INIZIO DETECTION...");

    while(true) {
        /**
         * Listen anomaly_stream and wait for an anomaly.
         */
    }


    freeReplyObject(reply);
    redisFree(c);

    return 0;

}