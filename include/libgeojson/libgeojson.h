/** Main include for libgeojson
 *
 *  \file libgeojson.h
 *  \author Dr. Philip Salvaggio (salvaggio.philip@gmail.com)
 *  \date 17 Jan 2020
 */

#pragma once

#include <array>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <variant>

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

// ----- GeoJSON writing -------------------------------------------------------

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

// ----- GeoJSON reading -------------------------------------------------------

/** Controls how missing altitude values are handled when a 3-D callback is
 *  used and a 2-D position [lon, lat] is encountered in the JSON.
 *
 *  Has no effect when a 2-D callback (no alt parameter) is supplied.
 */
enum class ZStrategy {
  DefaultZero,    ///< Set alt = 0.0.
  Throw,          ///< Throw ParseError at the offending position.
  Uninitialized,  ///< Leave alt uninitialised. Caller guarantees all
                  ///< positions in the document are 3-D.
                  ///< \warning Undefined behaviour if a 2-D position
                  ///<          is actually present.
};

// Error handling
namespace detail {

/** All GeoJSON key and type-name string constants used in path components.
 *
 *  Centralised here so every path step comes from static storage, making
 *  string_view storage in PathBuffer safe.
 */
struct Keys {
  static constexpr std::string_view type = "type";
  static constexpr std::string_view coordinates = "coordinates";
  static constexpr std::string_view geometries = "geometries";
  static constexpr std::string_view geometry = "geometry";
  static constexpr std::string_view properties = "properties";
  static constexpr std::string_view features = "features";
  static constexpr std::string_view id = "id";
};

using path_component_t = std::variant<std::string_view, std::size_t>;

inline constexpr std::size_t kMaxPathDepth = 8;

/** Fixed-depth path buffer — stack allocated, no heap.
 *
 *  Passed by reference through the JSON descent. Use PathGuard to push and
 *  pop steps automatically so that every throw/return path stays consistent.
 */
class PathBuffer {
 public:
  constexpr PathBuffer() = default;

  constexpr void push(std::size_t idx) noexcept {
    if (depth_ < kMaxPathDepth) components_[depth_++] = idx;
  }

  constexpr void push(std::string_view key) noexcept {
    if (depth_ < kMaxPathDepth) components_[depth_++] = key;
  }

  constexpr void pop() noexcept {
    if (depth_ > 0) --depth_;
  }

  constexpr size_t depth() const noexcept { return depth_; }
  constexpr const path_component_t& operator[](size_t i) const noexcept {
    return components_[i];
  }
  constexpr const path_component_t* begin() const noexcept {
    return components_.data();
  }
  constexpr const path_component_t* end() const noexcept {
    return components_.data() + depth_;
  }

 private:
  std::uint8_t depth_ = 0;
  std::array<path_component_t, kMaxPathDepth> components_;
};

/// RAII guard — pushes a step on construction, pops on destruction.
/// Eliminates the need to manually call pop() before every return/throw path.
class PathGuard {
 public:
  PathGuard(PathBuffer& path, std::uint8_t idx) : path_(path) {
    path_.push(idx);
  }
  PathGuard(PathBuffer& path, std::string_view key) : path_(path) {
    path_.push(key);
  }

  ~PathGuard() { path_.pop(); }

  PathGuard(const PathGuard&) = delete;
  PathGuard& operator=(const PathGuard&) = delete;

 private:
  PathBuffer& path_;
};
}  // namespace detail

class ParseError : public std::runtime_error {
 public:
  ParseError(const detail::PathBuffer& path, const std::string& msg)
      : std::runtime_error(_build(path, msg)), path_(path) {}

  const detail::PathBuffer& path() const noexcept { return path_; }

  std::string pointer() const { return _pointer(path_); }

 private:
  detail::PathBuffer path_;

  static std::string _pointer(const detail::PathBuffer& p) {
    if (p.depth() == 0) return "/";
    std::string out;
    for (const auto& step : p) {
      out += '/';
      std::visit(
          [&out](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::string_view>)
              out += v;
            else
              out += std::to_string(v);
          },
          step);
    }
    return out;
  }

  static std::string _build(const detail::PathBuffer& p,
                            const std::string& msg) {
    return "at " + _pointer(p) + ": " + msg;
  }
};

// ---- Validation and callback traits -----------------------------------------

namespace detail {

/// Asserts j is of the expected JSON value type; throws ParseError otherwise.
inline void AssertJsonType(const nlohmann::json& j,
                           nlohmann::json::value_t expected,
                           const PathBuffer& path) {
  if (j.type() != expected)
    throw ParseError(path, std::string("expected ") +
                               nlohmann::json(expected).type_name() + ", got " +
                               j.type_name());
}

/// Asserts j is a JSON number (integer, unsigned, or float).
template <typename T = double>
inline T AssertNumber(const nlohmann::json& j, const PathBuffer& path) {
  if (!j.is_number())
    throw ParseError(path,
                     std::string("expected number, got ") + j.type_name());
  return j.get<T>();
}

template <typename T = std::string_view>
inline T AssertString(const nlohmann::json& j, const PathBuffer& path) {
  AssertJsonType(j, nlohmann::json::value_t::string, path);
  return j.get<T>();
}

/// Asserts j contains key and that its value is not null; throws ParseError
/// with path pointing at the key if either condition fails.
inline const nlohmann::json& RequiredKey(const nlohmann::json& j,
                                         std::string_view key,
                                         PathBuffer& path) {
  if (auto it = j.find(key); it != j.end()) return *it;

  PathGuard g(path, key);
  throw ParseError(path, "property is required");
}

/// Asserts j["type"] equals the name of the expected GeoJSON type.
inline void AssertGeoJsonType(const nlohmann::json& j, Type expected,
                              PathBuffer& path) {
  AssertJsonType(j, nlohmann::json::value_t::object, path);
  const auto& t = RequiredKey(j, Keys::type, path);

  PathGuard g(path, Keys::type);
  auto actual = AssertString(t, path);
  if (actual != TypeName(expected))
    throw ParseError(path, std::string("expected GeoJSON type '") +
                               TypeName(expected) + "', got '" +
                               std::string(actual) + '\'');
}

// ---- Callback shape traits --------------------------------------------------
//
// For each geometry level we define a 2-D and a 3-D callback trait, then
// mutually exclusive enable_if aliases used in template parameter lists.
//
// Index arities:
//   Point / MultiPoint          : (size_t pt)
//   LineString                  : (size_t pt)
//   MultiLineString             : (size_t line, size_t pt)
//   Polygon                     : (size_t ring, size_t pt)
//   MultiPolygon                : (size_t poly, size_t ring, size_t pt)

// 2-D point callback: void(size_t, double, double)
template <typename F>
using is_2d_point_cb = is_invocable_r<void, F, size_t, double, double>;

// 3-D point callback: void(size_t, double, double, double)
template <typename F>
using is_3d_point_cb = is_invocable_r<void, F, size_t, double, double, double>;

// enable_if aliases for use in template parameter lists
template <typename F>
using Require2DPointCb =
    typename std::enable_if<is_2d_point_cb<F>::value, bool>::type;

template <typename F>
using Require3DPointCb = typename std::enable_if<
    is_3d_point_cb<F>::value && !is_2d_point_cb<F>::value, bool>::type;

// MultiLineString / Polygon — two indices
template <typename F>
using is_2d_ring_pt_cb =
    is_invocable_r<void, F, size_t, size_t, double, double>;

template <typename F>
using is_3d_ring_pt_cb =
    is_invocable_r<void, F, size_t, size_t, double, double, double>;

template <typename F>
using Require2DRingPtCb = typename std::enable_if<
    is_2d_ring_pt_cb<F>::value && !is_3d_ring_pt_cb<F>::value, bool>::type;

template <typename F>
using Require3DRingPtCb =
    typename std::enable_if<is_3d_ring_pt_cb<F>::value, bool>::type;

// MultiPolygon — three indices
template <typename F>
using is_2d_poly_pt_cb =
    is_invocable_r<void, F, size_t, size_t, size_t, double, double>;

template <typename F>
using is_3d_poly_pt_cb =
    is_invocable_r<void, F, size_t, size_t, size_t, double, double, double>;

template <typename F>
using Require2DPolyPtCb = typename std::enable_if<
    is_2d_poly_pt_cb<F>::value && !is_3d_poly_pt_cb<F>::value, bool>::type;

template <typename F>
using Require3DPolyPtCb =
    typename std::enable_if<is_3d_poly_pt_cb<F>::value, bool>::type;

// ---- Position reader --------------------------------------------------------

/** Reads a single position array into lon/lat
 *
 *  \param pos   The JSON position array.
 *  \param lon   Output longitude.
 *  \param lat   Output latitude.
 *  \param path  Caller-owned PathBuffer; must already point at this position.
 */
inline void ReadPositionInto(const nlohmann::json& pos, double& lon,
                             double& lat, PathBuffer& path) {
  AssertJsonType(pos, nlohmann::json::value_t::array, path);
  if (pos.size() < 2)
    throw ParseError(path, "position must have at least 2 elements");

  {
    PathGuard g(path, std::size_t{0});
    lon = AssertNumber(pos[0], path);
  }
  {
    PathGuard g(path, size_t{1});
    lat = AssertNumber(pos[1], path);
  }
}

/** Reads a single position array into lon/lat/alt, respecting ZStrategy.
 *
 *  \param pos   The JSON position array.
 *  \param lon   Output longitude.
 *  \param lat   Output latitude.
 *  \param alt   Output altitude; behaviour on 2-D position governed by Z.
 *  \param path  Caller-owned PathBuffer; must already point at this position.
 */
template <ZStrategy Z>
void ReadPositionInto(const nlohmann::json& pos, double& lon, double& lat,
                      double& alt, PathBuffer& path) {
  ReadPositionInto(pos, lon, lat, path);

  if (pos.size() >= 3) {
    PathGuard g(path, size_t{2});
    alt = AssertNumber(pos[2], path);
  } else {
    if constexpr (Z == ZStrategy::DefaultZero) {
      alt = 0.0;
    } else if constexpr (Z == ZStrategy::Throw) {
      throw ParseError(path,
                       "position coordinates were 2D, but 3D was required");
    }
    // ZStrategy::Uninitialized: leave alt as-is
  }
}

// ---- InvokePoint - dispatches to 2-D or 3-D callback ----------------------

/** 3-D callback overload.
 *
 *  \param path  Caller-owned PathBuffer; must already point at this position.
 */
template <ZStrategy Z, typename Callable, Require3DPointCb<Callable> = true>
void InvokePoint(size_t idx, const nlohmann::json& pos, Callable&& cb,
                 PathBuffer& path) {
  double lon, lat, alt;
  ReadPositionInto<Z>(pos, lon, lat, alt, path);
  cb(idx, lon, lat, alt);
}

/** 2-D callback overload. ZStrategy is irrelevant and not a template param.
 *
 *  \param path  Caller-owned PathBuffer; must already point at this position.
 */
template <typename Callable, Require2DPointCb<Callable> = true>
void InvokePoint(size_t idx, const nlohmann::json& pos, Callable&& cb,
                 PathBuffer& path) {
  double lon, lat, alt_ignored;
  ReadPositionInto<ZStrategy::Uninitialized>(pos, lon, lat, alt_ignored, path);
  cb(idx, lon, lat);
}
}  // namespace detail

// ---- ReadPoint --------------------------------------------------------------

/** Reads a GeoJSON Point object (section 3.1.2).
 *
 *  \tparam Z    ZStrategy governing missing altitude. Ignored for 2-D overload.
 *  \param j     A GeoJSON Point object.
 *  \param lon   Output longitude in decimal degrees.
 *  \param lat   Output latitude in decimal degrees.
 *  \param alt   Output altitude in WGS84 ellipsoidal metres.
 *
 *  \throws ParseError  On type mismatch, missing key, or ZStrategy::Throw
 *                      with a 2-D position.
 */
template <ZStrategy Z = ZStrategy::DefaultZero>
void ReadPoint(const nlohmann::json& j, double& lon, double& lat, double& alt) {
  detail::PathBuffer path;
  detail::AssertGeoJsonType(j, Type::Point, path);
  const auto& coords = detail::RequiredKey(j, detail::Keys::coordinates, path);

  detail::PathGuard g(path, detail::Keys::coordinates);
  detail::ReadPositionInto<Z>(coords, lon, lat, alt, path);
}

/** \overload — 2D; altitude not extracted. ZStrategy has no effect. */
inline void ReadPoint(const nlohmann::json& j, double& lon, double& lat) {
  double alt_ignored;
  ReadPoint<ZStrategy::Uninitialized>(j, lon, lat, alt_ignored);
}

// ---- ReadMultiPoint ---------------------------------------------------------

/** Reads a GeoJSON MultiPoint object (section 3.1.3).
 *
 *  \tparam Z         ZStrategy governing missing altitude.
 *  \tparam Callable  \code void(size_t index, double lon, double lat, double
 * alt) \endcode or \code void(size_t index, double lon, double lat) \endcode
 *  \param j          A GeoJSON MultiPoint object.
 *  \param onPoint    Called once per point with its 0-based index and
 * coordinates.
 *
 *  \throws ParseError  On structural or type error in j.
 */
template <ZStrategy Z = ZStrategy::DefaultZero, typename Callable,
          detail::Require3DPointCb<Callable> = true>
void ReadMultiPoint(const nlohmann::json& j, Callable&& onPoint) {
  detail::PathBuffer path;
  detail::AssertGeoJsonType(j, Type::MultiPoint, path);
  const auto& coords = detail::RequiredKey(j, detail::Keys::coordinates, path);

  detail::PathGuard cg(path, detail::Keys::coordinates);
  detail::AssertJsonType(coords, nlohmann::json::value_t::array, path);

  for (size_t i = 0; i < coords.size(); ++i) {
    detail::PathGuard pg(path, i);
    detail::InvokePoint<Z>(i, coords[i], std::forward<Callable>(onPoint), path);
  }
}

/** \overload — 2-D callback; ZStrategy not applicable. */
template <typename Callable, detail::Require2DPointCb<Callable> = true>
void ReadMultiPoint(const nlohmann::json& j, Callable&& onPoint) {
  detail::PathBuffer path;
  detail::AssertGeoJsonType(j, Type::MultiPoint, path);
  const auto& coords = detail::RequiredKey(j, detail::Keys::coordinates, path);

  detail::PathGuard cg(path, detail::Keys::coordinates);
  detail::AssertJsonType(coords, nlohmann::json::value_t::array, path);

  for (size_t i = 0; i < coords.size(); ++i) {
    detail::PathGuard pg(path, i);
    detail::InvokePoint(i, coords[i], std::forward<Callable>(onPoint), path);
  }
}

}  // namespace geojson
