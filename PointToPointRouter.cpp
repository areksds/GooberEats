#include "provided.h"
#include <list>
#include <queue>
#include <map>
#include <set>
using namespace std;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
    struct queuedCoord
    {
        queuedCoord(GeoCoord coord, double priority) : coord(coord), priority(priority) {}
        bool operator==(const queuedCoord& rhs) const
        {
            return coord == rhs.coord;
        }
        GeoCoord coord;
        double priority;
    };
    struct greaterPriority
    {
        bool operator()(const queuedCoord& lhs, const queuedCoord& rhs) const
        {
            return lhs.priority < rhs.priority;
        }
    };
    const StreetMap* m_map;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm) : m_map(sm)
{
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    vector<StreetSegment> begin;
    vector<StreetSegment> last;
    
    // Check if GeoCoords are valid
    if (!m_map->getSegmentsThatStartWith(start, begin) ||
        !m_map->getSegmentsThatStartWith(end, last))
        return BAD_COORD;
    
    // Segments to check
    set<queuedCoord,greaterPriority> openList;
    set<GeoCoord> closedList;
            
    // List of g's for each coordinate
    map<GeoCoord,double> g;
    
    // Map of path
    map<GeoCoord,StreetSegment> path;
    
    queuedCoord first(start,0);
    g[start] = 0;
    openList.insert(first);
    
    // A*
    while (openList.begin()->coord != end)
    {
        if (openList.empty())
            break;
        GeoCoord current = openList.begin()->coord;
        openList.erase(openList.begin());
        closedList.insert(current);
        vector<StreetSegment> segments;
        m_map->getSegmentsThatStartWith(current, segments);
        for (int i = 0; i != segments.size(); i++)
        {
            double newg = g[current] + distanceEarthMiles(segments[i].start, segments[i].end);
            queuedCoord child(segments[i].end,newg + distanceEarthMiles(segments[i].end,end));
            auto openCheck = openList.find(child);
            bool openExist = false;
            if (openCheck != openList.end())
            {
                if (g[openCheck->coord] > newg)
                {
                    g.erase(openCheck->coord);
                    openList.erase(openCheck);
                }
                else
                   openExist = true;
            }
            auto closedCheck = closedList.find(segments[i].end);
            bool closedExist = false;
            if (closedCheck != closedList.end())
            {
                if (g[*closedCheck] > newg)
                {
                    g.erase(*closedCheck);
                    closedList.erase(closedCheck);
                }
                else
                    closedExist = true;
            }
            if (!openExist && !closedExist)
            {
                openList.insert(child);
                g[segments[i].end] = newg;
                path[child.coord] = segments[i];
            }
        }
    }
            
    if (openList.empty())
        return NO_ROUTE;
    
    GeoCoord current = end;
    while (current != start)
    {
        route.push_front(path[current]);
        totalDistanceTravelled += distanceEarthMiles(path[current].end, path[current].start);
        current = path[current].start;
    }
    
    return DELIVERY_SUCCESS;
    
}


//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
