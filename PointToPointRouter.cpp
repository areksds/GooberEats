#include "provided.h"
#include <list>
#include <queue>
#include <unordered_map>
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
    struct AnalyzedCoord
    {
        AnalyzedCoord(const GeoCoord* coord, const GeoCoord* parent = nullptr, double g = 0, double h = 0) : coord(coord), parent(parent), g(g), h(h) {}
        const GeoCoord* coord;
        const GeoCoord* parent;
        double g;
        double h;
        double f() const
        {
            return g + h;
        }
        bool operator<(const AnalyzedCoord rhs) const
        {
            return f() < rhs.f();
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
    priority_queue<AnalyzedCoord> openList;
    set<AnalyzedCoord> openListCheck;
    set<AnalyzedCoord> closedList;
    
    // Map of path
    unordered_map<GeoCoord,StreetSegment> path;
    
    AnalyzedCoord first(&start);
    
    openList.push(first);
    openListCheck.insert(first);
    
    while (!openList.empty())
    {
        AnalyzedCoord parent = openList.top();
        openList.pop();
        openListCheck.erase(parent);
        vector<StreetSegment> segments;
        m_map->getSegmentsThatStartWith(*parent.coord, segments);
        for (int i = 0; i != segments.size(); i++)
        {
            // Path found
            if (segments[i].end == end)
            {
                path[segments[i].end] = segments[i];
                GeoCoord current = end;
                while (current != start)
                {
                    route.push_front(path[current]);
                    current = path[current].start;
                }
                return DELIVERY_SUCCESS;
            }
            
            // Coordinate with appropriate g and h values
            AnalyzedCoord child(&segments[i].end,&segments[i].start,parent.g + distanceEarthMiles(segments[i].start, segments[i].end),distanceEarthMiles(segments[i].end, end));
            
            // Check if current in lists
            if (openListCheck.find(child) != openListCheck.end() && openListCheck.find(child)->f() < child.f())
                continue;
            if (closedList.find(child) != closedList.end() && closedList.find(child)->f() < child.f())
                continue;
            
            // Add to list
            openList.push(child);
            openListCheck.insert(child);
            path[*child.coord] = segments[i];
        }
        closedList.insert(parent);
    }
    
    // No path found
    return NO_ROUTE;
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
