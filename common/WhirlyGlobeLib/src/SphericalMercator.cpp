/*  SphericalMercator.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/19/12.
 *  Copyright 2011-2022 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#import "SphericalMercator.h"
#import "GlobeMath.h"

namespace WhirlyKit
{

SphericalMercatorCoordSystem::SphericalMercatorCoordSystem(float originLon)
    : originLon(originLon)
{
}

// Keep things right below/above the poles
static constexpr double PoleLimit = DegToRad(85.05113);
    
/// Convert from the local coordinate system to lat/lon
GeoCoord SphericalMercatorCoordSystem::localToGeographic(Point3f pt) const
{
    GeoCoord coord;
    coord.lon() = pt.x() + originLon;
    coord.lat() = atanf(sinhf(pt.y()));
    
    return coord;    
}

GeoCoord SphericalMercatorCoordSystem::localToGeographic(Point3d pt) const
{
    GeoCoord coord;
    coord.lon() = pt.x() + originLon;
    coord.lat() = atan(sinh(pt.y()));
    
    return coord;    
}

Point2d SphericalMercatorCoordSystem::localToGeographicD(Point3d pt) const
{
    Point2d coord;
    coord.x() = pt.x() + originLon;
    coord.y() = atan(sinh(pt.y()));
    
    return coord;
}

/// Convert from lat/lon t the local coordinate system
Point3f SphericalMercatorCoordSystem::geographicToLocal(GeoCoord geo) const
{
    Point3f coord;
    coord.x() = geo.lon() - originLon;
    float lat = geo.lat();
    if (lat < -PoleLimit) lat = -PoleLimit;
    if (lat > PoleLimit) lat = PoleLimit;
    coord.y() = logf((1.0f+sinf(lat))/cosf(lat));
    coord.z() = 0.0;
    
    return coord;    
}

Point3d SphericalMercatorCoordSystem::geographicToLocal3d(GeoCoord geo) const
{
    Point3d coord;
    coord.x() = geo.lon() - originLon;
    double lat = geo.lat();
    if (lat < -PoleLimit) lat = -PoleLimit;
    if (lat > PoleLimit) lat = PoleLimit;
    coord.y() = log((1.0f+sin(lat))/cos(lat));
    coord.z() = 0.0;
    
    return coord;    
}

Point3d SphericalMercatorCoordSystem::geographicToLocal(Point2d geo) const
{
    const double lat = std::min(PoleLimit, std::max(-PoleLimit, geo.y()));
    return { geo.x() - originLon,
             std::log((1.0 + std::sin(lat)) / std::cos(lat)),
             0.0 };
}

Point2d SphericalMercatorCoordSystem::geographicToLocal2(const Point2d &geo) const
{
    const double lat = std::min(PoleLimit, std::max(-PoleLimit, geo.y()));
    return { geo.x() - originLon, std::log((1.0 + std::sin(lat)) / std::cos(lat)) };
}

/// Convert from the local coordinate system to geocentric
Point3f SphericalMercatorCoordSystem::localToGeocentric(Point3f localPt) const
{
    const GeoCoord geoCoord = localToGeographic(localPt);
    return GeoCoordSystem::LocalToGeocentric(Point3f(geoCoord.x(),geoCoord.y(),localPt.z()));
}

Point3d SphericalMercatorCoordSystem::localToGeocentric(Point3d localPt) const
{
    const Point2d geoCoord = localToGeographicD(localPt);
    return GeoCoordSystem::LocalToGeocentric(Point3d(geoCoord.x(),geoCoord.y(),localPt.z()));
}
    
/// Convert from display coordinates to geocentric
Point3f SphericalMercatorCoordSystem::geocentricToLocal(Point3f geocPt) const
{
    const Point3f geoCoordPlus = GeoCoordSystem::GeocentricToLocal(geocPt);
    const Point3f localPt = geographicToLocal(GeoCoord(geoCoordPlus.x(),geoCoordPlus.y()));
    return {localPt.x(),localPt.y(),geoCoordPlus.z()};
}

Point3d SphericalMercatorCoordSystem::geocentricToLocal(Point3d geocPt) const
{
    const Point3d geoCoordPlus = GeoCoordSystem::GeocentricToLocal(geocPt);
    const Point3d localPt = geographicToLocal3d(GeoCoord(geoCoordPlus.x(),geoCoordPlus.y()));
    return {localPt.x(),localPt.y(),geoCoordPlus.z()};
}

bool SphericalMercatorCoordSystem::isSameAs(const CoordSystem *coordSys) const
{
    const auto other = dynamic_cast<const SphericalMercatorCoordSystem *>(coordSys);
    return other && other->originLon == originLon;
}


SphericalMercatorDisplayAdapter::SphericalMercatorDisplayAdapter(float originLon,GeoCoord geoLL,GeoCoord geoUR) :
    CoordSystemDisplayAdapter(&smCoordSys,Point3d(0,0,0)),
    smCoordSys(originLon),
    geoLL(geoLL.lon(),geoLL.lat()),
    geoUR(geoUR.lon(),geoUR.lat())
{
    const Point3d ll3d = smCoordSys.geographicToLocal3d(geoLL);
    const Point3d ur3d = smCoordSys.geographicToLocal3d(geoUR);
    ll.x() = ll3d.x();  ll.y() = ll3d.y();
    ur.x() = ur3d.x();  ur.y() = ur3d.y();
    
    org = (ll+ur)/2.0;
}
    
SphericalMercatorDisplayAdapter::SphericalMercatorDisplayAdapter(float originLon,GeoCoord geoLL,GeoCoord geoUR,Point3d displayOrigin) :
    CoordSystemDisplayAdapter(&smCoordSys,displayOrigin),
    smCoordSys(originLon),
    geoLL(geoLL.lon(),geoLL.lat()),
    geoUR(geoUR.lon(),geoUR.lat())
{
    const Point3d ll3d = smCoordSys.geographicToLocal3d(geoLL);
    const Point3d ur3d = smCoordSys.geographicToLocal3d(geoUR);
    ll.x() = ll3d.x();  ll.y() = ll3d.y();
    ur.x() = ur3d.x();  ur.y() = ur3d.y();
    
    org = (ll+ur)/2.0;
}
    
/// Return the valid boundary in spherical mercator.  Z coordinate is ignored at present.
bool SphericalMercatorDisplayAdapter::getBounds(Point3f &outLL,Point3f &outUR) const
{
    outLL.x() = ll.x();  outLL.y() = ll.y();  outLL.z() = 0.0;
    outUR.x() = ur.x();  outUR.y() = ur.y();  outUR.z() = 0.0;
    
    return true;
}

/// Convert from the system's local coordinates to display coordinates
WhirlyKit::Point3f SphericalMercatorDisplayAdapter::localToDisplay(WhirlyKit::Point3f localPt) const
{
    Point3f dispPt = localPt-Point3f(org.x(),org.y(),0.0);
    return dispPt;
}

WhirlyKit::Point3d SphericalMercatorDisplayAdapter::localToDisplay(WhirlyKit::Point3d localPt) const
{
    Point3d dispPt = localPt-Point3d(org.x(),org.y(),0.0);
    return dispPt;
}
    
/// Convert from display coordinates to the local system's coordinates
WhirlyKit::Point3f SphericalMercatorDisplayAdapter::displayToLocal(WhirlyKit::Point3f dispPt) const
{
    Point3f localPt = dispPt+Point3f(org.x(),org.y(),0.0);
    return localPt;
}

WhirlyKit::Point3d SphericalMercatorDisplayAdapter::displayToLocal(WhirlyKit::Point3d dispPt) const
{
    Point3d localPt = dispPt+Point3d(org.x(),org.y(),0.0);
    return localPt;
}

}
