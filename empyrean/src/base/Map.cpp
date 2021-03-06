#include "Map.h"
#include "MapVisitor.h"


namespace pyr {

    class SegmentExtractor : public MapVisitor {
    public:
        SegmentExtractor(std::vector<Segment>& segments, float z)
        : _currentScale(1.0f)
        , _segments(segments)
        , _plane(z) {
        }

        void visitGroup(GroupElement* e) {
            Vec2f oldPosition = _currentPosition;
            float oldScale    = _currentScale;
            _currentPosition += e->pos * _currentScale;
            _currentScale    *= e->scale;

            for (size_t i = 0; i < e->children.size(); i++) {
                e->children[i]->handleVisitor(*this);
            }

            _currentPosition = oldPosition;
            _currentScale    = oldScale;
        }

        void visitGeometry(GeometryElement* e) {
            Vec2f oldPosition = _currentPosition;
            float oldScale    = _currentScale;
            _currentPosition += e->pos * _currentScale;
            _currentScale    *= e->scale;
            Vec3f currentPos(_currentPosition[0], _currentPosition[1], 0);
            

            std::vector<Vec3f>& v = e->vertexArray->positions;

            for (size_t i = 0; i < e->triangles.size(); ++i) {
                // Should only need 2, but in the case of numerical instability...
                Vec2f intersections[3];
                int intersectionCount = 0;
                
                GeometryElement::Triangle& t = e->triangles[i];
                // For each segment in the triangle...
                for (int j = 0; j < 3; ++j) {
                    Vec3f v1 = currentPos + v[t.pos[j]]           * _currentScale;
                    Vec3f v2 = currentPos + v[t.pos[(j + 1) % 3]] * _currentScale;

                    // Intersect each segment with the plane.
                    float dz = v2[2] - v1[2];
                    if (dz == 0) {
                        if (v1[2] == _plane) {
                            add(v1, v2);
                        }
                    } else {
                        float u = (_plane - v1[2]) / dz;
                        if (u >= 0 && u <= 1) {
                            intersections[intersectionCount++] = Vec2f(
                                v1[0] + u * (v2[0] - v1[0]),
                                v1[1] + u * (v2[1] - v1[1]));
                        }
                    }
                }

                if (intersectionCount == 2) {
                    add(intersections[0], intersections[1]);
                }
            }

            _currentPosition = oldPosition;
            _currentScale    = oldScale;
        }

    private:
        void add(const Vec3f& v1, const Vec3f& v2) {
            add(Vec2f(v1[0], v1[1]),
                Vec2f(v2[0], v2[1]));
        }

        void add(const Vec2f& v1, const Vec2f& v2) {
            _segments.push_back(Segment(v1, v2));
        }

        Vec2f _currentPosition;
        float _currentScale;
        std::vector<Segment>& _segments;
        float _plane;
    };


    class MapEnumerator : public MapVisitor {
    public:
        MapEnumerator(std::vector<MapElementPtr>& elts) : _elts(elts) {
        }

        void visitGeometry(GeometryElement* e) {
            _elts.push_back(e);
        }

        void visitGroup(GroupElement* e) {
            _elts.push_back(e);
            for (size_t i = 0; i < e->children.size(); ++i) {
                e->children[i]->handleVisitor(*this);
            }
        }

        void visitDummy(DummyElement* e) {
            _elts.push_back(e);

        }

    private:
        std::vector<MapElementPtr>& _elts;
    };


    MapElementPtr Map::findElementByName(const std::string& name) const {
        std::vector<MapElementPtr> elts;
        getAllElements(elts);
        for (size_t i = 0; i < elts.size(); ++i) {
            if (elts[i]->name == name) {
                return elts[i];
            }
        }
        return 0;
    }

    void Map::getAllElements(std::vector<MapElementPtr>& elts) const {
        MapEnumerator v(elts);
        _root->handleVisitor(v);
    }

    void Map::getSegs(std::vector<Segment>& segs, float xposition) const {
        if (!_isPartitioned) {
            processMap();
            _isPartitioned = true;
        }

        for(size_t i = 0; i < _regions.size(); ++i) {
            if(xposition >= _regions[i].minX &&
                xposition < _regions[i].maxX) { 
                segs = _regions[i]._regionSegs;
                break;
            }
        }
    }
    
    void Map::processMap() const {
        SegmentExtractor extractor(_mapSegs, 0);
        this->handleVisitor(extractor);

        // Calculate the normals for the map #NOTE bug in this (platform bottoms will have normals pointing up 
        for(size_t i = 0; i < _mapSegs.size(); ++i) {
            Vec2f diff = _mapSegs[i].v1 - _mapSegs[i].v2;
            _mapSegs[i].n[0] = -diff[1];
            _mapSegs[i].n[1] = diff[0];
    
            if(_mapSegs[i].n[1] < 0) {
                _mapSegs[i].n *= -1;
            }            
            
            gmtl::normalize(_mapSegs[i].n);
        }


        if (_mapSegs.empty()) {
            return;
        }

        float maxX = std::max(_mapSegs[0].v1[0],_mapSegs[0].v2[0]);
        float minX = std::min(_mapSegs[0].v1[0],_mapSegs[0].v2[0]);

        // Find the extents of the world
        for(size_t i = 1; i < _mapSegs.size(); ++i) {
            float newMax = std::max(_mapSegs[i].v1[0],_mapSegs[i].v2[0]);
            float newMin = std::min(_mapSegs[i].v1[0],_mapSegs[i].v2[0]);
            if(newMax > maxX) {
                maxX = newMax;
            }
            if(newMin < minX) {
                minX = newMin;
            }
        }

        // Geographically divide regions
        const int REGION_COUNT = 32;
        float length = maxX - minX;
        float regionLength = length / (float) REGION_COUNT;

        // Create a predefined number of regions
        for(int i = 0; i < REGION_COUNT; ++i) {
            Region reg;
            _regions.push_back(reg);
        }

        // Create boundaries for these regions
        for(size_t i = 0; i < _regions.size(); ++i) {
            _regions[i].minX = i * regionLength + minX;
            _regions[i].maxX = (i + 1) * regionLength + minX;
        }

        // Region overlap
        const float overlap = 2.0f;

        // Check each segment to see which region it falls into
        for(size_t i = 0; i < _mapSegs.size(); ++i) {
           
            // Check against each region
            for(size_t j = 0; j < _regions.size(); ++j) {
                // Check first point
                if(_mapSegs[i].v1[0] >= (_regions[j].minX - overlap) &&
                    _mapSegs[i].v1[0] < (_regions[j].maxX + overlap)) {
                    
                    _regions[j]._regionSegs.push_back(_mapSegs[i]);
                }
                // Check second point
                else if(_mapSegs[i].v2[0] >= (_regions[j].minX - overlap) &&
                    _mapSegs[i].v2[0] < (_regions[j].maxX + overlap)) {
                    
                    _regions[j]._regionSegs.push_back(_mapSegs[i]);
                }
            }
        }
    }
}
