
#include <iostream>
#include <vector>
#include <string>
#include "main.h"
#include "../con2redis/con2redis.h"


using namespace std;

int main() {
    // Connessione a Redis
    redisContext *c = redisConnect(REDIS_SERVER, REDIS_PORT);
    if (c == NULL || c->err) {
        if (c) {
            std::cerr << "Errore: " << c->errstr << std::endl;
        } else {
            std::cerr << "Non posso allocare il contesto Redis" << std::endl;
        }
        exit(1);
    }

    // Aspetta la configurazione sulla stream conf
    config conf = getConf(c);

    cout << "n_stream: " << conf.num_streams << " Threshold: "<< conf.threshold << " W: "<< conf.W <<endl;
    /**
    vector<string> streams = {"stream1", "stream2", "stream3"};
    vector<string> stream_ids(streams.size(), "$");

    while (true) {
        // Costruisci il comando XREAD
        string command = "XREAD BLOCK 0 STREAMS";
        for (const auto& stream : streams) {
            command += " " + stream;
        }
        for (const auto& id : stream_ids) {
            command += " " + id;
        }

        // Invia il comando
        redisReply *reply = (redisReply *)redisCommand(c, command.c_str());
        if (reply == NULL) {
            std::cerr << "Errore: " << c->errstr << std::endl;
            redisFree(c);
            exit(1);
        }

        if (reply->type == REDIS_REPLY_NIL) {
            // Nessun nuovo messaggio
            freeReplyObject(reply);
            continue;
        }

        if (reply->type != REDIS_REPLY_ARRAY) {
            std::cerr << "Tipo di risposta inaspettato: " << reply->type << std::endl;
            freeReplyObject(reply);
            continue;
        }


        for (size_t i = 0; i < reply->elements; ++i) {
            redisReply *stream_reply = reply->element[i];
            if (stream_reply->type != REDIS_REPLY_ARRAY || stream_reply->elements != 2) {
                continue;
            }

            std::string stream_name = stream_reply->element[0]->str;
            redisReply *messages = stream_reply->element[1];

            for (size_t j = 0; j < messages->elements; ++j) {
                redisReply *message = messages->element[j];
                if (message->type != REDIS_REPLY_ARRAY || message->elements != 2) {
                    continue;
                }

                std::string message_id = message->element[0]->str;
                redisReply *fields = message->element[1];

                std::cout << "Stream: " << stream_name << ", ID: " << message_id << std::endl;
                for (size_t k = 0; k < fields->elements; k += 2) {
                    std::string field_name = fields->element[k]->str;
                    std::string field_value = fields->element[k + 1]->str;
                    std::cout << field_name << ": " << field_value << std::endl;
                }
                std::cout << "------------------------" << std::endl;

                // Aggiorna l'ID dello stream
                for (size_t s = 0; s < streams.size(); ++s) {
                    if (streams[s] == stream_name) {
                        stream_ids[s] = message_id;
                        break;
                    }
                }
            }
        }

        freeReplyObject(reply);
    }



     **/
    redisFree(c);
    return 0;
}
