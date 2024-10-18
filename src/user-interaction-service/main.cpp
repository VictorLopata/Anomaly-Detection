#include "main.h"

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
    redisReply* streamReply;
    redisReply* entriesReply;
    redisReply* entryReply;
    redisReply* fieldsReply;

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
    string covAnomaly_id = "$";
    string avgAnomaly_id = "$";
    string com;
    while(true) {

        com = "XREAD BLOCK 0 STREAMS covAnomaly avgAnomaly " + covAnomaly_id + " " + avgAnomaly_id;
        reply = RedisCommand(c, com.c_str());

        if (reply == nullptr) {
            cerr << "Errore nell'esecuzione del comando XREAD" << endl;
            break;
        } else {
            for (int i = 0; i < reply->elements; ++i) {

                streamReply = reply->element[i];
                string nomeStream = streamReply->element[0]->str;
                entriesReply = streamReply->element[1];

                for (int j = 0; j < entriesReply->elements; ++j) {

                    entryReply = entriesReply->element[j];
                    string entryID = entryReply->element[0]->str;
                    fieldsReply = entryReply->element[1];


                    for (size_t k = 0; k < fieldsReply->elements; k += 2)
                    {

                        string campo = fieldsReply->element[k]->str;
                        double valore = stod(fieldsReply->element[k+1]->str);

                        if (nomeStream == "avgAnomaly") {
                            if (campo == "sensor") {
                                cout << "Anomalia trovata al sensore " << valore << "; MEDIA ANOMALA: ";
                            } else {
                                cout << valore << endl;
                            }
                        } else if (nomeStream == "covAnomaly") {
                            if (campo == "anomalValue") {
                                cout << campo << ": " << valore << endl;
                            } else {
                                cout << campo << ": " << valore << "  ";
                            }
                        }


                    }
                    avgAnomaly_id = entryID;
                }

            }
        }
        freeReplyObject(reply);
    }

    redisFree(c);

    return 0;

}