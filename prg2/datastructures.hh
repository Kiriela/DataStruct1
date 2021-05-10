// Datastructures.hh

#ifndef DATASTRUCTURES_HH
#define DATASTRUCTURES_HH

#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <limits>
#include <functional>
#include <unordered_map>
#include <map>
#include <memory>
#include <math.h>
#include <algorithm>
#include <QDebug>

// Types for IDs
using PlaceID = long long int;
using AreaID = long long int;
using Name = std::string;
using WayID = std::string;

// Return values for cases where required thing was not found
PlaceID const NO_PLACE = -1;
AreaID const NO_AREA = -1;
WayID const NO_WAY = "!!No way!!";

// Return value for cases where integer values were not found
int const NO_VALUE = std::numeric_limits<int>::min();

// Return value for cases where name values were not found
Name const NO_NAME = "!!NO_NAME!!";

// Enumeration for different place types
// !!Note since this is a C++11 "scoped enumeration", you'll have to refer to
// individual values as PlaceType::SHELTER etc.
enum class PlaceType { OTHER=0, FIREPIT, SHELTER, PARKING, PEAK, BAY, AREA, NO_TYPE };

// Type for a distance (in metres)
using Distance = int;

// Return value for cases where Duration is unknown
Distance const NO_DISTANCE = NO_VALUE;

// Type for a coordinate (x, y)
struct Coord
{
    int x = NO_VALUE;
    int y = NO_VALUE;
};

// Type to store the data of each Place
struct Place {
    Place(PlaceID id, Name const& name, PlaceType type, Coord coordinates):
        id(id), name(name), type(type), coordinates(coordinates)

    {}
    PlaceID id;
    Name name;
    PlaceType type;
    Coord coordinates;
};

// Type to store the data of each Area
struct Area {
    Area(AreaID id, Name const& name, std::vector<Coord> coordinates):
        id(id), name(name), coordinates(coordinates), parent_area(nullptr), subareas({})
    {}
    AreaID id;
    Name name;
    std::vector<Coord> coordinates;
    // One possible parent area
    std::shared_ptr<Area> parent_area;
    // Several possible subareas
    std::vector<std::weak_ptr<Area>> subareas;
};

struct Way {
    Way(WayID id, std::vector<Coord> coordinates):
        id(id), coordinates(coordinates), end1(*coordinates.begin()), end2(*coordinates.rbegin()), length(0)
    {
        for (std::vector<int>::size_type i = 0; i != coordinates.size() - 1; ++i) {
            int section_length = std::floor(std::sqrt(std::pow(coordinates[i].x - coordinates[i+1].x, 2)
                                            + std::pow(coordinates[i].y - coordinates[i+1].y, 2)));
            length += section_length;
        }
    }
    WayID id;
    std::vector<Coord> coordinates;
    Coord end1;
    Coord end2;
    int length;
};

struct Crossroad_data {
    Crossroad_data(Coord coordinates):
        coordinates(coordinates), visited(false), minimum_distance(NO_DISTANCE)
    {}
    Coord coordinates;
    bool visited;
    int minimum_distance;
};

// Function used to calculate the euclidean distance
// Estimate of performance: O(1)
// Short rationale for estimate: Only makes a set amount of operations to the coordinates
double calculate_euclidean(Coord coord);

inline bool operator==(Coord c1, Coord c2) { return c1.x == c2.x && c1.y == c2.y; }
inline bool operator!=(Coord c1, Coord c2) { return !(c1==c2); }

// Changed inline operator< to do what it should do according to the project specification
inline bool operator<(Coord c1, Coord c2) {
    double c1_e = calculate_euclidean(c1);
    double c2_e = calculate_euclidean(c2);
    // First compare the distance
    if (c1_e < c2_e) { return true; }
    else if (c1_e > c2_e) { return false; }
    // Then compare y. Since if both are equal, the order does not matter, it will always return false
    else { return c1.y < c2.y; }
}

struct CoordHash
{
    std::size_t operator()(Coord xy) const
    {
        auto hasher = std::hash<int>();
        auto xhash = hasher(xy.x);
        auto yhash = hasher(xy.y);
        // Combine hash values (magic!)
        return xhash ^ (yhash + 0x9e3779b9 + (xhash << 6) + (xhash >> 2));
    }
};

// Return value for cases where coordinates were not found
Coord const NO_COORD = {NO_VALUE, NO_VALUE};

class Datastructures
{
public:
    Datastructures();
    ~Datastructures();

    // Estimate of performance: O(1)
    // Short rationale for estimate: The only operation used by the method is a constant, always used once
    int place_count();

    // Estimate of performance: O(n) [technically 3*O(n) + O(m), where n is the amount of Places and m is the amount of Areas.
    // Short rationale for estimate: All of the clear() functions are linear
    void clear_all();

    // Estimate of performance: O(n), linear with the number of elements in places_by_id_
    // Short rationale for estimate: Simply pushes all of the IDs from the unordered map onto a vector
    std::vector<PlaceID> all_places();

    // Estimate of performance: From get_place(id) -> O(n), average case is constant
    // Short rationale for estimate: From get_place(id) -> Up to linear between the searched container: std::find(), rest of the operations are constant
    bool add_place(PlaceID id, Name const& name, PlaceType type, Coord xy);

    // Estimate of performance: From get_place(id) -> O(n), average case is constant
    // Short rationale for estimate: From get_place(id) -> Up to linear between the searched container: std::find(), other operations constant
    std::pair<Name, PlaceType> get_place_name_type(PlaceID id);

    // Estimate of performance: From get_place(id) -> O(n), average case is constant
    // Short rationale for estimate: From get_place(id) -> Up to linear between the searched container: std::find(), other operations constant
    Coord get_place_coord(PlaceID id);

    // Estimate of performance: O(n*log n), if alphabetical_vector_ids_ is known to be already sorted the runtime is Ω(1)
    // Short rationale for estimate: Adding a key-value pair to the temporary multimap is log n, so the runtime is therefore
    // going through the entire container n*log n
    std::vector<PlaceID> places_alphabetically();

    // Estimate of performance: O(n*log n), if coordinate_vector_ids_ is known to be already sorted the runtime is Ω(1)
    // Short rationale for estimate: Adding a key-value pair to the temporary multimap is log n, so the runtime is therefore
    // going through the entire container sized n is n*log n
    std::vector<PlaceID> places_coord_order();

    // Estimate of performance: θ(n), where n is the amount of keys of the specified type, O(n) container size
    // Short rationale for estimate: Since std::equal_range is linear to the number of elements with the specified key, and the for-loop
    // only pushes these values back onto a vector, the runtime is linear to the number of elements. Worst case is if all of the keys
    // are valid for the operation
    std::vector<PlaceID> find_places_name(Name const& name);

    // Estimate of performance: θ(n), where n is the amount of keys of the specified type, O(n) container size
    // Short rationale for estimate: Since std::equal_range is linear to the number of elements with the specified key, and the for-loop
    // only pushes these values back onto a vector, the runtime is linear to the number of elements. Worst case is if all of the keys
    // are valid for the operation
    std::vector<PlaceID> find_places_type(PlaceType type);

    // Estimate of performance:
    // Short rationale for estimate:
    bool change_place_name(PlaceID id, Name const& newname);

    // Estimate of performance: From get_place(id) -> O(n), average case is constant
    // Short rationale for estimate: From get_place(id) -> Up to linear between the searched container: std::find(), rest of the operations are constant
    bool change_place_coord(PlaceID id, Coord newcoord);

    // Estimate of performance: From get_place(id) -> O(n), average case is constant
    // Short rationale for estimate: From get_place(id) -> Up to linear between the searched container: std::find(), rest of the operations are constant
    bool add_area(AreaID id, Name const& name, std::vector<Coord> coords);

    // Estimate of performance: From get_area(id) -> O(n), average case is constant
    // Short rationale for estimate: From get_area(id) -> Up to linear between the searched container: std::find(), rest of the operations are constant
    Name get_area_name(AreaID id);

    // Estimate of performance: From get_place(id) -> O(n), average case is constant
    // Short rationale for estimate: From get_place(id) -> Up to linear between the searched container: std::find(), rest of the operations are constant
    std::vector<Coord> get_area_coords(AreaID id);

    // Estimate of performance: O(n), linear with the number of elements in areas_by_id_
    // Short rationale for estimate: Simply pushes all of the IDs from the unordered map onto a vector
    std::vector<AreaID> all_areas();

    // Estimate of performance: From get_area(id) -> O(n), average case is constant
    // Short rationale for estimate: From get_area(id) -> Up to linear between the searched container: std::find(), rest of the operations constant
    bool add_subarea_to_area(AreaID id, AreaID parentid);

    // Estimate of performance: From get_area(id) -> O(n) with the container size, average case is linear with the amount of parents
    // Short rationale for estimate: From get_area(id) -> Up to linear between the searched container: std::find(),
    // on average linear with the amount of parents, indirect or direct the area has, since the search operation is on average constant
    std::vector<AreaID> subarea_in_areas(AreaID id);

    // Estimate of performance: From get_area(id) -> O(n) with the container size, average case is constant
    // Short rationale for estimate: From get_area(id) -> Up to linear between the searched container: std::find(), rest of the operations constant
    void creation_finished();

    // Estimate of performance: O(n) with the container size (if type = PlaceType::NO_TYPE), average case is linear with the defined range of wanted elements
    // as per with std::equal_range() and the for-loop itself
    // Short rationale for estimate: The method only loops over the values necessary for the specific case, so if the type is not defined, will have to
    // loop over the entire structure, otherwise the loop and std::equal_range() is on an average case linear with the number of elements with the wanted key
    std::vector<AreaID> all_subareas_in_area(AreaID id);

    // Estimate of performance:
    // Short rationale for estimate:
    std::vector<PlaceID> places_closest_to(Coord xy, PlaceType type);

    // Estimate of performance: From get_area(id) -> O(n) with the container size, average case is linear in the amount of specified names and placetypes
    // Short rationale for estimate: From get_area(id) -> Up to linear between the searched container: std::find(); std::find is on average constant,
    // therefore the for-loops affect the average case the most. They are linear with the amount of elements with the same keys within the places_by_type_
    // and places_by_name_ data structures, as the loops only go through any elements which have the same key, therefore both of the loops also have
    // a complexity of O(n) with the container size, but in an average case they are linear with the appropriate elements with the same key as the wanted element.
    bool remove_place(PlaceID id);

    // Estimate of performance: From get_area(id) -> O(n) with the container size, average case is linear in the amount of parents
    // Short rationale for estimate: Up to linear between the searched container: std::find(); as std::mismatch is at worst case the range
    // of the iterators given to it, which have already been processed by the while functions which go through all of the parents, the average case is therefore
    // the while-loop length while the std::find() still has the worst case as container size.
    AreaID common_area_of_subareas(AreaID id1, AreaID id2);

    // Phase 2 operations

    // Estimate of performance: O(n) in the ways_by_id_ container size
    // Short rationale for estimate: Simply looping over the container
    std::vector<WayID> all_ways();

    // Estimate of performance: O(n) in the size of the ways_by_id_ unordered map
    // Short rationale for estimate: Simply looping over the container
    bool add_way(WayID id, std::vector<Coord> coords);

    // Estimate of performance: O(n), where n is the amount of ways with the coord as either of their ends.
    // Worst case is equal to container size.
    // Short rationale for estimate: equal_range is constant in an average case, worth case linear to container size,
    // and the for-loop is linear to the amount of applicable ways n
    std::vector<std::pair<WayID, Coord>> ways_from(Coord xy);

    // Estimate of performance: O(n), where n is the container size. Average case constant.
    // Short rationale for estimate: std::find on an unordered map is average case constant,
    // worst case linear in container size
    std::vector<Coord> get_way_coords(WayID id);

    // Estimate of performance: O(n) [Technically all of the data structures can have different sizes,
    // so it would be O(n + m...)]
    // Short rationale for estimate: All of the clear() functions are linear
    void clear_ways();

    // Estimate of performance: Clean_for_search() -> O(n), search_any() -> O(m), therefore O(n + m) -> O(m)
    // Short rationale for estimate: Simply looping over the containers as necessary when cleaning
    // them ready for the search, we traverse without repetition in search_any(), which is a
    // DFS searching algorithm, therefore O(m)
    std::vector<std::tuple<Coord, WayID, Distance>> route_any(Coord fromxy, Coord toxy);

    // Non-compulsory operations

    // Estimate of performance: O(n), to ways_by_coord_ or ways_by_id_ data structure, technically O(n + m)
    // Short rationale for estimate: Both the find and equal_range functions are linear to container size
    // in worst case
    bool remove_way(WayID id);

    // Estimate of performance: -
    // Short rationale for estimate: -
    std::vector<std::tuple<Coord, WayID, Distance>> route_least_crossroads(Coord fromxy, Coord toxy);

    // Estimate of performance: Clean_for_search() -> O(n), search_any() -> O(m), therefore O(n + m) -> O(m)
    // Short rationale for estimate: Simply looping over the containers as necessary when cleaning
    // them ready for the search, we traverse without repetition in search_any(), which is a
    // DFS searching algorithm, therefore O(m), seemingly this route algorithm is considerably more
    // expensive than the route_any, so an educated guess would be to say O(m) is considerably
    // more expensive than O(n) in this case
    std::vector<std::tuple<Coord, WayID>> route_with_cycle(Coord fromxy);

    // Estimate of performance: -
    // Short rationale for estimate: -
    std::vector<std::tuple<Coord, WayID, Distance>> route_shortest_distance(Coord fromxy, Coord toxy);

    // Estimate of performance: -
    // Short rationale for estimate: -
    Distance trim_ways();

private:
    // Used as flags to determine if the alphabetical_vector_ids_ and coordinate_vector_ids_
    // are already in order to prevent unnecessary sorting
    bool coordinate_sorted_;
    bool alphabetical_sorted_;

    // Containers used by places_alphabetically() and places_coord_order()
    std::vector<PlaceID> alphabetical_vector_ids_;
    std::vector<PlaceID> coordinate_vector_ids_;

    // Pointers to all Places are stored in these three structures, keys as IDs, names and types
    std::unordered_map<PlaceID, std::shared_ptr<Place>> places_by_id_;
    // Different from IDs, names and types can overlap, using multimap instead of regular map
    std::unordered_multimap<Name, std::shared_ptr<Place>> places_by_name_;
    std::unordered_multimap<PlaceType, std::shared_ptr<Place>> places_by_type_;

    // Areas are only stored by ID
    std::unordered_map<AreaID, std::shared_ptr<Area>> areas_by_id_;

    // Estimate of performance: O(n), average case is constant
    // Short rationale for estimate: Up to linear between the searched container: std::find()
    // Used to see if a place exists within the data structure
    std::shared_ptr<Place> get_place(PlaceID id);

    // Estimate of performance: O(n), average case is constant
    // Short rationale for estimate: Up to linear between the searched container: std::find()
    // Used to see if an area exists within the data structure
    std::shared_ptr<Area> get_area(AreaID id);

    // Estimate of performance: θ(n) where n is the amount of children, worst case is n where n is the container size
    // Short rationale for estimate: Since every child only has constant operations done to them, the runtime is linear to the child amount
    // Used recursively by the all_subareas_in_area method
    std::vector<AreaID> get_children(std::weak_ptr<Area> current_area);

    // PHASE 2

    // Pointers to all Ways are stored by WayID and Coord within these two data structures
    std::unordered_map<WayID, std::shared_ptr<Way>> ways_by_id_;
    std::unordered_multimap<Coord, std::shared_ptr<Way>, CoordHash> ways_by_coord_;

    // Stores data about any given crossroad, with the Coord as a key
    std::unordered_map<Coord, std::shared_ptr<Crossroad_data>, CoordHash> visited_coordinates_;

    // Used to build the returned vectors for the route_any and route_with_cycle functions
    std::vector<std::tuple<Coord, WayID, Distance>> chosen_route_;
    std::vector<std::tuple<Coord, WayID>> cyclic_route_;

    bool route_found_;

    // Estimate of performance: O(n) to the amount of ways in ways_by_id_
    // Short rationale for estimate: we traverse without repetition, which is a
    // DFS searching algorithm, therefore O(n)
    void search_any(Coord current, Coord goal, int route_length);

    // Estimate of performance: O(n) to the amount of ways in ways_by_id_
    // Short rationale for estimate: we traverse without repetition (until the single looping node), which is a
    // DFS searching algorithm, therefore O(n)
    void search_cycle(Coord current, Coord previous);

    // Estimate of performance: O(n) in the visited_coordinates container size
    // Short rationale for estimate: Simply loops over the container
    void clean_for_search();

    // Estimate of performance: O(n), average case is constant
    // Short rationale for estimate: Up to linear between the searched container: std::find()
    // Used to see if a way exists within the data structure
    std::shared_ptr<Way> get_way(WayID id);
};

#endif // DATASTRUCTURES_HH
