/** Main include for libgeojson
 *
 *  \file libgeojson.h
 *  \author Dr. Philip Salvaggio (salvaggio.philip@gmail.com)
 *  \date 17 Jan 2020
 */

#pragma once

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <type_traits>

namespace geojson {

enum class Type {
  Point,
  MultiPoint,
  LineString,
  MultiLineString,
  Polygon,
  MultiPolygon,
  GeometryCollection,
  Feature,
  FeatureCollection
};

/** Returns the name of the given geometry type
 *
 *  \param type  The geometry type
 */
inline const char* TypeName(Type type) {
  static constexpr auto kPointName = "Point";
  static constexpr auto kMultiPointName = "MultiPoint";
  static constexpr auto kLineStringName = "LineString";
  static constexpr auto kMultiLineStringName = "MultiLineString";
  static constexpr auto kPolygonName = "Polygon";
  static constexpr auto kMultiPolygonName = "MultiPolygon";
  static constexpr auto kGeometryCollectionName = "GeometryCollection";
  static constexpr auto kFeatureName = "Feature";
  static constexpr auto kFeatureCollectionName = "FeatureCollection";

  switch (type) {
    case Type::Point:
      return kPointName;
    case Type::MultiPoint:
      return kMultiPointName;
    case Type::LineString:
      return kLineStringName;
    case Type::MultiLineString:
      return kMultiLineStringName;
    case Type::Polygon:
      return kPolygonName;
    case Type::MultiPolygon:
      return kMultiPolygonName;
    case Type::GeometryCollection:
      return kGeometryCollectionName;
    case Type::Feature:
      return kFeatureName;
    case Type::FeatureCollection:
      return kFeatureCollectionName;
    default:
      throw std::out_of_range("Invalid geometry type given");
  }
}

/** Returns the name of the given geometry type
 *
 *  \tparam G  The geometry type
 */
template <Type G>
const char* TypeName() {
  return TypeName(G);
}

/** Returns a position array (section 3.1.1)
 *
 *  \param lon  The longitude in decimal degrees
 *  \param lat  The latitude in decimal degrees
 *  \param alt  The altitude in WGS84 ellipsoidal meters
 *
 *  \return A JSON array with the position
 */
inline nlohmann::json Position(double lon, double lat, double alt) {
  return nlohmann::json::array({lon, lat, alt});
}

/** \overload */
inline nlohmann::json Position(double lon, double lat) {
  return nlohmann::json::array({lon, lat});
}

namespace detail {

/** Returns the coordinates array of a Point object (section 3.1.2)
 *
 *  \param lon  The longitude in decimal degrees
 *  \param lat  The latitude in decimal degrees
 *  \param alt  The altitude in WGS84 ellipsoidal meters
 *
 *  \return A JSON array that can go into the coordinates property of a Point
 *          object
 */
inline nlohmann::json PointCoordinates(double lon, double lat, double alt) {
  return Position(lon, lat, alt);
}

/** \overload */
inline nlohmann::json PointCoordinates(double lon, double lat) {
  return Position(lon, lat);
}

/** Returns a GeoJSON object, with a "type" and "coordinates" object */
template <Type T>
inline nlohmann::json CoordinatesObject(nlohmann::json&& coords) {
  return nlohmann::json{{"type", TypeName<T>()},
                        {"coordinates", std::move(coords)}};
}
}  // namespace detail

/** Returns a GeoJSON Point object (section 3.1.2)
 *
 *  \param lon  The longitude in decimal degrees
 *  \param lat  The latitude in decimal degrees
 *  \param alt  The altitude in WGS84 ellipsoidal meters
 *
 *  \return A GeoJSON Point object
 */
inline nlohmann::json Point(double lon, double lat, double alt) {
  return detail::CoordinatesObject<Type::Point>(
      detail::PointCoordinates(lon, lat, alt));
}

/** \overload */
inline nlohmann::json Point(double lon, double lat) {
  return detail::CoordinatesObject<Type::Point>(
      detail::PointCoordinates(lon, lat));
}

namespace detail {

#ifdef __cpp_lib_is_invocable

template <class F, class... Args>
using invoke_result = std::invoke_result<F, Args...>;

template <typename F, typename... Args>
using is_invocable = std::is_invocable<F, Args...>;

template <typename F, typename... Args>
using is_invocable_r = std::is_invocable_r<F, Args...>;

#else

template <class F, class... Args>
using invoke_result = std::result_of<F(Args...)>;

template <typename F, typename... Args>
struct is_invocable
    : std::is_constructible<
          std::function<void(Args...)>,
          std::reference_wrapper<typename std::remove_reference<F>::type> > {};

template <typename R, typename F, typename... Args>
struct is_invocable_r
    : std::is_constructible<
          std::function<R(Args...)>,
          std::reference_wrapper<typename std::remove_reference<F>::type> > {};

#endif

template <class F, class R, class... Args>
using IsCallbackSignature = typename std::enable_if<
    std::is_same<typename detail::invoke_result<F, Args...>::type, R>::value,
    bool>::type;

/** Returns the coordinates array of a MultiPoint object (section 3.1.3)
 *
 *  \tparam Callable A callable of the form
 *                   void(size_t index, double& lon, double& lat,
 *                        double& alt)
 *  \param numPoints The number of points
 *  \param getPoint  A callback that takes the point index and sets the
 *                   lat/lon/altitude
 *
 *  \return A JSON array that can go into the coordinates property of a
 *          MultiPoint object
 */
template <typename Callable,
          detail::IsCallbackSignature<Callable, void, size_t, double&, double&,
                                      double&> = true>
nlohmann::json MultiPointCoordinates(size_t numPoints, Callable&& getPoint) {
  auto coords = nlohmann::json::array();
  double lon, lat, alt;
  for (size_t i = 0; i < numPoints; i++) {
    getPoint(i, lon, lat, alt);
    coords.push_back(PointCoordinates(lon, lat, alt));
  }
  return coords;
}

/** \overload */
template <typename Callable,
          detail::IsCallbackSignature<Callable, void, size_t, double&,
                                      double&> = true>
nlohmann::json MultiPointCoordinates(size_t numPoints, Callable&& getPoint) {
  auto coords = nlohmann::json::array();
  double lon, lat;
  for (size_t i = 0; i < numPoints; i++) {
    getPoint(i, lon, lat);
    coords.push_back(PointCoordinates(lon, lat));
  }
  return coords;
}
}  // namespace detail

/** Returns a GeoJSON MultiPoint object (section 3.1.3)
 *
 *  \tparam Callable A callable of the form
 *                   void(size_t, double&, double&, double&) or
 *                   void(size_t, double&, double&)
 *  \param numPoints The number of points
 *  \param getPoint  A callback that takes the point index and sets the
 *                   lat/lon/altitude
 *
 *  \return A JSON MultiPoint object
 */
template <typename Callable>
nlohmann::json MultiPoint(size_t numPoints, Callable&& getPoint) {
  static_assert(
      detail::is_invocable_r<void, Callable, size_t, double&, double&,
                             double&>::value ||
          detail::is_invocable_r<void, Callable, size_t, double&,
                                 double&>::value,
      "Callback must either be void(size_t, double&, double&, double&) or "
      "void(size_t, double&, double&)");
  return detail::CoordinatesObject<Type::MultiPoint>(
      detail::MultiPointCoordinates(numPoints,
                                    std::forward<Callable>(getPoint)));
}

namespace detail {

/** Returns the coordinates array of a LineString object (section 3.1.4)
 *
 *  \tparam Callable A callable of the form
 *                   void(size_t index, double& lon, double& lat, double& alt)
 *  \param numPoints The number of points
 *  \param getPoint  A callback that takes the point index and sets the
 *                   lat/lon/altitude
 *
 *  \return A JSON array that can go into the coordinates property of a
 *          LineString object
 */
template <typename Callable,
          detail::IsCallbackSignature<Callable, void, size_t, double&, double&,
                                      double&> = true>
nlohmann::json LineStringCoordinates(size_t numPoints, Callable&& getPoint) {
  if (numPoints <= 1) {
    throw std::domain_error("LineString objects must have at least 2 points");
  }
  auto coords = nlohmann::json::array();
  double lon, lat, alt;
  for (size_t i = 0; i < numPoints; i++) {
    getPoint(i, lon, lat, alt);
    coords.push_back(detail::PointCoordinates(lon, lat, alt));
  }
  return coords;
}

/** \overload */
template <typename Callable,
          detail::IsCallbackSignature<Callable, void, size_t, double&,
                                      double&> = true>
nlohmann::json LineStringCoordinates(size_t numPoints, Callable&& getPoint) {
  if (numPoints <= 1) {
    throw std::domain_error("LineString objects must have at least 2 points");
  }
  auto coords = nlohmann::json::array();
  double lon, lat;
  for (size_t i = 0; i < numPoints; i++) {
    getPoint(i, lon, lat);
    coords.push_back(detail::PointCoordinates(lon, lat));
  }
  return coords;
}
}  // namespace detail

/** Returns a GeoJSON LineString object (section 3.1.4)
 *
 *  \tparam Callable A callable of the form
 *                   void(size_t index, double& lon, double& lat, double& alt)
 *                   or void(size_t index, double& lon, double& lat)
 *  \param numPoints The number of points
 *  \param getPoint  A callback that takes the point index and sets the
 *                   lat/lon/altitude
 *
 *  \return A GeoJSON LineString object
 */
template <typename Callable>
nlohmann::json LineString(size_t numPoints, Callable&& getPoint) {
  static_assert(
      detail::is_invocable_r<void, Callable, size_t, double&, double&,
                             double&>::value ||
          detail::is_invocable_r<void, Callable, size_t, double&,
                                 double&>::value,
      "Callback must either be void(size_t, double&, double&, double&) or "
      "void(size_t, double&, double&)");
  return detail::CoordinatesObject<Type::LineString>(
      detail::LineStringCoordinates(numPoints,
                                    std::forward<Callable>(getPoint)));
}

namespace detail {

/** Returns the coordinates array of a MultiLineString object
 * (section 3.1.5)
 *
 *  \tparam GetLineLength A callable of the form size_t(size_t index)
 *  \tparam GetPoint      A callable of the form
 *                        void(size_t lineIndex, size_t pointIndex,
 *                             double& lon, double& lat, double& alt)
 *  \param numLines       The number of lines
 *  \param getLineLength  A callback that takes the line index and returns
 *                        the length of the line
 *  \param getPoint       A callback that takes the line and point indices and
 *                        sets the lat/lon/altitude
 *
 *  \return A JSON array that can go into the coordinates property of a
 *          MultiLineString object
 */
template <typename GetLineLength, typename GetPoint,
          detail::IsCallbackSignature<GetPoint, void, size_t, size_t, double&,
                                      double&, double&> = true>
nlohmann::json MultiLineStringCoordinates(size_t numLines,
                                          GetLineLength&& getLineLength,
                                          GetPoint&& getPoint) {
  auto coords = nlohmann::json::array();
  for (size_t i = 0; i < numLines; i++) {
    coords.push_back(detail::LineStringCoordinates(
        getLineLength(i), [&](size_t j, double& lon, double& lat, double& alt) {
          getPoint(i, j, lon, lat, alt);
        }));
  }
  return coords;
}

/* \overload */
template <typename GetLineLength, typename GetPoint,
          detail::IsCallbackSignature<GetPoint, void, size_t, size_t, double&,
                                      double&> = true>
nlohmann::json MultiLineStringCoordinates(size_t numLines,
                                          GetLineLength&& getLineLength,
                                          GetPoint&& getPoint) {
  auto coords = nlohmann::json::array();
  for (size_t i = 0; i < numLines; i++) {
    coords.push_back(detail::LineStringCoordinates(
        getLineLength(i),
        [&](size_t j, double& lon, double& lat) { getPoint(i, j, lon, lat); }));
  }
  return coords;
}
}  // namespace detail

/** Returns a MultiLineString GeoJSON object (section 3.1.5)
 *
 *  \tparam GetLineLength A callable of the form size_t(size_t index)
 *  \tparam GetPoint      A callable of the form
 *                        void(size_t lineIndex, size_t pointIndex,
 *                             double& lon, double& lat, double& alt)
 *  \param numLineStrings The number of lines
 *  \param getLineLength  A callback that takes the line index and returns
 *                        the length of the line
 *  \param getPoint       A callback that takes the line and point indices and
 *                        sets the lat/lon/altitude
 *
 *  \return A GeoJSON MultiLineString object
 */
template <typename GetLineLength, typename GetPoint>
nlohmann::json MultiLineString(size_t numLineStrings,
                               GetLineLength&& getLineLength,
                               GetPoint&& getPoint) {
  static_assert(
      detail::is_invocable_r<void, GetPoint, size_t, size_t, double&, double&,
                             double&>::value ||
          detail::is_invocable_r<void, GetPoint, size_t, size_t, double&,
                                 double&>::value,
      "GetPoint callback must either be void(size_t, size_t, double&, "
      "double&, double&) or void(size_t, size_t, double&, double&)");
  return detail::CoordinatesObject<Type::MultiLineString>(
      detail::MultiLineStringCoordinates(
          numLineStrings, std::forward<GetLineLength>(getLineLength),
          std::forward<GetPoint>(getPoint)));
}

namespace detail {

/** Tests whether the given position array is counter-clockwise
 *
 *  \param coords  The position coordinate array to test
 *
 *  \return Whether the array is in counter-clockwise order
 */
inline bool IsCcw(const nlohmann::json& coords) {
  // Sum (x2 - x1)(y2 + y1), that will be > 0, if the points are CW
  double cwEdgeSum = 0;
  for (size_t i = 0; i < coords.size(); i++) {
    const auto& pt1 = coords[i];
    const auto& pt2 = coords[(i + 1) % coords.size()];

    cwEdgeSum +=
        (pt2[0].template get<double>() - pt1[0].template get<double>()) *
        (pt2[1].template get<double>() + pt1[1].template get<double>());
  }

  return cwEdgeSum < 0;
}

/** Gets the coordinates array for a linear ring, ensures the vertices are
 * in CW or CCW order.
 *
 *  \tparam GetPoint Either a 2D or 3D point callback
 *  \param numPoints The number of points in the ring
 *  \param ccw       Whether the ring shoudl be CCW
 *  \param getPoint  2D or 3D callback
 *
 *  \return A JSON array containing the positions in the linear ring
 */
template <typename GetPoint>
nlohmann::json LinearRingCoordinates(size_t numPoints, bool ccw,
                                     GetPoint&& getPoint) {
  // We must be at least a triangle
  if (numPoints < 3) {
    throw std::domain_error("Linear rings must have at least 3 points");
  }

  // A linear ring is essentailly a line string
  auto coords =
      LineStringCoordinates(numPoints, std::forward<GetPoint>(getPoint));

  // Reverse if need it
  bool isCcw = IsCcw(coords);
  bool reverse = (isCcw && !ccw) || (!isCcw && ccw);
  if (reverse) {
    for (size_t i = 0; i < coords.size() / 2; i++) {
      std::swap(coords[i], coords[coords.size() - i - 1]);
    }
  }

  // Close the ring
  coords.push_back(coords[0]);

  return coords;
}

/** Returns the coordinates array of a Polygon object (section 3.1.6)
 *
 *  \tparam GetRingLength A callable of the form size_t(size_t index)
 *  \tparam GetPoint      A callable of the form
 *                        void(size_t ringIndex, size_t pointIndex, double& lon,
 *                             double& lat, double& alt)
 *  \param numRings       The number of rings
 *  \param getRingLength  A callback that takes the ring index and returns the
 *                        length of the ring
 *  \param getPoint       A callback that takes the ring and point indices and
 *                        sets the lat/lon/altitude
 *
 *  \return A JSON array that can go into the coordinates property of a
 *          Polygon object
 */
template <typename GetRingLength, typename GetPoint,
          detail::IsCallbackSignature<GetPoint, void, size_t, size_t, double&,
                                      double&, double&> = true>
nlohmann::json PolygonCoordinates(size_t numRings,
                                  GetRingLength&& getRingLength,
                                  GetPoint&& getPoint) {
  nlohmann::json coords;
  for (size_t i = 0; i < numRings; i++) {
    coords.push_back(detail::LinearRingCoordinates(
        getRingLength(i), i == 0,
        [&](size_t j, double& lat, double& lon, double& alt) {
          getPoint(i, j, lat, lon, alt);
        }));
  }

  return coords;
}

/** \overload */
template <typename GetRingLength, typename GetPoint,
          detail::IsCallbackSignature<GetPoint, void, size_t, size_t, double&,
                                      double&> = true>
nlohmann::json PolygonCoordinates(size_t numRings,
                                  GetRingLength&& getRingLength,
                                  GetPoint&& getPoint) {
  nlohmann::json coords;
  for (size_t i = 0; i < numRings; i++) {
    coords.push_back(detail::LinearRingCoordinates(
        getRingLength(i), i == 0,
        [&](size_t j, double& lat, double& lon) { getPoint(i, j, lat, lon); }));
  }

  return coords;
}
}  // namespace detail

/** Returns a Polygon GeoJSON object (section 3.1.6)
 *
 *  \tparam GetRingLength A callable of the form size_t(size_t index)
 *  \tparam GetPoint      A callable of the form
 *                        void(size_t ringIndex, size_t pointIndex, double& lon,
 *                             double& lat, double& alt)
 *  \param numRings       The number of rings
 *  \param getRingLength  A callback that takes the ring index and returns the
 *                        length of the ring
 *  \param getPoint       A callback that takes the ring and point indices and
 *                        sets the lat/lon/altitude
 *
 *  \return A GeoJSON Polygon object
 */
template <typename GetRingLength, typename GetPoint>
nlohmann::json Polygon(size_t numRings, GetRingLength&& getRingLength,
                       GetPoint&& getPoint) {
  static_assert(
      detail::is_invocable_r<void, GetPoint, size_t, size_t, double&, double&,
                             double&>::value ||
          detail::is_invocable_r<void, GetPoint, size_t, size_t, double&,
                                 double&>::value,
      "GetPoint callback must either be void(size_t, size_t, double&, "
      "double&, double&) or void(size_t, size_t, double&, double&)");
  return detail::CoordinatesObject<Type::Polygon>(detail::PolygonCoordinates(
      numRings, std::forward<GetRingLength>(getRingLength),
      std::forward<GetPoint>(getPoint)));
}

namespace detail {

/** Returns the coordinates array of a MultiPolygon object (section 3.1.7)
 *
 *  \tparam GetNumRings   A callable of the form size_t(size_t index)
 *  \tparam GetRingLength A callable of the form
 *                        size_t(size_t polyIndex, size_t ringIndex)
 *  \tparam GetPoint      A callable of the form
 *                        void(size_t polyIndex, size_t ringIndex,
 *                             size_t pointIndex, double& lon, double& lat,
 *                             double& alt)
 *  \param numPolygons    The number of polygons
 *  \param getNumRings    A callback that takes the polygon index and returns
 *                        the number of rings
 *  \param getRingLength  A callback that takes the polygon and ring indices and
 *                        returns the length of the ring
 *  \param getPoint       A callback that takes the polygon, ring and point
 *                        indices and sets the lat/lon/altitude
 *
 *  \return A JSON array that can go into the coordinates property of a
 *          MultiPolygon object
 */
template <typename GetNumRings, typename GetRingLength, typename GetPoint,
          detail::IsCallbackSignature<GetPoint, void, size_t, size_t, size_t,
                                      double&, double&, double&> = true>
nlohmann::json MultiPolygonCoordinates(size_t numPolygons,
                                       GetNumRings&& getNumRings,
                                       GetRingLength&& getRingLength,
                                       GetPoint&& getPoint) {
  nlohmann::json coords;
  for (size_t i = 0; i < numPolygons; i++) {
    coords.push_back(detail::PolygonCoordinates(
        getNumRings(i),
        [&](size_t ring) -> size_t { return getRingLength(i, ring); },
        [&](size_t ring, size_t pt, double& lon, double& lat, double& alt) {
          getPoint(i, ring, pt, lon, lat, alt);
        }));
  }
  return coords;
}

/** \overload */
template <typename GetNumRings, typename GetRingLength, typename GetPoint,
          detail::IsCallbackSignature<GetPoint, void, size_t, size_t, size_t,
                                      double&, double&> = true>
nlohmann::json MultiPolygonCoordinates(size_t numPolygons,
                                       GetNumRings&& getNumRings,
                                       GetRingLength&& getRingLength,
                                       GetPoint&& getPoint) {
  nlohmann::json coords;
  for (size_t i = 0; i < numPolygons; i++) {
    coords.push_back(PolygonCoordinates(
        getNumRings(i),
        [&](size_t ring) -> size_t { return getRingLength(i, ring); },
        [&](size_t ring, size_t pt, double& lon, double& lat) {
          getPoint(i, ring, pt, lon, lat);
        }));
  }
  return coords;
}
}  // namespace detail

/** Returns a MultiPolygon GeoJSON object (section 3.1.7)
 *
 *  \tparam GetNumRings   A callable of the form size_t(size_t index)
 *  \tparam GetRingLength A callable of the form
 *                        size_t(size_t polyIndex, size_t ringIndex)
 *  \tparam GetPoint      A callable of the form
 *                        void(size_t polyIndex, size_t ringIndex,
 *                             size_t pointIndex, double& lon, double& lat,
 *                             double& alt)
 *  \param numPolygons    The number of polygons
 *  \param getNumRings    A callback that takes the polygon index and returns
 *                        the number of rings
 *  \param getRingLength  A callback that takes the polygon and ring indices and
 *                        returns the length of the ring
 *  \param getPoint       A callback that takes the polygon, ring and point
 *                        indices and sets the lat/lon/altitude
 *
 *  \return A GeoJSON MultiPolygon object
 */
template <typename GetNumRings, typename GetRingLength, typename GetPoint>
nlohmann::json MultiPolygon(size_t numPolygons, GetNumRings&& getNumRings,
                            GetRingLength&& getRingLength,
                            GetPoint&& getPoint) {
  static_assert(
      detail::is_invocable_r<void, GetPoint, size_t, size_t, size_t, double&,
                             double&, double&>::value ||
          detail::is_invocable_r<void, GetPoint, size_t, size_t, size_t,
                                 double&, double&>::value,
      "GetPoint callback must either be void(size_t, size_t, size_t, double&, "
      "double&, double&) or void(size_t, size_t, size_t, double&, double&)");
  return detail::CoordinatesObject<Type::MultiPolygon>(
      detail::MultiPolygonCoordinates(
          numPolygons, std::forward<GetNumRings>(getNumRings),
          std::forward<GetRingLength>(getRingLength),
          std::forward<GetPoint>(getPoint)));
}

/** Returns a GeometryCollection object (section 3.1.8)
 *
 *  \tparam Callable      A callable of the form
 *                        nlohmann::json(size_t index)
 *  \param numGeometries  The number of geometries
 *  \param getGeometry    A callback that takes the geometry index and returns
 *                        the geometry object
 *
 *  \return A GeometryCollection JSON object
 */
template <typename Callable>
nlohmann::json GeometryCollection(size_t numGeometries,
                                  Callable&& getGeometry) {
  nlohmann::json j;
  for (size_t i = 0; i < numGeometries; i++) {
    j.push_back(getGeometry(i));
  }

  return nlohmann::json{{"type", TypeName<Type::GeometryCollection>()},
                        {"geometries", j}};
}

/** Returns a Feature object (section 3.2)
 *
 *  \param geometry   A GeoJSON object for the geometry
 *  \param properties A JSON object holding properties for the feature
 *
 *  \return A GeoJSON Feature object
 */
inline nlohmann::json Feature(const nlohmann::json& geometry,
                              const nlohmann::json& properties) {
  return nlohmann::json{{"type", TypeName<Type::Feature>()},
                        {"geometry", geometry},
                        {"properties", properties}};
}

/** Returns a Feature object (section 3.2)
 *
 *  \param id         The ID of the feature
 *  \param geometry   A GeoJSON object for the geometry
 *  \param properties A JSON object holding properties for the feature
 *
 *  \return A GeoJSON Feature object
 */
inline nlohmann::json Feature(const std::string& id,
                              const nlohmann::json& geometry,
                              const nlohmann::json& properties) {
  auto j = Feature(geometry, properties);
  j["id"] = id;
  return j;
}

/** \overload */
template <typename T, typename = typename std::enable_if<
                          std::is_arithmetic<T>::value>::type>
inline nlohmann::json Feature(T id, const nlohmann::json& geometry,
                              const nlohmann::json& properties) {
  auto j = Feature(geometry, properties);
  j["id"] = id;
  return j;
}

/** Returns a FeatureCollection object (section 3.3)
 *
 *  \param numFeatures  The number of features in the collection
 *  \param getFeature   Callback that takes the feature index and gives back the
 *                      feature.
 *
 *  \return A GeoJSON Feature object
 */
template <typename Callback>
inline nlohmann::json FeatureCollection(size_t numFeatures,
                                        Callback&& getFeature) {
  static_assert(detail::is_invocable_r<nlohmann::json, Callback, size_t>::value,
                "Callback must be of the form nlohmann::json(size_t)");

  nlohmann::json j{{"type", TypeName<Type::FeatureCollection>()}};

  auto& featuresJ = j["features"];
  featuresJ = nlohmann::json::array();
  for (size_t i = 0; i < numFeatures; i++) {
    featuresJ.push_back(getFeature(i));
  }
  return j;
}
}  // namespace geojson
