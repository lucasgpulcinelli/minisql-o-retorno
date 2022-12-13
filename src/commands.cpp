#include "commands.hpp"
#include "NetworkGraph.hpp"

void commandMaxSpeed(NetworkGraph& net_topology){
    int32_t num_checks;
    std::cin >> num_checks;
        
    for(int32_t check = 0; check < num_checks; check++){
        int32_t origin_pop;
        int32_t destination_pop;

        std::cin >> origin_pop >> destination_pop;
        std::cout << "Fluxo máximo entre " << origin_pop;
        std::cout << " e " << destination_pop << ": ";

        int32_t max_speed = net_topology.getMaxSpeed(origin_pop, destination_pop);
        if(max_speed == -1){
            std::cout << max_speed << std::endl;
            continue;
        }

        std::cout << max_speed << " Mbps" << std::endl;
    }
}

void commandLength(NetworkGraph& net_topology){
    int32_t num_calculations;
    std::cin >> num_calculations;
        
    for(int32_t check = 0; check < num_calculations; check++){
        int32_t origin_pop;
        int32_t destination_pop;
        int32_t stop;

        std::cin >> origin_pop >> destination_pop >> stop;
        std::cout << "Comprimento do caminho entre " << origin_pop << " e "; 
        std::cout << destination_pop << " parando em " << stop << ": ";

        int32_t min_len_cb = net_topology.getLen(stop, destination_pop);
        if(min_len_cb < 0){
            std::cout << -1 << std::endl;
            continue;
        }

        int32_t min_len_ac = net_topology.getLen(origin_pop, stop);
        if(min_len_ac < 0){
            std::cout << -1 << std::endl;
            continue;
        }

        std::cout << min_len_ac + min_len_cb << "Mbps" << std::endl;
    }
}