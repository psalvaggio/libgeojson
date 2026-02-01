/** Custom testing predicates for libgeojson
 *
 *  \file Predicates.cpp
 *  \author Dr. Philip Salvaggio (salvaggio.philip@gmail.com)
 *  \date 17 Jan 2020
 */

#include <gtest/gtest.h>

#include "libgeojson/libgeojson.h"

/** Tests whether j is a JSON object */
inline ::testing::AssertionResult IsJsonObject(const nlohmann::json& j) {
  if (!j.is_object()) {
    return ::testing::AssertionFailure() << "Expected a JSON object, but got:\n"
                                         << j.dump();
  }
  return ::testing::AssertionSuccess();
}

/** Test whether the "type" field is the given value */
inline ::testing::AssertionResult IsType(const nlohmann::json& j,
                                         const char* type) {
  auto res = IsJsonObject(j);
  if (!res) return res;

  auto it = j.find("type");
  if (it == j.end()) {
    return ::testing::AssertionFailure() << "\"type\" field not found:\n"
                                         << j.dump();
  }
  if (it.value().get<std::string>() != type) {
    return ::testing::AssertionFailure()
           << "Expected type to be \"" << type << "\" but it was \""
           << it.value().get<std::string>() << "\"";
  }
  return ::testing::AssertionSuccess();
}
template <geojson::Type T>
::testing::AssertionResult IsType(const nlohmann::json& j) {
  return IsType(j, geojson::TypeName<T>());
}

/** Tests whether j is an object of the form:
 *  {"type": "Name", "coordinates": array}
 */
template <geojson::Type T>
::testing::AssertionResult IsCoordinatesObject(const nlohmann::json& j) {
  auto res = IsType<T>(j);
  if (!res) return res;
  if (!j["coordinates"].is_array()) {
    return ::testing::AssertionFailure()
           << "Expected \"coordinates\" to be an array, but it was:\n"
           << j["coordinates"].dump();
  }

  return ::testing::AssertionSuccess();
}

/** Tests whether j is a JSON array */
inline ::testing::AssertionResult IsJsonArray(const nlohmann::json& j) {
  if (!j.is_array()) {
    return ::testing::AssertionFailure() << "Expect an array, but got:\n"
                                         << j.dump();
  }
  return ::testing::AssertionSuccess();
}

/** Tests whether j is an array of size size */
inline ::testing::AssertionResult IsJsonArrayOfSize(const nlohmann::json& j,
                                                    size_t size) {
  if (j.is_array()) {
    if (j.size() == size) {
      return ::testing::AssertionSuccess();
    } else {
      return ::testing::AssertionFailure()
             << "Expect an array of size " << size << ", but got one of size "
             << j.size();
    }
  } else {
    return ::testing::AssertionFailure()
           << "Expected an array, but got: " << j.dump();
  }
}

/** Tests the given 3D position */
inline ::testing::AssertionResult TestPosition(const nlohmann::json& test,
                                               double lon, double lat,
                                               double alt) {
  auto res = IsJsonArrayOfSize(test, 3u);
  if (!res) return res;

  if (test[0].get<double>() != lon || test[1].get<double>() != lat ||
      test[2].get<double>() != alt) {
    return ::testing::AssertionFailure()
           << "Expected [" << lon << "," << lat << "," << alt << "], but got "
           << test.dump();
  }
  return ::testing::AssertionSuccess();
}

/** Tests the given 2D position */
inline ::testing::AssertionResult TestPosition(const nlohmann::json& test,
                                               double lon, double lat) {
  auto res = IsJsonArrayOfSize(test, 2u);
  if (!res) return res;

  if (test[0].get<double>() != lon || test[1].get<double>() != lat) {
    return ::testing::AssertionFailure()
           << "Expected [" << lon << "," << lat << "], but got " << test.dump();
  }
  return ::testing::AssertionSuccess();
}

/** Tests the given 3D point object */
inline ::testing::AssertionResult TestPoint(const nlohmann::json& j, double lon,
                                            double lat, double alt) {
  auto res = IsCoordinatesObject<geojson::Type::Point>(j);
  if (!res) return res;
  return TestPosition(j["coordinates"], lon, lat, alt);
}

/** Tests the given 2D point object */
inline ::testing::AssertionResult TestPoint(const nlohmann::json& j, double lon,
                                            double lat) {
  auto res = IsCoordinatesObject<geojson::Type::Point>(j);
  if (!res) return res;
  return TestPosition(j["coordinates"], lon, lat);
}

/** Tests the given array of 3D positions */
template <typename Callable,
          geojson::detail::IsCallbackSignature<Callable, void, size_t, double&,
                                               double&, double&> = true>
::testing::AssertionResult TestPositionArray(const nlohmann::json& j,
                                             size_t numPoints,
                                             Callable&& getPoint) {
  auto res = IsJsonArrayOfSize(j, numPoints);
  if (!res) return res;

  for (size_t i = 0; i < numPoints; i++) {
    double lon, lat, alt;
    getPoint(i, lon, lat, alt);
    res = TestPosition(j[i], lon, lat, alt);
    if (!res) return res;
  }
  return ::testing::AssertionSuccess();
}

/** Tests the given array of 2D positions */
template <typename Callable,
          geojson::detail::IsCallbackSignature<Callable, void, size_t, double&,
                                               double&> = true>
::testing::AssertionResult TestPositionArray(const nlohmann::json& j,
                                             size_t numPoints,
                                             Callable&& getPoint) {
  auto res = IsJsonArrayOfSize(j, numPoints);
  if (!res) return res;

  for (size_t i = 0; i < numPoints; i++) {
    double lon, lat;
    getPoint(i, lon, lat);
    res = TestPosition(j[i], lon, lat);
    if (!res) return res;
  }
  return ::testing::AssertionSuccess();
}

/** Tests the geojson::MultiPoint function with the given parameters */
template <typename Callable>
::testing::AssertionResult TestMultiPoint(size_t numPoints,
                                          Callable&& getPoint) {
  auto j = geojson::MultiPoint(numPoints, getPoint);
  auto res = IsCoordinatesObject<geojson::Type::MultiPoint>(j);
  if (!res) return res;
  return TestPositionArray(j["coordinates"], numPoints, getPoint);
}

/** Tests the geojson::LineString function with the given parameters */
template <typename Callable>
::testing::AssertionResult TestLineString(size_t numPoints,
                                          Callable&& getPoint) {
  auto j = geojson::LineString(numPoints, getPoint);
  auto res = IsCoordinatesObject<geojson::Type::LineString>(j);
  if (!res) return res;
  return TestPositionArray(j["coordinates"], numPoints, getPoint);
}

/** Tests the geojson::MultiLineString function with the given parameters */
template <typename GetLineLength, typename GetPoint,
          geojson::detail::IsCallbackSignature<
              GetPoint, void, size_t, size_t, double&, double&, double&> = true>
::testing::AssertionResult TestMultiLineString(size_t numLineStrings,
                                               GetLineLength&& getLineLength,
                                               GetPoint&& getPoint) {
  auto j = geojson::MultiLineString(numLineStrings, getLineLength, getPoint);

  auto res = IsCoordinatesObject<geojson::Type::MultiLineString>(j);
  if (!res) return res;

  const auto& coords = j["coordinates"];
  res = IsJsonArrayOfSize(coords, numLineStrings);
  if (!res) return res;

  for (size_t i = 0; i < numLineStrings; i++) {
    res =
        TestPositionArray(coords[i], getLineLength(i),
                          [&](size_t pt, double& lon, double& lat,
                              double& alt) { getPoint(i, pt, lon, lat, alt); });
    if (!res) return res;
  }
  return ::testing::AssertionSuccess();
}

/** Tests the geojson::MultiLineString function with the given parameters */
template <typename GetLineLength, typename GetPoint,
          geojson::detail::IsCallbackSignature<GetPoint, void, size_t, size_t,
                                               double&, double&> = true>
::testing::AssertionResult TestMultiLineString(size_t numLineStrings,
                                               GetLineLength&& getLineLength,
                                               GetPoint&& getPoint) {
  auto j = geojson::MultiLineString(numLineStrings, getLineLength, getPoint);

  auto res = IsCoordinatesObject<geojson::Type::MultiLineString>(j);
  if (!res) return res;

  const auto& coords = j["coordinates"];
  res = IsJsonArrayOfSize(coords, numLineStrings);
  if (!res) return res;

  for (size_t i = 0; i < numLineStrings; i++) {
    res = TestPositionArray(coords[i], getLineLength(i),
                            [&](size_t pt, double& lon, double& lat) {
                              getPoint(i, pt, lon, lat);
                            });
    if (!res) return res;
  }
  return ::testing::AssertionSuccess();
}

/** Tests the given linear ring array */
template <typename Callable>
::testing::AssertionResult TestLinearRing3D(const nlohmann::json& test,
                                            size_t numPoints, bool reverse,
                                            Callable&& getPoint) {
  auto res = IsJsonArrayOfSize(test, numPoints + 1);
  if (!res) return res;

  for (size_t i = 0; i < numPoints; i++) {
    size_t testIdx = reverse ? numPoints - i - 1 : i;

    double lon, lat, alt;
    getPoint(i, lon, lat, alt);
    auto res = TestPosition(test[testIdx], lon, lat, alt);
    if (!res) return res;
  }
  if (test[0] != test[test.size() - 1]) {
    return ::testing::AssertionFailure()
           << "First and last element of linear ring do not match: "
           << test.dump();
  }
  return ::testing::AssertionSuccess();
}

/** Tests the given polygon coordinates array */
template <typename GetRingLength, typename GetPoint>
::testing::AssertionResult TestPolygonCoordinates3D(
    const nlohmann::json& j, size_t numRings, GetRingLength&& getRingLength,
    GetPoint&& getPoint) {
  auto res = IsJsonArrayOfSize(j, numRings);
  if (!res) return res;

  size_t ring = 0;
  auto getRingPt = [&](size_t pt, double& lon, double& lat, double& alt) {
    getPoint(ring, pt, lon, lat, alt);
  };

  auto rawString =
      geojson::detail::LineStringCoordinates(getRingLength(0), getRingPt);
  res = TestLinearRing3D(j[0], getRingLength(0),
                         !geojson::detail::IsCcw(rawString), getRingPt);
  if (!res) return res;

  for (ring = 1; ring < numRings; ring++) {
    rawString =
        geojson::detail::LineStringCoordinates(getRingLength(ring), getRingPt);
    res = TestLinearRing3D(j[ring], getRingLength(ring),
                           geojson::detail::IsCcw(rawString), getRingPt);
    if (!res) return res;
  }
  return ::testing::AssertionSuccess();
}

/** Tests geojson::Polygon with the given parameters */
template <typename GetRingLength, typename GetPoint>
::testing::AssertionResult TestPolygon3D(size_t numRings,
                                         GetRingLength&& getRingLength,
                                         GetPoint&& getPoint) {
  auto j = geojson::Polygon(numRings, getRingLength, getPoint);
  auto res = IsCoordinatesObject<geojson::Type::Polygon>(j);
  if (!res) return res;
  return TestPolygonCoordinates3D(j["coordinates"], numRings, getRingLength,
                                  getPoint);
}

/** Tests geojson::MultiPolygon with the given parameters */
template <typename GetNumRings, typename GetRingLength, typename GetPoint>
::testing::AssertionResult TestMultiPolygon3D(size_t numPolygons,
                                              GetNumRings&& getNumRings,
                                              GetRingLength&& getRingLength,
                                              GetPoint&& getPoint) {
  auto j =
      geojson::MultiPolygon(numPolygons, getNumRings, getRingLength, getPoint);
  auto res = IsCoordinatesObject<geojson::Type::MultiPolygon>(j);
  if (!res) return res;

  const auto& coords = j["coordinates"];
  res = IsJsonArrayOfSize(coords, numPolygons);
  if (!res) return res;

  size_t poly = 0;
  auto getRingLengthI = [&](size_t ring) -> size_t {
    return getRingLength(poly, ring);
  };
  auto getPointI = [&](size_t ring, size_t pt, double& lat, double& lon,
                       double& alt) {
    getPoint(poly, ring, pt, lat, lon, alt);
  };
  for (; poly < numPolygons; poly++) {
    res = TestPolygonCoordinates3D(coords[poly], getNumRings(poly),
                                   getRingLengthI, getPointI);
    if (!res) return res;
  }
  return ::testing::AssertionSuccess();
}

/** Tests the given Feature object */
inline ::testing::AssertionResult TestFeature(const nlohmann::json& j,
                                              const nlohmann::json& geometry,
                                              const nlohmann::json& props) {
  auto res = IsType<geojson::Type::Feature>(j);
  if (!res) return res;
  if (j["geometry"] != geometry) {
    return ::testing::AssertionFailure() << "Expected the geometry to be:\n"
                                         << geometry.dump() << "\nbut it was:\n"
                                         << j["geometry"].dump() << std::endl;
  }
  if (j["properties"] != props) {
    return ::testing::AssertionFailure() << "Expected the properties to be:\n"
                                         << props.dump() << "\nbut it was:\n"
                                         << j["properties"].dump() << std::endl;
  }
  return ::testing::AssertionSuccess();
}

/** Tests geojson::FeatureCollection with the given inputs */
template <typename Callable>
::testing::AssertionResult TestFeatureCollection(size_t numFeatures,
                                                 Callable&& getFeature) {
  auto j = geojson::FeatureCollection(numFeatures, getFeature);

  auto res = IsType<geojson::Type::FeatureCollection>(j);
  if (!res) return res;

  const auto& features = j["features"];
  res = IsJsonArrayOfSize(features, numFeatures);
  if (!res) return res;

  for (size_t i = 0; i < numFeatures; i++) {
    if (features[i] != getFeature(i)) {
      return ::testing::AssertionFailure()
             << "Expected features[" << i << "] to be:\n"
             << getFeature(i) << "\nbut it was:\n"
             << features[i];
    }
  }
  return ::testing::AssertionSuccess();
}
