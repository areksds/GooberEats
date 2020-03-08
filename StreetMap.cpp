#include "provided.h"
#include "ExpandableHashMap.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <functional>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
  public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
  private:
    ExpandableHashMap<GeoCoord,vector<StreetSegment>> m_map;
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}

bool StreetMapImpl::load(string mapFile)
{
    ifstream inf(mapFile);
    if (!inf)
    {
        cout << "Cannot open map data file!" << endl;
        return false;
    }
    string line, street;
    int segments;
    while (getline(inf,line))
    {
        vector<StreetSegment> segs;
        // Street name
        istringstream str(line);
        str >> street;
        // Number of segment pairs
        getline(inf,line);
        istringstream num(line);
        num >> segments;
        string lat1, long1, lat2, long2;
        GeoCoord key;
        for (int i = 0; i <= segments; i++)
        {
            getline(inf,line);
            istringstream seg(line);
            if (i > 0)
            {
                // Reverse segments
                GeoCoord coord1(lat2, long2);
                GeoCoord coord2(lat1, long1);
                StreetSegment seg(coord1,coord2,street);
                segs.push_back(seg);
                if (i == segments)
                    GeoCoord key(lat2,long2);
            }
            if (i < segments)
            {
                // Segments
                seg >> lat1 >> long1;
                GeoCoord coord1(lat1, long1);
                seg >> lat2 >> long2;
                GeoCoord coord2(lat2, long2);
                StreetSegment seg(coord1,coord2,street);
                segs.push_back(seg);
                GeoCoord key(lat1,long2);
            }
            m_map.associate(key, segs);
        }
    }
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    return false;  // Delete this line and implement this function correctly
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
