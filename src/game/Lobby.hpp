#ifndef LOBBY_HPP
#define LOBBY_HPP

#include <vector>

#include "../network/Client.hpp"

class Lobby {
private:
    std::vector<Client*> clients;

public:
    Lobby();
};


#endif
