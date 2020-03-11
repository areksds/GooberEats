#include "provided.h"
#include <string>
#include <vector>
#include <list>
#include <algorithm>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    string dir(double angle) const;
    const StreetMap* sm;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm) : sm(sm) {}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    
    // DELIVERY OPTIMIZER
    vector<DeliveryRequest> optimizedDeliveries;
    optimizedDeliveries = deliveries;
    /*
    DeliveryOptimizer optimizer(sm);
    double oldCrow, newCrow;
    optimizer.optimizeDeliveryOrder(depot, optimizedDeliveries, oldCrow, newCrow);
     */
    
    PointToPointRouter router(sm);
    vector<list<StreetSegment>> paths = vector<list<StreetSegment>>(optimizedDeliveries.size() + 1);
    
    // Start from depot
    GeoCoord current = depot;
    
    // Generate street segments
    for (int i = 0; i < optimizedDeliveries.size(); i++)
    {
        double distance = 0;
        DeliveryResult generate = router.generatePointToPointRoute(current, optimizedDeliveries[i].location, paths[i], distance);
        switch (generate)
        {
            case DELIVERY_SUCCESS:
                totalDistanceTravelled += distance;
                current = optimizedDeliveries[i].location;
                break;
            case BAD_COORD:
                return BAD_COORD;
            case NO_ROUTE:
                return NO_ROUTE;
        }
    }
    
    // Return path
    double distance;
    router.generatePointToPointRoute(optimizedDeliveries[optimizedDeliveries.size() - 1].location, depot, paths[optimizedDeliveries.size()], distance);
    totalDistanceTravelled += distance;
    
    // Turn segments into commands
    for (int i = 0; i < paths.size(); i++)
    {
        string street = paths[i].begin()->name;
        string direction = dir(angleOfLine(*paths[i].begin()));
        double distance = 0;
        for (auto it = paths[i].begin(); it != paths[i].end(); it++)
        {
            if (it->name != street)
            {
                // Process length on one street
                DeliveryCommand dc;
                dc.initAsProceedCommand(direction, street, distance);
                commands.push_back(dc);
                
                // Add new stretch of street
                street = it->name;
                direction = dir(angleOfLine(*it));
                distance = 0;
                
                // Check for turns
                double turn = angleBetween2Lines(*(--it), *(++it));
                if (turn >= 1 && turn < 180)
                {
                    DeliveryCommand dt;
                    dt.initAsTurnCommand("left", street);
                    commands.push_back(dt);
                }
                else if (turn >= 180 && turn <= 359)
                {
                    DeliveryCommand dt;
                    dt.initAsTurnCommand("right", street);
                    commands.push_back(dt);
                }
            }
            else
                distance += distanceEarthMiles(it->start, it->end);
        }
        DeliveryCommand dc;
        dc.initAsProceedCommand(direction, street, distance);
        commands.push_back(dc);
        if (i != optimizedDeliveries.size())
        {
            DeliveryCommand dv;
            dv.initAsDeliverCommand(optimizedDeliveries[i].item);
            commands.push_back(dv);
        }
    }
    
    
    return DELIVERY_SUCCESS;
}

string DeliveryPlannerImpl::dir(double angle) const
{
    if (angle < 0)
        return "";
    if (angle < 22.5)
        return "east";
    if (angle < 67.5)
        return "northeast";
    if (angle < 112.5)
        return "north";
    if (angle < 157.5)
        return "northwest";
    if (angle < 202.5)
        return "west";
    if (angle < 247.5)
        return "southwest";
    if (angle < 292.5)
        return "south";
    if (angle < 337.5)
        return "southeast";
    if (angle >= 337.5)
        return "east";
    return "";
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
