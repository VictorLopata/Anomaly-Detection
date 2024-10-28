#include "con2redis.h"
#include "local.h"

void print_reply_types()
{
  printf("REDIS_REPLY_STRING=%d,\
REDIS_REPLY_STATUS=%d,\
REDIS_REPLY_INTEGER=%d,\
REDIS_REPLY_NIL=%d,\
REDIS_REPLY_ERROR=%d,\
REDIS_REPLY_ARRAY=%d\n",
	 REDIS_REPLY_STRING,
	 REDIS_REPLY_STATUS,
	 REDIS_REPLY_INTEGER,
	 REDIS_REPLY_NIL,
	 REDIS_REPLY_ERROR,
	 REDIS_REPLY_ARRAY
	 );
  
};

config getConf(redisContext *c) {
    config conf;
    string last_conf_id = "$";

    while (true) {
        redisReply* reply = (redisReply*)redisCommand(c, "XREAD BLOCK 0 STREAMS conf %s", last_conf_id.c_str());
        if (reply == nullptr) {
            break;
        }
        if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0) {
            redisReply* streamReply = reply->element[0];
            if (streamReply->type == REDIS_REPLY_ARRAY && streamReply->elements == 2) {
                redisReply* messages = streamReply->element[1];
                for (size_t j = 0; j < messages->elements; ++j) {
                    redisReply* message = messages->element[j];
                    last_conf_id = message->element[0]->str;
                    redisReply* fields = message->element[1];
                    for (size_t k = 0; k < fields->elements; k += 2) {
                        string field_name = fields->element[k]->str;
                        string field_value = fields->element[k + 1]->str;
                        if (field_name == "num_streams") {
                            conf.num_streams = stoi(field_value);
                            // cout << "Numero di stream da ascoltare: " << conf.num_streams << endl;
                        } else if (field_name == "sens") {
                            conf.sens = stod(field_value);
                            // cout << "Threshold: " << conf.threshold << endl;
                        } else if (field_name == "W") {
                            conf.W = stod(field_value);
                            // cout << "W: " << conf.W << endl;
                        }
                    }
                }
            }
            freeReplyObject(reply);
            break;
        }
        freeReplyObject(reply);
    }
    return conf;
}


void assertReplyType(redisContext *c, redisReply *r, int type) {
    if (r == NULL)
        dbg_abort("NULL redisReply (error: %s)", c->errstr);
    if (r->type != type)
      { print_reply_types();
	dbg_abort("Expected reply type %d but got type %d", type, r->type);
      }
}



void assertReply(redisContext *c, redisReply *r) {
    if (r == NULL)
        dbg_abort("NULL redisReply (error: %s)", c->errstr);
}



void initStreams(redisContext *c, const char *stream) {
    redisReply *r = RedisCommand(c, "XGROUP CREATE %s diameter $ MKSTREAM", stream);
    assertReply(c, r);
    freeReplyObject(r);
}
