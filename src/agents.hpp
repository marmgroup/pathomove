#ifndef AGENTS_H
#define AGENTS_H

#define _USE_MATH_DEFINES
/// code to make agents
#include <vector>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <boost/foreach.hpp>
#include "landscape.hpp"

// Agent class
struct Population {
public:
    Population(const int popsize, const float range_agents,
    const float range_food, const int handling_time,
    float pTransmit) :
        // agents and traits
        nAgents (popsize),
        coordX (popsize, 0.0f),
        coordY (popsize, 0.0f),
        energy (popsize, 0.001),
        coef_nbrs (popsize, 0.f),
        coef_food (popsize, 0.f),
        coef_nbrs2 (popsize, 0.f),
        coef_food2 (popsize, 0.f),
        
        // count stationary behaviour
        counter (popsize, 0),
        // associations
        associations(popsize, 0),
        degree(popsize, 0),
        range_agents(range_agents),
        range_food(range_food),
        handling_time(handling_time),
        order(popsize, 1),
        infected(popsize, false),//,
        timeInfected(popsize, 0),
        pTransmit (pTransmit),
        nInfected(0),
        srcInfect(popsize, 0)
    {}
    ~Population() {}

    const int nAgents;
    std::vector<float> coordX;
    std::vector<float> coordY;
    std::vector<float> energy;
    // weights
    std::vector<float> coef_nbrs;
    std::vector<float> coef_food;
    std::vector<float> coef_nbrs2;
    std::vector<float> coef_food2;

    // counter and metrics
    std::vector<int> counter;
    std::vector<int> associations; // number of total interactions
    std::vector<int> degree;

    // sensory range and foraging
    const float range_agents, range_food;
    const int handling_time;

    // shuffle vector and transmission
    std::vector<int> order;
    std::vector<bool> infected;
    std::vector<int> timeInfected;
    float pTransmit;

    // the number of infected agents
    int nInfected;
    std::vector<int> srcInfect;

    // position rtree
    bgi::rtree< value, bgi::quadratic<16> > agentRtree;

    // funs for pop
    void shufflePop();
    void setTrait ();
    void initPos(Resources food);

    void updateRtree();
    std::pair<int, std::vector<int> > countFood (
        Resources &food, const float xloc, const float yloc);
    std::pair<int, std::vector<int> > countAgents (
        const float xloc, const float yloc);

    void move(Resources &food);
    void forage(Resources &food);
    
    std::vector<float> handleFitness();
    void Reproduce();
    
    //pathogen
    void introducePathogen(const int nAgInf);
    void pathogenSpread();
    void pathogenCost(const float costInfect);

    void countInfected();
    float propSrcInfection();

    // counting proximity based interactions
    void countAssoc();

};

float get_distance(float x1, float x2, float y1, float y2);

#endif // AGENTS_H
