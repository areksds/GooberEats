#include "provided.h"
#include <vector>
#include <algorithm>
using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private:
    double crowDistance(const GeoCoord& start, vector<DeliveryRequest>& paths) const;
    const StreetMap* sm;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm) : sm(sm) {}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    // Calculate oldCrowDistance
    oldCrowDistance = crowDistance(depot,deliveries);
    
    // Simulated annealing
    newCrowDistance = oldCrowDistance;
    vector<DeliveryRequest> current = deliveries;
    double q = 0.9;
    int i = 0;
    while (i < 5)
    {
        double oldDistance = crowDistance(depot,current);
        vector<DeliveryRequest> newCombo = current;
        random_shuffle(newCombo.begin(),newCombo.end());
        double newDistance = crowDistance(depot,newCombo);
        if (newDistance < oldDistance) {
            i = 0;
            current = newCombo;
            if (newDistance < newCrowDistance)
            {
                newCrowDistance = newDistance;
                deliveries = current;
            }
        } else if (newDistance >= oldDistance) {
            double r = (rand() % 100)/10;
            if (r < q)
                current = newCombo;
            i++;
        }
        q *= .999;
    }
}

double DeliveryOptimizerImpl::crowDistance(const GeoCoord& start, vector<DeliveryRequest>& paths) const
{
    double crowDistance = 0;
    GeoCoord current = start;
    for (int i = 0; i != paths.size(); i++)
    {
        crowDistance += distanceEarthMiles(current, paths[i].location);
        current = paths[i].location;
    }
    return crowDistance;
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
