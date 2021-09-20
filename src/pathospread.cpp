/// define functions for pathogen spread

#include "parameters.hpp"
#include "agents.hpp"
#include <unordered_set>

/// function to infect n individuals
void Population::introducePathogen(const int initialInfections) {

    shufflePop();
    // loop through the intended number of infections
    for (int i = 0; i < initialInfections; i++)
    {       
        // toggle infected agents boolean for infected
        infected[order[i]] = true;
        timeInfected[order[i]] = 1;       
        srcInfect[order[i]] = 2; // count as inherited?
    }
}

/// function to spread pathogen
void Population::pathogenSpread() {
    std::bernoulli_distribution transmission (pTransmit);
    // looping through agents, query rtree for neighbours
    for (size_t i = 0; i < nAgents; i++)
    {
        // spread to neighbours if self infected
        if (infected[i]) 
        {
            timeInfected[i]++; // increase time infecetd
            // get neigbour ids
            std::vector<int> nbrsId = countAgents(
                coordX[i], coordY[i]
            ).second;

            if (nbrsId.size() > 0) 
            {
                // loop through neighbours
                for(size_t j = 0; j < nbrsId.size(); j++) 
                {
                    size_t toInfect = nbrsId[j];
                    if (!infected[toInfect]) 
                    {
                        // infect neighbours with prob p
                        if(transmission(rng))
                        {
                            infected[toInfect] = true;
                            srcInfect[toInfect] = 2;
                        }
                    }
                }
            }
        }   
    }
}

/// function for pathogen cost
void Population::pathogenCost(const float costInfect) {
    for (size_t i = 0; i < nAgents; i++)
    {
        if(infected[i]) {
            energy[i] -= (costInfect * static_cast<float>(timeInfected[i]));
        }
    }
}

/// count infected agents
void Population::countInfected() {
    nInfected = 0;
    for (size_t i = 0; i < nAgents; i++)
    {
        if(infected[i]) {
            nInfected++;
        }
    }
    assert(nInfected <= nAgents);
}

/// proportion of infection sources
float Population::propSrcInfection() {
    int vertical = 0; int horizontal = 0;
    for (size_t i = 0; i < nAgents; i++)
    {
        if(infected[i]) {
            if (srcInfect[i] == 1)
            {
                vertical ++;
            } else if (srcInfect[i] == 2) {
                horizontal ++;
            }
            
        }
    }
    // Rcpp::Rcout << "# horizontal infections = " << horizontal << "\n";
    float propSource = (vertical == 0 && horizontal == 0) ? 0.f : (static_cast<float>(horizontal) / static_cast<float>(vertical + horizontal));

    return propSource;
}
