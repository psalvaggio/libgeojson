# GeoJSON for Modern C++

![CI](https://github.com/psalvaggio/libgeojson/workflows/CI/badge.svg)
![Code Quality](https://github.com/psalvaggio/libgeojson/workflows/Code%20Quality/badge.svg)

This library serves as wrapper around [JSON for Modern C++](https://github.com/nlohmann/json) for the GeoJSON ([RFC 7946](https://tools.ietf.org/html/rfc7946)) standard. This library provides a set of functions to create GeoJSON objects in the form of `nlohmann::json` objects. This library does not assume any geometry data types and instead uses a callback interface to allow the user to encode their own data types into GeoJSON.

For all positions in this library, the longitude and latitude are in decimal degrees and altitude is a WGS84 ellipsoidal altitude in meters.

## Serialization

### Point

In order to encode a Point, use `geojson::Point()`. For example,

```cpp
auto j2d = geojson::Point(lon, lat);
auto j3d = geojson::Point(lon, lat, alt);
```

### MultiPoint

In order to encode a MultiPoint, use `geojson::MultiPoint()`. For example,

```cpp
std::vector<std::array<double, 2>> pts{
    {-40.5, 32.2}, {-39, 33.1}, {-39.5, 33.02}
};
auto j = geojson::MultiPoint(pts.size(), [&pts](size_t pt, double& lon, double& lat) {
  lon = pts[pt][0];
  lat = pts[pt][1];
});
```
The callback can also take a third `double&` for altitude.

### LineString

In order to encode a LineString, use `geojson::LineString()`. For example,

```cpp
std::vector<std::array<double, 2>> pts{
    {-40.5, 32.2}, {-39, 33.1}, {-39.5, 33.02}
};
auto j = geojson::LineString(pts.size(), [&pts](size_t pt, double& lon, double& lat) {
  lon = pts[pt][0];
  lat = pts[pt][1];
});
```
The callback can also take a third `double&` for altitude.

### MultiLineString

In order to encode a MultiLineString, use `geojson::MultiLineString()`. The first callback is used to determine the length of each line string and the second is used for getting the points in each line string. For example,

```cpp
std::vector<std::vector<std::array<double, 2>>> pts{
    {{-40.5, 32.2}, {-39, 33.1}, {-39.5, 33.02}},
    {{-35.1, 10}, {-35.2, 10.1}}
};
auto j = geojson::MultiLineString(pts.size(),
    [&pts](size_t line) { return pts[line].size(); },
    [&pts](size_t line, size_t pt, double& lon, double& lat) {
      lon = pts[line][pt][0];
      lat = pts[line][pt][1];
    });
```
The callback can also take a third `double&` for altitude.

### Polygon

In order to encode a Polygon, use `geojson::Polygon()`. The first callback is used to determine the length of each linear ring and the second is used for getting the points in each ring. The function handles the clockwise and counter-clockwise ordering, as well as closing the rings. For example,
```cpp
std::vector<std::array<double, 3>> outer{
    {0, 0, 0.5}, {1.5, 0, 0.3}, {1.5, 1.5, 0.6}, {0, 1.5, 0.9}};
std::vector<std::vector<std::array<double, 3>>> inners{
    {{0.25, 0.25, 0.5}, {0.35, 0.75, 0.6}, {0.5, 0.25, 0.7}},
    {{1, 0.25, 0.5}, {1.25, 0.25, 0.6}, {1.125, 0.5, 0.7}}};

auto j = geojson::Polygon(
    inners.size() + 1,
    [&](size_t ring) {
      return ring == 0 ? outer.size() : inners[ring - 1].size();
    },
    [&](size_t ring, size_t pt, double& lon, double& lat,
                    double& alt) {
      if (ring == 0) {
        lon = outer[pt][0];
        lat = outer[pt][1];
        alt = outer[pt][2];
      } else {
        lon = inners[ring - 1][pt][0];
        lat = inners[ring - 1][pt][1];
        alt = inners[ring - 1][pt][2];
      }
    ));
```
If you don't want altitude, the last `double&` can be omitted.

## MultiPolygon

In order to encode a MultiPolygon, use `geojson::MultiPolygon()`. The first callback is used to determine the number of rings in each polygon, the second is used to determine the length of each linear ring in each polygon and the third is used for getting the points in each ring. The function handles the clockwise and counter-clockwise ordering, as well as closing the rings. For example,
```cpp
std::vector<std::vector<std::array<double, 3>>> outers{
    {{0, 0, 0.5}, {1.5, 0, 0.3}, {1.5, 1.5, 0.6}, {0, 1.5, 0.9}},
    {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}};
std::vector<std::vector<std::vector<std::array<double, 3>>>> inners{
    {{{0.25, 0.25, 0.5}, {0.35, 0.75, 0.6}, {0.5, 0.25, 0.7}},
     {{1, 0.25, 0.5}, {1.25, 0.25, 0.6}, {1.125, 0.5, 0.7}}},
    {}};

auto j = geojson::MultiPolygon(
    outers.size(),
    [&](size_t poly) { return inners[poly].size() + 1; },
    [&](size_t poly, size_t ring) {
      return ring == 0 ? outers[poly].size() : inners[poly][ring - 1].size();
    },
    [&](size_t poly, size_t ring, size_t pt, double& lon, double& lat,
        double& alt) {
      if (ring == 0) {
        lon = outers[poly][pt][0];
        lat = outers[poly][pt][1];
        alt = outers[poly][pt][2];
      } else {
        lon = inners[poly][ring - 1][pt][0];
        lat = inners[poly][ring - 1][pt][1];
        alt = inners[poly][ring - 1][pt][2];
      }
    });
```
If you don't want altitude, the last `double&` can be omitted.

## Feature

In order to encode a Feature, use `geojson::Feature()`. For example,
```cpp
struct Props {
  Props(const std::string& _name) : name(_name) {}
  std::string name;
};

void to_json(nlohmann::json& j, const Props& props) {
  return nlohmann::json{{"name", props.name}};
}

auto j1 = geojson::Feature("id1", geojson::Point(-40, 30),
                           nlohmann::json(Props("foo")));
auto j2 = geojson::Feature(2, geojson::Point(-40, 30),
                           nlohmann::json(Props("foo2")));
auto j3 = geojson::Feature(geojson::Point(-40, 30),
                           nlohmann::json(Props("foo2")));
```
In the case of `j3`, no `"id"` element will be added to the feature.

## FeatureCollection

In order to encode a Feature, use `geojson::FeatureCollection()`. The callback is used to get the features in the collection. For example,

```cpp
std::vector<std::array<double, 2>> pts{
    {-40.5, 32.2}, {-39, 33.1}, {-39.5, 33.02}
};

auto j = geojson::FeatureCollection(pts.size(), [&](size_t i) {
  return geojson::Feature(
      i, geojson::Point(pts[i][0], pts[i][1]),
      nlohmann::json(Props(std::string("foo") + std::to_string(i))));
});
```
