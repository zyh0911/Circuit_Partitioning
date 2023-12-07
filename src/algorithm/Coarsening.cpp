

std::vector<std::pair<int, std::vector<int>*>>* Coarsen::Coarsening(Hypergraph* graph, 
                                                                    LayerInfo* inst, 
                                                                    std::vector<double>& spaceLimit, 
                                                                    bool restricted, 
                                                                    std::string& scheme)
{
    if (scheme == "HEC")
        return HEC.Coarsening(graph, inst, spaceLimit, restricted, false);
    else if (scheme == "MHEC")
        return HEC.Coarsening(graph, inst, spaceLimit, restricted, true); // MHEC
    else
        return EC.Coarsening(graph, inst, spaceLimit, restricted);
}