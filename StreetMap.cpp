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
            StreetSegment first(coord1,coord2,street);
            vector<StreetSegment>* exist1 = m_map.find(coord1);
            if (exist1 == nullptr)
            {
                vector<StreetSegment> segs;
                segs.push_back(first);
                m_map.associate(coord1,segs);
            }
            else
            {
                auto it = exist1->begin();
                for (; it != exist1->end(); it++)
                {
                    if (*it == first)
                        break;
                }
                if (it == exist1->end())
                    exist1->push_back(first);
            }
            // Reverse segment
            StreetSegment reverse(coord2,coord1,street);
            vector<StreetSegment>* exist2 = m_map.find(coord2);
            if (exist2 == nullptr)
            {
                vector<StreetSegment> segs;
                segs.push_back(reverse);
                m_map.associate(coord2,segs);
            }
            else
            {
                auto it = exist2->begin();
                for (; it != exist2->end(); it++)
                {
                    if (*it == reverse)
                        break;
                }
                if (it == exist2->end())
                    exist2->push_back(first);
            }
            
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
