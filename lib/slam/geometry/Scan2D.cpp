#include <slam/geometry/Scan2D.h>

namespace slam {

void Scan2D::SetScanedPoints(const std::vector<ScanPoint2D> &scaned_points) {
  // copy ctor
  m_scaned_points.clear();
  for (const auto point : scaned_points) {
    m_scaned_points.emplace_back(point);
  }
}

void Scan2D::SetPose(const slam::Pose2D &pose) { m_pose = pose; }

void Scan2D::SetAngle(double rad) { m_pose.SetAngle(rad); }

void Scan2D::SetId(int id) { m_id = id; }

} // namespace slam
