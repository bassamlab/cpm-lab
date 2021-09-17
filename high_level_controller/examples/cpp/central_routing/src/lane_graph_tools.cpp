#include "lane_graph_tools.hpp"
#include "geometry.hpp"
#include <future>

/**
 * \file lane_graph_tools.cpp
 * \ingroup central_routing
 */

/**
 * \brief TODO
 * \ingroup central_routing
 */
const LaneGraphTools laneGraphTools;

LaneGraphTools::LaneGraphTools()
{   /////////the arc length is computed by d_x =x_now -x_prev and d_y = y_now-y_prev and then length = sqrt(d_x^2+d_y^2) for all edges
    // Compute path arc lengths
    for (size_t i_edge = 0; i_edge < n_edges; ++i_edge)
    {
        std::vector<double> edge_s;
        edge_s.push_back(0);

        for (size_t i_path = 1; i_path < n_edge_path_nodes; ++i_path)
        {
            const double dx = edges_x[i_edge][i_path] - edges_x[i_edge][i_path-1];
            const double dy = edges_y[i_edge][i_path] - edges_y[i_edge][i_path-1];
            const double len = sqrt(dx*dx+dy*dy);
            //std::cout << len << ", ";
            //the new calculated waylength is addad to the previous length
            edge_s.push_back(edge_s.back() + len);
        }
        assert(edge_s.size() == edges_x[i_edge].size());
        edges_s.push_back(edge_s);
    }


    //////////////// Precompute collision between all reference poses
    edge_path_collisions = std::vector< std::vector< std::vector< std::vector<bool> > > >
    (
        n_edges,
        std::vector< std::vector< std::vector<bool> > >
        (
            n_edge_path_nodes,
            std::vector< std::vector<bool> >
            (
                n_edges,
                std::vector<bool> 
                (
                    n_edge_path_nodes,
                    false
                )
            )
        )
    );
    // In the vector <jobs> all distances from node A to every known node B is listed. 
    // In vector <edge_path_collisions> every potential collision is written under criteria that the distance is under 0.01 m.
    std::vector<std::future<bool>> jobs;
    for (size_t i_edge_A = 0; i_edge_A < n_edges; ++i_edge_A)
    {
        jobs.push_back(std::async(std::launch::async, 
            [this, i_edge_A](){
                for (size_t i_path_A = 0; i_path_A < n_edge_path_nodes; ++i_path_A)
                {
                    PathNode nodeA (
                        edges_x.at(i_edge_A).at(i_path_A),
                        edges_y.at(i_edge_A).at(i_path_A),
                        edges_cos.at(i_edge_A).at(i_path_A),
                        edges_sin.at(i_edge_A).at(i_path_A)
                    );

                    for (size_t i_edge_B = 0; i_edge_B < n_edges; ++i_edge_B)
                    {
                        for (size_t i_path_B = 0; i_path_B < n_edge_path_nodes; ++i_path_B)
                        {
                            PathNode nodeB (
                                edges_x.at(i_edge_B).at(i_path_B),
                                edges_y.at(i_edge_B).at(i_path_B),
                                edges_cos.at(i_edge_B).at(i_path_B),
                                edges_sin.at(i_edge_B).at(i_path_B)
                            );

                            const double distance = min_distance_vehicle_to_vehicle(nodeA, nodeB);
                            edge_path_collisions.at(i_edge_A).at(i_path_A).at(i_edge_B).at(i_path_B) = (distance < 0.01);
                        }
                    }
                }
                return true;
            }
        ));
    }

    for(auto& job:jobs)
    {
        job.get();
    }
}
//////////////////Localization on the map////////////////////////////////////////
//each edge is searched on the lane graph and if it matched, true is returned
bool LaneGraphTools::map_match_pose(Pose2D pose, int &out_edge_index, int &out_edge_path_index) const
{
    bool match_found = false;
    double min_squared_distance = 1e300;

    for (size_t i_edge = 0; i_edge < n_edges; ++i_edge)
    {
        for (size_t i_path = 0; i_path < n_edge_path_nodes; ++i_path)
        {
            const double x = edges_x[i_edge][i_path];
            const double y = edges_y[i_edge][i_path];
            const double c = edges_cos[i_edge][i_path];
            const double s = edges_sin[i_edge][i_path];

            const double cos_delta_yaw = c * cos(pose.yaw()) + s * sin(pose.yaw());
            const double dx = x - pose.x();
            const double dy = y - pose.y();
            const double squared_distance = dx*dx + dy*dy;

            if( cos_delta_yaw > 0.7
                && squared_distance < 0.09
                && squared_distance < min_squared_distance)
            {
                match_found = true;
                out_edge_index = i_edge;
                out_edge_path_index = i_path;
                min_squared_distance = squared_distance;
            }
        }
    }
    return match_found;
}
//find
vector<size_t> LaneGraphTools::find_subsequent_edges(int edge_index) const
{
    vector<size_t> result;
    //if start index is the same as end index, remebr that
    size_t subsequent_node_index = edges_end_index.at(edge_index);
    for (size_t i_edge = 0; i_edge < n_edges; ++i_edge)
    {
        if(edges_start_index[i_edge] == subsequent_node_index)
        {
            result.push_back(i_edge);
        }
    }

    return result;
}

void LaneGraphTools::move_along_route(
    vector<size_t> route_edge_indices, 
    size_t &edge_index, 
    size_t &edge_path_index, 
    double &delta_s) const
{
    assert(route_edge_indices.size() > 2);
    assert(route_edge_indices[0] == edge_index);
    assert(delta_s >= 0);

    size_t route_index = 0;

    while(1)
    {
        // Do we need to go to the next edge?
        const double remaining_distance_current_edge = 
            edges_s[edge_index][n_edge_path_nodes-1] - edges_s[edge_index][edge_path_index];

        if(remaining_distance_current_edge > delta_s)
        {
            // We are on the final edge
            // Advance path nodes until delta_s is zero
            while(1)
            {
                if(edge_path_index >= n_edge_path_nodes-1) break;
                const double step_length = edges_s[edge_index][edge_path_index+1] - edges_s[edge_index][edge_path_index];
                if(delta_s < step_length) break;
                delta_s -= step_length;
                edge_path_index++;
            }
            return;
        }
        else
        {
            // Need to go to the next edge
            delta_s -= remaining_distance_current_edge;
            route_index++;
            assert(route_index < route_edge_indices.size());
            edge_index = route_edge_indices.at(route_index);
            edge_path_index = 0;
        }

        /*std::cout <<
        "  route_index " << route_index <<
        "  remaining_distance_current_edge " << remaining_distance_current_edge <<
        "  delta_s " << delta_s <<
        "  edge_index " << edge_index <<
        "  edge_path_index " << edge_path_index <<
        std::endl;*/
    }
}