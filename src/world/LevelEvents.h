#ifndef WORLD_LEVELEVENTS_H_
#define WORLD_LEVELEVENTS_H_

#include <functional>
#include <vector>
#include <unordered_map>

class Chunk;

enum lvl_event_type {
    CHUNK_HIDDEN,
};

typedef std::function<void(lvl_event_type, Chunk*)> chunk_event_func;

class LevelEvents {
	std::unordered_map<lvl_event_type, std::vector<chunk_event_func>> chunk_callbacks;
public:
	void listen(lvl_event_type type, chunk_event_func func);
	void trigger(lvl_event_type type, Chunk* chunk);
};

#endif // WORLD_LEVELEVENTS_H_
