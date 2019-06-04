#include <cmath>
#include <slam/icp/ScanMatcher2D.h>

namespace slam {

bool ScanMatcher2D::MatchScan(Scan2D &curScan) {
  m_cnt++;

  if (m_cnt == 0) {
    GrowMap(curScan, m_init_pose);
    m_prev_scan = curScan;
    return true;
  }

  // compare the previous pose and curent pose by odometry
  // in the frame of previous pose
  Pose2D odoMotion;
  Pose2D::CalcRelativePose(curScan.pose(), m_prev_scan.pose(), odoMotion);

  // m_point_cloud_ptr stores the optimized (ICP-ed) poses
  Pose2D lastPose = m_point_cloud_ptr->GetLastPose();

  // predictedPose is the predicted pose based on odoMotion from lastPose
  // used as the initial guess in ICP algorithm
  Pose2D predictedPose;
  Pose2D::CalcGlobalPose(odoMotion, lastPose, predictedPose);

  const Scan2D *refScan = m_ref_scan_maker_ptr->MakeRefScan();
  // set refScan as the reference PointClouds in ICP, use
  // DataAssociator->SetRefBase()
  m_estimator_ptr->SetScanPair(&curScan, refScan);

  Pose2D estimatedPose;

  double score = m_estimator_ptr->EstimatePose(predictedPose, estimatedPose);

  std::size_t usedNum = m_estimator_ptr->GetUsedNum();

  bool successful =
      (score <= m_score_thresh && usedNum >= m_used_num_thresh) ? true : false;

  if (m_degenerate_check) {
    if (successful) {
      // todo
      // do sensor fusion
    }
  } else {
    estimatedPose = predictedPose; // use raw odometry
  }

  GrowMap(curScan,
          estimatedPose); // add the current scan with the estimated Pose

  m_prev_scan = curScan;

  // for validation
  Pose2D estimatedMotion;
  Pose2D::CalcRelativePose(estimatedPose, lastPose, estimatedMotion);
  m_acc_dist = std::hypot(estimatedMotion.tx(), estimatedMotion.ty());

  return successful;
}

void ScanMatcher2D::GrowMap(const Scan2D &scan, const Pose2D &pose) {
  const auto &scaned_points = scan.scaned_points();
  double tx = pose.tx();
  double ty = pose.ty();

  std::vector<ScanPoint2D> scaned_points_global;
  for (unsigned i = 0; i < scaned_points.size(); ++i) {
    const ScanPoint2D &scan_point = scaned_points[i];

    double x = pose.R00() * scan_point.x() + pose.R01() * scan_point.y() + tx;
    double y = pose.R10() * scan_point.x() + pose.R11() * scan_point.y() + ty;

    scaned_points_global.emplace_back(m_cnt, x, y);
  }

  // register the new global points
  m_point_cloud_ptr->AddPose(pose);
  m_point_cloud_ptr->AddPoints(scaned_points_global);
  m_point_cloud_ptr->SetLastPose(pose);
  m_point_cloud_ptr->SetLastScan(scan);
}

} /* namespace slam */
