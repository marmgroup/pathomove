#include <vector>
#include <random>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "parameters.h"
#include "landscape.h"
#include "agents.h"
#include "network.h"
#include "network_operations.hpp"
#include "data_types.h"

#include <Rcpp.h>

using namespace Rcpp;

// function to evolve population
Rcpp::List evolve_pop(int genmax, double tmax,
                Population &pop, Resources &food, Network &pbsn)
{
    // make generation data
    genData thisGenData;
    networkData thisNetworkData;
    // set seed
    gsl_rng_set(r, seed);
    for(int gen = 0; gen < genmax; gen++) {
        pop.initPos(food);

        double total_act = std::accumulate(pop.trait.begin(), pop.trait.end(), 0.0);
        double trait_array[pop.nAgents];
        std::copy(pop.trait.begin(), pop.trait.end(), trait_array);
        double time = 0.0;
        double feed_time = 0.0;
        double it_t = 0.0;
        size_t id;
        double increment = 0.1;

        // lookup table for discrete distr
        gsl_ran_discrete_t*g = gsl_ran_discrete_preproc(static_cast<size_t>(pop.nAgents), trait_array);

        for(; time < tmax; ) {
            time += gsl_ran_exponential(r, total_act);

            /// foraging dynamic
            if (time > regenTime) {
                // count available food items
                food.countAvailable();
                for (size_t j = 0; j < static_cast<size_t>(food.nItems); j++)
                {
                    if(food.counter[j] > 0.0) {
                        food.counter[j] -= time;
                    }
                }
                // pop forages
                for (size_t i = 0; i < static_cast<size_t>(pop.nAgents); i++) {
                    forage(i, food, pop, 2.0);
                }
                // update population pbsn
                pop.updatePbsn(pbsn, 2.0, food.dSize);
                feed_time += 1.0;
            }

            /// movement dynamic
            if (time > it_t) {
                id = gsl_ran_discrete(r, g);
                pop.move(id, food, moveCost);
                it_t = (std::floor(time / increment) * increment) + increment;
            }
        }
        // generation ends here
        // update gendata
        thisGenData.updateGenData(pop, gen);
        thisNetworkData.updateNetworkData(pop, gen, pbsn);
        // subtract competition costs
        pop.competitionCosts(0.0001);
        // reproduce
        pop.Reproduce();
    }
    return Rcpp::List::create(
                Named("trait_data") = thisGenData.getGenData(),
                Named("network_measures") = thisNetworkData.getNetworkData()
            );
}

//' Make landscapes with discrete food items in clusters.
//'
//' @description Makes landscape and writes them to file with unique id.
//'
//' @param foodClusters Number of clusters around which food is generated.
//' @param clusterDispersal How dispersed food is around the cluster centre.
//' @param nFood The number of food items.
//' @param landsize The size of the landscape as a numeric (double).
//' @param replicates How many replicates.
//' @return Nothing. Runs simulation.
// [[Rcpp::export]]
void export_test_landscapes(int foodClusters, double clusterDispersal,
                            int nFood, double landsize, int replicates) {
    // outpath is data/test_landscape

    // assumes path/type already prepared
    std::string path = "data/test_landscape";
    // output filename as milliseconds since epoch

    for(int i = 0; i < replicates; i++) {

        // make a landscape
        Resources tmpFood(nFood, landsize);
        tmpFood.initResources(foodClusters, clusterDispersal);

        // get unique id
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto value = now_ms.time_since_epoch();

        // add a random number to be sure of discrete values
        long duration = value.count() + static_cast<long>(gsl_rng_uniform_int(r, 10000));
        std::string output_id = std::to_string(duration);
        output_id = "test_landscape" + output_id;

        // write summary of test landscapes
        const std::string summary_out = path + "/test_landscape_lookup.csv";
        std::ofstream summary_ofs;

        // if not exists write col names
        std::ifstream f2(summary_out.c_str());
        if (!f2.good()) {
            summary_ofs.open(summary_out, std::ofstream::out);
            summary_ofs << "filename,clusters,dispersal,replicate\n";
            summary_ofs.close();
        }
        // append if not
        summary_ofs.open(summary_out, std::ofstream::out | std::ofstream::app);
        summary_ofs << output_id << ","
                    << foodClusters << ","
                    << clusterDispersal << ","
                    << i << "\n";
        summary_ofs.close();

        // write the test landscape
        std::ofstream test_land_ofs;
        test_land_ofs.open(path + "/" + output_id + ".csv", std::ofstream::out);
        test_land_ofs << "x,y\n";

        for (size_t i = 0; i < static_cast<size_t>(tmpFood.nItems); i++)
        {
            test_land_ofs << tmpFood.coordX[i] << "," << tmpFood.coordY[i] << "\n";
        }

        test_land_ofs.close();

    }
}

//' Runs the sociality model simulation.
//'
//' @description Run the simulation using parameters passed as
//' arguments to the corresponding R function.
//' 
//' @param popsize The population size.
//' @param genmax The maximum number of generations per simulation.
//' @param tmax The number of timesteps per generation.
//' @param nFood The number of food items.
//' @param foodClusters Number of clusters around which food is generated.
//' @param clusterDispersal How dispersed food is around the cluster centre.
//' @param landsize The size of the landscape as a numeric (double).
//' @return A data frame of the evolved population traits.
// [[Rcpp::export]]
Rcpp::List do_simulation(int popsize, int genmax, int tmax, 
    int nFood, int foodClusters, double clusterDispersal, double landsize) {

    // prepare landscape
    Resources food (nFood, landsize);
    food.initResources(foodClusters, clusterDispersal);
    food.countAvailable();
    Rcpp::Rcout << "landscape with " << foodClusters << " clusters\n";
     /// export landscape

    // prepare population
    Population pop (popsize, 0);
    // pop.initPop(popsize);
    pop.setTrait();
    Rcpp::Rcout << pop.nAgents << " agents over " << genmax << " gens of " << tmax << " timesteps\n";

    // prepare social network struct
    Network pbsn;
    pbsn.initAssociations(pop.nAgents);

    // evolve population and store data
    Rcpp::List evoSimData = evolve_pop(genmax, tmax, pop, food, pbsn);

    Rcpp::Rcout << "data prepared\n";

    return evoSimData;
}

//' Export a population.
//'
//' @param popsize The population size.
// [[Rcpp::export]]
DataFrame export_pop(int popsize) {
    Rcpp::Rcout << "in export function";
    Population pop (popsize, 2);
    // pop.initPop(popsize);
    pop.setTrait();

    DataFrame df_pop = DataFrame::create(
                Named("trait") = pop.trait,
                Named("energy") = pop.energy
            );

    return df_pop;
}
