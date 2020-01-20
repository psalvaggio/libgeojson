/** Unit tests for libgeojson
 *
 *  \file unit_tests.cpp
 *  \author Dr. Philip Salvaggio (salvaggio.philip@gmail.com)
 *  \date 17 Jan 2020
 */

#include <vector>

#include <gtest/gtest.h>

#include "Predicates.h"
#include "libgeojson/libgeojson.h"

// A simple struct to hold a 3D point
struct Pt3D {
  Pt3D(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
  double x, y, z;
};

// Test struct for holding feature properties
struct Props {
  Props(const std::string _name, double _foo) : name(_name), foo(_foo) {}

  std::string name;
  double foo;
};

// JSON conversion for Props
void to_json(nlohmann::json& j, const Props& props) {
  j = nlohmann::json{{"name", props.name}, {"foo", props.foo}};
}

TEST(LibgeojsonTest, NameTests) {
  using namespace geojson;

#define TEST_GEO_TYPE(constant, name)  \
  EXPECT_EQ(TypeName(constant), name); \
  EXPECT_EQ(TypeName<constant>(), name);

  TEST_GEO_TYPE(Type::Point, "Point")
  TEST_GEO_TYPE(Type::MultiPoint, "MultiPoint")
  TEST_GEO_TYPE(Type::LineString, "LineString")
  TEST_GEO_TYPE(Type::MultiLineString, "MultiLineString")
  TEST_GEO_TYPE(Type::Polygon, "Polygon")
  TEST_GEO_TYPE(Type::MultiPolygon, "MultiPolygon")

#undef TEST_GEO_TYPE
}

TEST(LibgeojsonTest, PositionTests) {
  EXPECT_TRUE(TestPosition(geojson::Position(5.3, 10.4), 5.3, 10.4));
  EXPECT_TRUE(TestPosition(geojson::Position(2.1, 3.4, 4.5), 2.1, 3.4, 4.5));
}

TEST(LibgeojsonTest, PointTest) {
  EXPECT_TRUE(TestPoint(geojson::Point(5.3, 10.4), 5.3, 10.4));
  EXPECT_TRUE(TestPoint(geojson::Point(2.1, 3.4, 4.5), 2.1, 3.4, 4.5));
}

TEST(LibgeojsonTest, MultiPointTest) {
  std::vector<double> pts2d{0, 0.5, 1, 1.5, 2, 2.5};
  EXPECT_TRUE(
      TestMultiPoint(pts2d.size() / 2, [&](size_t i, double& lon, double& lat) {
        lon = pts2d[2 * i];
        lat = pts2d[2 * i + 1];
      }));

  std::vector<std::array<double, 3>> pts3d{{0, 1.1, 2.2}, {3.3, 4.4, 5.5}};
  EXPECT_TRUE(TestMultiPoint(
      pts3d.size(), [&](size_t i, double& lon, double& lat, double& alt) {
        lon = pts3d[i][0];
        lat = pts3d[i][1];
        alt = pts3d[i][2];
      }));
}

TEST(LibgeojsonTest, LineStringTest) {
  std::vector<double> pts2d{0, 0.5, 1, 1.5, 2, 2.5};
  EXPECT_TRUE(
      TestLineString(pts2d.size() / 2, [&](size_t i, double& lon, double& lat) {
        lon = pts2d[2 * i];
        lat = pts2d[2 * i + 1];
      }));

  std::vector<std::array<double, 3>> pts3d{{0, 1.1, 2.2}, {3.3, 4.4, 5.5}};
  EXPECT_TRUE(TestLineString(
      pts3d.size(), [&](size_t i, double& lon, double& lat, double& alt) {
        lon = pts3d[i][0];
        lat = pts3d[i][1];
        alt = pts3d[i][2];
      }));
}

TEST(LibgeojsonTest, MultiLineStringCoordinatesTest) {
  std::vector<std::vector<std::pair<double, double>>> pts2d{
      {{0, 0.5}, {1, 1.5}, {2, 2.5}}, {{2, 3}, {4, 5}}};
  EXPECT_TRUE(TestMultiLineString(
      pts2d.size(), [&](size_t l) -> size_t { return pts2d[l].size(); },
      [&](size_t l, size_t p, double& lon, double& lat) {
        lon = pts2d[l][p].first;
        lat = pts2d[l][p].second;
      }));

  std::vector<std::vector<std::array<double, 3>>> pts3d{
      {{0, 1, 2}, {3, 4.1, 5}}, {{3, 4, 5}, {6, 7, 8}, {9, 10, 11}}};
  EXPECT_TRUE(TestMultiLineString(
      pts3d.size(), [&](size_t l) -> size_t { return pts3d[l].size(); },
      [&](size_t l, size_t p, double& lon, double& lat, double& alt) {
        lon = pts3d[l][p][0];
        lat = pts3d[l][p][1];
        alt = pts3d[l][p][2];
      }));
}

TEST(LibgeojsonTest, IsCcwTest) {
  std::vector<double> pts;
  auto getPoint = [&](size_t i, double& x, double& y) {
    x = pts[2 * i];
    y = pts[2 * i + 1];
  };

  // CCW unit square
  pts = std::vector<double>{0, 0, 1, 0, 1, 1, 0, 1};
  EXPECT_TRUE(geojson::detail::IsCcw(
      geojson::detail::LineStringCoordinates(pts.size() / 2, getPoint)));

  // CW unit square
  pts = std::vector<double>{0, 0, 0, 1, 1, 1, 1, 0};
  EXPECT_FALSE(geojson::detail::IsCcw(
      geojson::detail::LineStringCoordinates(pts.size() / 2, getPoint)));

  // CCW C shape
  pts = std::vector<double>{0, 0,   2, 0,   2, 0.5, 1, 0.5,
                            1, 1.5, 2, 1.5, 2, 2,   0, 2};
  EXPECT_TRUE(geojson::detail::IsCcw(
      geojson::detail::LineStringCoordinates(pts.size() / 2, getPoint)));

  // CW C shape
  pts = std::vector<double>{0, 0,   0, 2,   2, 2,   2, 1.5,
                            1, 1.5, 1, 0.5, 2, 0.5, 2, 0};
  EXPECT_FALSE(geojson::detail::IsCcw(
      geojson::detail::LineStringCoordinates(pts.size() / 2, getPoint)));
}

TEST(LibgeojsonTest, LinearRingCoordinatesTest) {
  std::vector<std::array<double, 3>> test;
  auto getPoint = [&](size_t i, double& lon, double& lat, double& alt) {
    lon = test[i][0];
    lat = test[i][1];
    alt = test[i][2];
  };

  test = std::vector<std::array<double, 3>>{
      {0, 0, 0.5}, {1.5, 0, 0.3}, {1.5, 1.5, 0.6}, {0, 1.5, 0.9}};

  auto j = geojson::detail::LinearRingCoordinates(test.size(), true, getPoint);
  EXPECT_TRUE(TestLinearRing3D(j, test.size(), false, getPoint));

  j = geojson::detail::LinearRingCoordinates(test.size(), false, getPoint);
  EXPECT_TRUE(TestLinearRing3D(j, test.size(), true, getPoint));
}

TEST(LibgeojsonTest, PolygonTest) {
  std::vector<std::array<double, 3>> outer{
      {0, 0, 0.5}, {1.5, 0, 0.3}, {1.5, 1.5, 0.6}, {0, 1.5, 0.9}};
  std::vector<std::vector<std::array<double, 3>>> inners{
      {{0.25, 0.25, 0.5}, {0.35, 0.75, 0.6}, {0.5, 0.25, 0.7}},
      {{1, 0.25, 0.5}, {1.25, 0.25, 0.6}, {1.125, 0.5, 0.7}}};

  auto getRingLength = [&](size_t ring) {
    return ring == 0 ? outer.size() : inners[ring - 1].size();
  };
  auto getPoint = [&](size_t ring, size_t pt, double& lon, double& lat,
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
  };

  EXPECT_TRUE(TestPolygon3D(inners.size() + 1, getRingLength, getPoint));
}

TEST(LibgeojsonTest, MultiPolygonCoordinates) {
  std::vector<std::vector<std::array<double, 3>>> outers{
      {{0, 0, 0.5}, {1.5, 0, 0.3}, {1.5, 1.5, 0.6}, {0, 1.5, 0.9}},
      {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}};
  std::vector<std::vector<std::vector<std::array<double, 3>>>> inners{
      {{{0.25, 0.25, 0.5}, {0.35, 0.75, 0.6}, {0.5, 0.25, 0.7}},
       {{1, 0.25, 0.5}, {1.25, 0.25, 0.6}, {1.125, 0.5, 0.7}}},
      {}};

  EXPECT_TRUE(TestMultiPolygon3D(
      outers.size(),
      [&](size_t poly) -> size_t { return inners[poly].size() + 1; },
      [&](size_t poly, size_t ring) -> size_t {
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
      }));
}

TEST(LibgeojsonTest, FeatureTest) {
  Pt3D pt(1.2, 3.4, 5.6);
  nlohmann::json props(Props("bar", 4.3));
  auto geomJ = geojson::Point(pt.x, pt.y, pt.z);
  auto j = geojson::Feature("foo", geomJ, props);

  EXPECT_TRUE(TestFeature(j, geomJ, props));
  EXPECT_EQ(j["id"].get<std::string>(), "foo");
}

TEST(LibgeojsonTest, FeatureCollectionTest) {
  std::vector<Pt3D> pts{Pt3D(1, 2, 3), Pt3D(2, 3, 4), Pt3D(3, 4, 5)};
  std::vector<nlohmann::json> props{Props("bar", 4.3), Props("bar2", 5.1),
                                    Props("bar3", 4.8)};
  TestFeatureCollection(pts.size(), [&](size_t i) -> nlohmann::json {
    return geojson::Feature(geojson::Point(pts[i].x, pts[i].y, pts[i].z),
                            props[i]);
  });
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
