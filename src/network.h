#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <cassert>

struct Network {
public:

    int nAgents = 0;
    std::vector<std::vector<int> > associations;

    void initAssociations(int nVertices);
};

// initialise associations
void Network::initAssociations(int nVertices){

    nAgents = nVertices;
    associations = std::vector<std::vector<int> > (nAgents - 1, std::vector<int> (nAgents - 1, 0));
    for(size_t i = 0; i < static_cast<size_t>(nAgents - 1); i ++) {
        associations[i] = (std::vector<int> (nAgents - (i + 1), 0));
    }

    // check size
    assert(static_cast<int>(associations[0].size()) == (nAgents - 1) && "association triangle is wrong");
}

#endif // NETWORK_H
