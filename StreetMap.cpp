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
    void associate(GeoCoord coord1, GeoCoord coord2, string street);
    ExpandableHashMap<GeoCoord,vector<StreetSegment> > m_map;
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
        street = line;
        // Number of segment pairs
        getline(inf,line);
        istringstream num(line);
        num >> segments;
        string lat1, long1, lat2, long2;
        for (int i = 0; i < segments; i++)
        {
            // Get new segments
            getline(inf,line);
            istringstream seg(line);
            // Segments
            seg >> lat1 >> long1;
            GeoCoord coord1(lat1, long1);
            seg >> lat2 >> long2;
            GeoCoord coord2(lat2, long2);
            // Normal segment
            associate(coord1,coord2,street);
            // Reverse segment
            associate(coord2,coord1,street);
        }
    }
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    const vector<StreetSegment>* search = m_map.find(gc);
    if (search == nullptr)
        return false;
    segs = *search;
    return true;
}

void StreetMapImpl::associate(GeoCoord coord1, GeoCoord coord2, string street)
{
    StreetSegment seg(coord1,coord2,street);
    vector<StreetSegment>* exist = m_map.find(coord1);
    if (exist == nullptr)
    {
        vector<StreetSegment> segs;
        segs.push_back(seg);
        m_map.associate(coord1,segs);
    }
    else
    {
        auto it = exist->begin();
        for (; it != exist->end(); it++)
        {
            if (*it == seg)
                break;
        }
        if (it == exist->end())
            exist->push_back(seg);
    }
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
