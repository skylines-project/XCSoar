/* Generated by Together */

#ifndef SAMPLEDTASKPOINT_H
#define SAMPLEDTASKPOINT_H

#include "Navigation/SearchPointVector.hpp"
#include "ObservationZone.hpp"
#include "TaskPoint.hpp"
#include "Navigation/TaskProjection.hpp"

class TaskEvents;

/**
 * Abstract specialisation of TaskPoint which has an observation zone
 * and can manage records of the appearance of the aircraft within the
 * observation zone, as well as provide methods to scan for potential
 * paths to border locations.
 *
 * \todo
 * - Currently undefined as to what happens to interior samples if observation 
 *   zone is modified (e.g. due to previous/next taskpoint moving)
 */
class SampledTaskPoint:
  public TaskPoint, 
  public ObservationZone
{
public:  
/** 
 * Constructor.  Clears boundary and interior samples on instantiation.
 * Must be followed by default_boundary_points() and update_projection() after
 * complete task is constructed.
 * 
 * @param tp Projection used for internal representations of borders and samples
 * @param wp Waypoint associated with this task point
 * @param tb Task Behaviour defining options (esp safety heights)
 * @param b_scored Whether distance within OZ is scored 
 * 
 * @return Partially initialised object 
 */
  SampledTaskPoint(const TaskProjection& tp,
                   const Waypoint & wp, 
                   const TaskBehaviour &tb,
                   const bool b_scored);

  virtual ~SampledTaskPoint() {};

  /** 
   * Reset the task (as if never flown)
   * 
   */
  virtual void reset();

/** 
 * Retrieve interior sample polygon.
 * Because sometimes an OZ will be skipped (by accident, true miss, or
 * failure of electronics), but we still want rest of task to function,
 * the 'cheat' option allows non-achieved task points to be considered achieved
 * by assuming the aircraft appeared at the reference location.
 * 
 * @param cheat If true, adds reference point as a sampled point
 * @return Vector of boundary points representing a closed polygon
 */
  const SearchPointVector& get_search_points(bool cheat=false);

/** 
 * Retrieve boundary polygon
 * 
 * @return Vector of boundary points representing a closed polygon
 */
  const SearchPointVector& get_boundary_points() const;

/** 
 * Re-project boundary and interior sample polygons.
 * Must be called if task_projection changes.
 * 
 */
  virtual void update_projection();

/** 
 * Set the location of the sample/boundary polygon node
 * that produces the maximum task distance.
 * 
 * @param locmax Location of max distance node 
 */
  void set_search_max(const SearchPoint &locmax) {
    search_max = locmax;
  }

/** 
 * Set the location of the sample/boundary polygon node
 * that produces the minimum task distance.
 * 
 * @param locmin Location of min distance node 
 */
  void set_search_min(const SearchPoint &locmin) {
    search_min = locmin;
  }

/** 
 * Accessor to retrieve the sample/boundary polygon
 * node that produces the maximum task distance.
 * 
 * @return Max distance node
 */
  const SearchPoint& get_search_max() const {
    return search_max;
  }

/** 
 * Accessor to retrieve location of the sample/boundary polygon
 * node that produces the maximum task distance.
 * 
 * @return Location of max distance node
 */
  GEOPOINT getMaxLocation() const {
    return search_max.getLocation();
  };

/** 
 * Accessor to retrieve node of the sample/boundary polygon
 * that produces the minimum task distance.
 * 
 * @return Minimum distance node
 */
  const SearchPoint& get_search_min() const {
    return search_min;
  }

/** 
 * Accessor to retrieve location of the sample/boundary polygon
 * node that produces the minimum task distance.
 * 
 * @return Location of minimum distance node
 */
  GEOPOINT getMinLocation() const {
    return search_min.getLocation();
  };

/** 
 * Construct boundary polygon from internal representation of observation zone.
 * 
 */
  virtual void initialise_boundary_points();

/** 
 * Check if aircraft is within observation zone, and if so,
 * update the interior sample polygon.
 * 
 * @param state Aircraft state
 * @param task_events Callback class for feedback
 *
 * @return True if internal state changed
 */
  virtual bool update_sample(const AIRCRAFT_STATE& state,
                             const TaskEvents &task_events);

/** 
 * Accessor for task projection
 * 
 * @return Task projection used by this point
 */
  const TaskProjection &get_task_projection() const {
    return task_projection;
  };

#ifdef DO_PRINT
  virtual void print(std::ostream& f, const AIRCRAFT_STATE&state) const;
  virtual void print_samples(std::ostream& f, const AIRCRAFT_STATE&state);
#endif

protected:
  const bool boundary_scored; /**< Whether boundaries are used in scoring distance, or just the reference point */

/** 
 * Clear all sample points.
 * 
 */
  virtual void clear_sample_points();

/** 
 * Clear all sample points and add the current state as a sample.
 * This is used, for exmaple, for StartPoints to only remember the last sample
 * prior to crossing the start.
 */  
  virtual void clear_sample_all_but_last(const AIRCRAFT_STATE& state);

private:
  const TaskProjection &task_projection;

/** 
 * Convert interior sample points to convex hull
 * 
 * @return True if interior sample points were trimmed
 */
  virtual bool prune_sample_points();

/** 
 * Convert boundary points to convex hull
 * 
 * @return True if boundary points were trimmed
 */
  virtual bool prune_boundary_points();

/** 
 * Clear all boundary points.
 * 
 */
  virtual void clear_boundary_points();

  SearchPointVector sampled_points;
  SearchPointVector boundary_points;
  SearchPoint search_max;
  SearchPoint search_min;
};
#endif //SAMPLEDOBSERVATIONZONE_H
